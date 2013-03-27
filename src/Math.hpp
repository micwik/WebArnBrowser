// Copyright (C) 2010-2013 Michael Wiklund.
// All rights reserved.
// Contact: arnlib@wiklunden.se
//
// This file is part of the WebArnBrowser - Active Registry Network Browser.
// Parts of WebArnBrowser depend on Qt 4 and/or other libraries that have their own
// licenses. WebArnBrowser is independent of these licenses; however, use of these other
// libraries is subject to their respective license agreements.
//
// GNU Lesser General Public License Usage
// The dependent library Wt is not using LGPL.
// This file may be used under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation and
// appearing in the file LICENSE.LGPL included in the packaging of this file.
// In addition, as a special exception, you may use the rights described
// in the Nokia Qt LGPL Exception version 1.1, included in the file
// LGPL_EXCEPTION.txt in this package.
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public License version 3.0 as published by the Free Software Foundation
// and appearing in the file LICENSE.GPL included in the packaging of this file.
//
// Other Usage
// Alternatively, this file may be used in accordance with the terms and
// conditions contained in a signed written agreement between you and Michael Wiklund.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//

#ifndef MATH_HPP
#define MATH_HPP

#include <QDateTime>


namespace Math
{
    template <typename T>
    T minLim( const T& x, const T& lim)
    {
        return x < lim ? lim : x;
    }

    template <typename T>
    T maxLim( const T& x, const T& lim)
    {
        return x > lim ? lim : x;
    }

    template <typename T>
    T rangeLim( const T& x, const T& min, const T& max)
    {
        return x < min ? min : (x > max ? max : x);
    }

    int  mod( int x, int y);
    int  circVal( int x, int lo, int hi);
}

#endif // MATH_HPP