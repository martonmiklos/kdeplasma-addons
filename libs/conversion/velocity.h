/*
 *   Copyright (C) 2008-2009 Petri Damstén <damu@iki.fi>
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

#ifndef VELOCITY_H
#define VELOCITY_H

#include "unitcategory.h"

class Velocity : public Conversion::UnitCategory
{
public:
    enum Id {
        MeterPerSecond = 9000, KilometerPerHour, MilePerHour, FootPerSecond, InchPerSecond, Knot,
        Mach, SpeedOfLight, Beaufort
    };

    Velocity(QObject* parent = 0);
};

#endif
