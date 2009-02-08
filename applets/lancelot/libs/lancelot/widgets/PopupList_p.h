/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_POPUPLIST_PH
#define LANCELOT_POPUPLIST_PH

#include "PopupList.h"
#include <lancelot/widgets/ActionListView.h>
#include <QBasicTimer>
#include <lancelot/models/ActionTreeModel.h>
#include <lancelot/models/ActionListModel.h>

namespace Lancelot {

class PopupList::Private: public QObject {
    Q_OBJECT
public:
    Private(PopupList * parent);
    ~Private();

    ActionListView * list;
    QGraphicsScene * scene;

    ActionListModel * listModel;
    ActionTreeModel * treeModel;

    QBasicTimer timer;
    int closeTimeout;

    const PopupList * q;

public Q_SLOTS:
    void listItemActivated(int index);
    void connectSignals();
};

} // namespace Lancelot

#endif /* LANCELOT_POPUPLIST_PH */

