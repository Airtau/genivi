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

#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <ostream>

class Vector2
{
public:
    Vector2()
    : val1(0)
    , val2(0)
    {
    }

    Vector2(unsigned int val1, unsigned int val2)
    : val1(val1)
    , val2(val2)
    {
    }

    unsigned int val1;
    unsigned int val2;

    bool operator ==(const Vector2& rhs) const
    {
        return (val1 == rhs.val1 && val2 == rhs.val2);
    }

    friend std::ostream& operator<<(std::ostream& stream, const Vector2& obj);
};

#endif /* _VECTOR2_H_ */
