/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "weatherpopupapplet.h"

#include <QTimer>
#include <QIcon>

#include <KConfigGroup>
#include <KConfigDialog>
#include <KLocalizedString>

#include <KUnitConversion/Converter>

#include <Plasma/PluginLoader>

#include "weatherconfig.h"
#include "weatherlocation.h"

using namespace KUnitConversion;

class WeatherPopupApplet::Private
{
public:
    Private(WeatherPopupApplet *weatherapplet)
        : q(weatherapplet)
        , weatherConfig(0)
        , weatherEngine(0)
        , timeEngine(0)
        , updateInterval(0)
        , location(0)
    {
        busyTimer = new QTimer(q);
        busyTimer->setInterval(2*60*1000);
        busyTimer->setSingleShot(true);
        QObject::connect(busyTimer, SIGNAL(timeout()), q, SLOT(giveUpBeingBusy()));
    }

    WeatherPopupApplet *q;
    WeatherConfig *weatherConfig;
    Plasma::DataEngine *weatherEngine;
    Plasma::DataEngine *timeEngine;
    Converter converter;
    Unit temperatureUnit;
    Unit speedUnit;
    Unit pressureUnit;
    Unit visibilityUnit;
    int updateInterval;
    QString source;
    WeatherLocation *location;

    QString conditionIcon;
    QString tend;
    Value pressure;
    Value temperature;
    double latitude;
    double longitude;
    QTimer *busyTimer;

    void locationReady(const QString &src)
    {
        if (!src.isEmpty()) {
            source = src;
            KConfigGroup cfg = q->config();
            cfg.writeEntry("source", source);
            emit q->configNeedsSaving();
            q->connectToEngine();
            q->setConfigurationRequired(false);
        } else {
            busyTimer->stop();
            // PORT!
//             q->showMessage(QIcon(), QString(), Plasma::ButtonNone);
            QObject *graphicObject = q->property("_plasma_graphicObject").value<QObject *>();
            if (graphicObject) {
                graphicObject->setProperty("busy", false);
            }
            q->setConfigurationRequired(true);
        }

        location->deleteLater();
        location = 0;
    }

    void giveUpBeingBusy()
    {
        QObject *graphicObject = q->property("_plasma_graphicObject").value<QObject *>();
        if (graphicObject) {
            graphicObject->setProperty("busy", false);
        }

        QStringList list = source.split(QLatin1Char( '|' ), QString::SkipEmptyParts);
        if (list.count() < 3) {
            q->setConfigurationRequired(true);
        } else {
            // PORT!
//             q->showMessage(KIcon(QLatin1String( "dialog-error" )),
//                            i18n("Weather information retrieval for %1 timed out.", list.value(2)),
//                            Plasma::ButtonNone);
        }
    }

    qreal tendency(const Value& pressure, const QString& tendency)
    {
        qreal t;

        if (tendency.toLower() == QLatin1String( "rising" )) {
            t = 0.75;
        } else if (tendency.toLower() == QLatin1String( "falling" )) {
            t = -0.75;
        } else {
            t = Value(tendency.toDouble(), pressure.unit()).convertTo(Kilopascal).number();
        }
        return t;
    }

    QString conditionFromPressure()
    {
        QString result;
        if (!pressure.isValid()) {
            return QLatin1String( "weather-none-available" );
        }
        qreal temp = temperature.convertTo(Celsius).number();
        qreal p = pressure.convertTo(Kilopascal).number();
        qreal t = tendency(pressure, tend);

        // This is completely unscientific so if anyone have a better formula for this :-)
        p += t * 10;

        // PORT!
//         Plasma::DataEngine::Data data = timeEngine->query(
//                 QString(QLatin1String( "Local|Solar|Latitude=%1|Longitude=%2" )).arg(latitude).arg(longitude));
        bool day = true;//(data[QLatin1String( "Corrected Elevation" )].toDouble() > 0.0);

        if (p > 103.0) {
            if (day) {
                result = QLatin1String( "weather-clear" );
            } else {
                result = QLatin1String( "weather-clear-night" );
            }
        } else if (p > 100.0) {
            if (day) {
                result = QLatin1String( "weather-clouds" );
            } else {
                result = QLatin1String( "weather-clouds-night" );
            }
        } else if (p > 99.0) {
            if (day) {
                if (temp > 1.0) {
                    result = QLatin1String( "weather-showers-scattered-day" );
                } else if (temp < -1.0)  {
                    result = QLatin1String( "weather-snow-scattered-day" );
                } else {
                    result = QLatin1String( "weather-snow-rain" );
                }
            } else {
                if (temp > 1.0) {
                    result = QLatin1String( "weather-showers-scattered-night" );
                } else if (temp < -1.0)  {
                    result = QLatin1String( "weather-snow-scattered-night" );
                } else {
                    result = QLatin1String( "weather-snow-rain" );
                }
            }
        } else {
            if (temp > 1.0) {
                result = QLatin1String( "weather-showers" );
            } else if (temp < -1.0)  {
                result = QLatin1String( "weather-snow" );
            } else {
                result = QLatin1String( "weather-snow-rain" );
            }
        }
        //kDebug() << result;
        return result;
    }

