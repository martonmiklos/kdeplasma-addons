#include "kdeobservatory.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <KConfigDialog>

#include "ui_kdeobservatoryconfigcommits.h"
#include "kdeobservatoryconfiggeneral.h"
#include "commitcollector.h"

KdeObservatory::KdeObservatory(QObject *parent, const QVariantList &args)
: Plasma::Applet(parent, args)
{
    setBackgroundHints(DefaultBackground);
    setHasConfigurationInterface(true);  
    resize(200, 200);

    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(m_view = new QGraphicsView);
    m_view->setScene(m_scene = new QGraphicsScene);
    //m_view->setBackgroundBrush(QColor(0, 0, 0));
    //m_view->setAutoFillBackground(true);
    m_scene->addEllipse(0, 0, 50, 50);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout;
    layout->addItem(proxy);
    setLayout(layout);

    CommitCollector *c = new CommitCollector(this);
    c->run();
}

KdeObservatory::~KdeObservatory()
{
}

void KdeObservatory::init()
{
}

void KdeObservatory::createConfigurationInterface(KConfigDialog *parent)
{
    KdeObservatoryConfigGeneral *configGeneral = new KdeObservatoryConfigGeneral;
    parent->addPage(configGeneral, i18n("General"), icon());

    QWidget *configCommits = new QWidget;
    Ui::KdeObservatoryConfigCommits *ui_configCommits = new Ui::KdeObservatoryConfigCommits;
    ui_configCommits->setupUi(configCommits); 
    parent->addPage(configCommits, i18n("Commits"), icon());
}

#include "kdeobservatory.moc"

