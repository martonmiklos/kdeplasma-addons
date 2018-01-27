/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHOWDESKTOP_HEADER
#define SHOWDESKTOP_HEADER

#include <QObject>

class ShowDesktop : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool showingDesktop READ showingDesktop WRITE setShowingDesktop NOTIFY showingDesktopChanged)

public:
    ShowDesktop(QObject *parent = nullptr);
    ~ShowDesktop();

    bool showingDesktop() const;
    void setShowingDesktop(bool showingDesktop);

    Q_INVOKABLE void minimizeAll();

Q_SIGNALS:
    void showingDesktopChanged(bool showingDesktop);

};

#endif //SHOWDESKTOP_HEADER