    Unit unit(const QString& unit)
    {
        if (!unit.isEmpty() && unit[0].isDigit()) {
            // PORT?
            return converter.unit(static_cast<UnitId>(unit.toInt()));
        }
        // Support < 4.4 config values
        return converter.unit(unit);
    }
};

WeatherPopupApplet::WeatherPopupApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
    , d(new Private(this))
{
    setHasConfigurationInterface(true);
}

WeatherPopupApplet::~WeatherPopupApplet()
{
    delete d;
}

void WeatherPopupApplet::init()
{
    configChanged();
}

void WeatherPopupApplet::connectToEngine()
{
    emit newWeatherSource();
    const bool missingLocation = d->source.isEmpty();

    if (missingLocation) {
        if (!d->location) {
            d->location = new WeatherLocation(this);
            connect(d->location, SIGNAL(finished(QString)), this, SLOT(locationReady(QString)));
        }

        Plasma::DataEngine *dataEngine =
            Plasma::PluginLoader::self()->loadDataEngine( QStringLiteral("geolocation") );
        d->location->setDataEngines(dataEngine, d->weatherEngine);
        d->location->getDefault();
        QObject *graphicObject = property("_plasma_graphicObject").value<QObject *>();
        if (graphicObject) {
            graphicObject->setProperty("busy", false);
        }
    } else {
        delete d->location;
        d->location = 0;
        d->weatherEngine->connectSource(d->source, this, d->updateInterval * 60 * 1000);
        QObject *graphicObject = property("_plasma_graphicObject").value<QObject *>();
        if (graphicObject) {
            graphicObject->setProperty("busy", true);
        }
        d->busyTimer->start();
    }
}
#if 0
void WeatherPopupApplet::createConfigurationInterface(KConfigDialog *parent)
{
    d->weatherConfig = new WeatherConfig(parent);
    d->weatherConfig->setDataEngine(d->weatherEngine);
    d->weatherConfig->setSource(d->source);
    d->weatherConfig->setUpdateInterval(d->updateInterval);
    d->weatherConfig->setTemperatureUnit(d->temperatureUnit->id());
    d->weatherConfig->setSpeedUnit(d->speedUnit->id());
    d->weatherConfig->setPressureUnit(d->pressureUnit->id());
    d->weatherConfig->setVisibilityUnit(d->visibilityUnit->id());
    parent->addPage(d->weatherConfig, i18n("Weather"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(d->weatherConfig, SIGNAL(configValueChanged()) , parent , SLOT(settingsModified()));
}
#endif
void WeatherPopupApplet::configAccepted()
{
#if 0
    d->temperatureUnit = d->converter.unit(static_cast<UnitId>(d->weatherConfig->temperatureUnit()));
    d->speedUnit = d->converter.unit(static_cast<UnitId>(d->weatherConfig->speedUnit()));
    d->pressureUnit = d->converter.unit(static_cast<UnitId>(d->weatherConfig->pressureUnit()));
    d->visibilityUnit = d->converter.unit(static_cast<UnitId>(d->weatherConfig->visibilityUnit()));
    d->updateInterval = d->weatherConfig->updateInterval();
    d->source = d->weatherConfig->source();
#endif
    KConfigGroup cfg = config();
    cfg.writeEntry("temperatureUnit", static_cast<int>(d->temperatureUnit.id()));
    cfg.writeEntry("speedUnit", static_cast<int>(d->speedUnit.id()));
    cfg.writeEntry("pressureUnit", static_cast<int>(d->pressureUnit.id()));
    cfg.writeEntry("visibilityUnit", static_cast<int>(d->visibilityUnit.id()));
    cfg.writeEntry("updateInterval", d->updateInterval);
    cfg.writeEntry("source", d->source);

    emit configNeedsSaving();
}

void WeatherPopupApplet::configChanged()
{
    if (!d->source.isEmpty()) {
        d->weatherEngine->disconnectSource(d->source, this);
    }

    KConfigGroup cfg = config();

    if (QLocale().measurementSystem() == QLocale::MetricSystem) {
        d->temperatureUnit = d->unit(cfg.readEntry("temperatureUnit", "C"));
        d->speedUnit = d->unit(cfg.readEntry("speedUnit", "m/s"));
        d->pressureUnit = d->unit(cfg.readEntry("pressureUnit", "hPa"));
        d->visibilityUnit = d->unit(cfg.readEntry("visibilityUnit", "km"));
    } else {
        d->temperatureUnit = d->unit(cfg.readEntry("temperatureUnit", "F"));
        d->speedUnit = d->unit(cfg.readEntry("speedUnit", "mph"));
        d->pressureUnit = d->unit(cfg.readEntry("pressureUnit", "inHg"));
        d->visibilityUnit = d->unit(cfg.readEntry("visibilityUnit", "ml"));
    }
    d->updateInterval = cfg.readEntry("updateInterval", 30);
    // TEMP!
    d->source = cfg.readEntry("source", "wettercom|weather|Turin, Piemont, IT|IT0PI0397;Turin");//"");
    setConfigurationRequired(d->source.isEmpty());
    d->weatherEngine = Plasma::PluginLoader::self()->loadDataEngine( QStringLiteral("weather") );
    d->timeEngine = Plasma::PluginLoader::self()->loadDataEngine( QStringLiteral("time") );

    connectToEngine();
}

void WeatherPopupApplet::dataUpdated(const QString& source,
                                     const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)

    if (data.isEmpty()) {
        return;
    }

    d->conditionIcon = data[QLatin1String( "Condition Icon" )].toString();
    if (data[QLatin1String( "Pressure" )].toString() != QLatin1String( "N/A" )) {
        d->pressure = Value(data[QLatin1String( "Pressure" )].toDouble(),
                            static_cast<UnitId>(data[QLatin1String( "Pressure Unit" )].toInt()));
    } else {
        d->pressure = Value();
    }
    d->tend = data[QLatin1String( "Pressure Tendency" )].toString();
    d->temperature = Value(data[QLatin1String( "Temperature" )].toDouble(),
                           static_cast<UnitId>(data[QLatin1String( "Temperature Unit" )].toInt()));
    d->latitude = data[QLatin1String( "Latitude" )].toDouble();
    d->longitude = data[QLatin1String( "Longitude" )].toDouble();
    setAssociatedApplicationUrls(QList<QUrl>() << QUrl(data.value(QLatin1String( "Credit Url" )).toString()));

    d->busyTimer->stop();
    // PORT!
//     showMessage(QIcon(), QString(), Plasma::ButtonNone);
    QObject *graphicObject = property("_plasma_graphicObject").value<QObject *>();
    if (graphicObject) {
        graphicObject->setProperty("busy", false);
    }
}

Unit WeatherPopupApplet::pressureUnit()
{
    return d->pressureUnit;
}

Unit WeatherPopupApplet::temperatureUnit()
{
    return d->temperatureUnit;
}

Unit WeatherPopupApplet::speedUnit()
{
    return d->speedUnit;
}

Unit WeatherPopupApplet::visibilityUnit()
{
    return d->visibilityUnit;
}

int WeatherPopupApplet::updateInterval() const
{
    return d->updateInterval;
}

void WeatherPopupApplet::setUpdateInterval(int updateInterval)
{
    if (d->updateInterval == updateInterval) {
        return;
    }
    d->updateInterval = updateInterval;
    emit updateIntervalChanged(updateInterval);
}

int WeatherPopupApplet::temperatureUnitId() const
{
    return d->temperatureUnit.id();
}

void WeatherPopupApplet::setTemperatureUnitId(int temperatureUnitId)
{
    if (d->temperatureUnit.id() == temperatureUnitId) {
        return;
    }
    d->temperatureUnit = d->converter.unit(static_cast<UnitId>(temperatureUnitId));
    emit temperatureUnitIdChanged(temperatureUnitId);
}

int WeatherPopupApplet::pressureUnitId() const
{
    return d->pressureUnit.id();
}

void WeatherPopupApplet::setPressureUnitId(int pressureUnitId)
{
    if (d->pressureUnit.id() == pressureUnitId) {
        return;
    }
    d->pressureUnit = d->converter.unit(static_cast<UnitId>(pressureUnitId));
    emit pressureUnitIdChanged(pressureUnitId);
}

int WeatherPopupApplet::windSpeedUnitId() const
{
    return d->speedUnit.id();
}

void WeatherPopupApplet::setWindSpeedUnitId(int windSpeedUnitId)
{
    if (d->speedUnit.id() == windSpeedUnitId) {
        return;
    }
    d->speedUnit = d->converter.unit(static_cast<UnitId>(windSpeedUnitId));
    emit windSpeedUnitIdChanged(windSpeedUnitId);
}

int WeatherPopupApplet::visibilityUnitId() const
{
    return d->visibilityUnit.id();
}

void WeatherPopupApplet::setVisibilityUnitId(int visibilityUnitId)
{
    if (d->visibilityUnit.id() == visibilityUnitId) {
        return;
    }
    d->visibilityUnit = d->converter.unit(static_cast<UnitId>(visibilityUnitId));
    emit visibilityUnitIdChanged(visibilityUnitId);
}


QString WeatherPopupApplet::conditionIcon()
{
    if (d->conditionIcon.isEmpty() || d->conditionIcon == QLatin1String( "weather-none-available" )) {
        d->conditionIcon = d->conditionFromPressure();
    }
    return d->conditionIcon;
}

WeatherConfig* WeatherPopupApplet::weatherConfig()
{
    return d->weatherConfig;
}


// needed due to private slots
#include "moc_weatherpopupapplet.cpp"
