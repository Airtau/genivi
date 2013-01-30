/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include <ostream>

class Rectangle
{
public:
    Rectangle()
    : x(0)
    , y(0)
    , width(0)
    , height(0)
    {
    }

    Rectangle(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    : x(x)
    , y(y)
    , width(width)
    , height(height)
    {
    }

    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

    bool operator ==(const Rectangle& rhs) const
    {
        return (x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height);
    }
};

class FloatRectangle
{
public:
    FloatRectangle()
    : x(0.0f)
    , y(0.0f)
    , width(0.0f)
    , height(0.0f)
    {
    }

    FloatRectangle(float x, float y, float width, float height)
    : x(x)
    , y(y)
    , width(width)
    , height(height)
    {
    }

    FloatRectangle(const Rectangle& rhs)
    : x(rhs.x)
    , y(rhs.y)
    , width(rhs.width)
    , height(rhs.height)
    {
    }

    float x;
    float y;
    float width;
    float height;
};

#endif /* _RECTANGLE_H_ */
