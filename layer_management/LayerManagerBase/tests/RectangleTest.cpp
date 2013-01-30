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

#include <gtest/gtest.h>

#include "Rectangle.h"

TEST(RectangleTest, defaultConstructor)
{
    /// create rectangle
    Rectangle rect;

    /// check default properties of rectangle
    EXPECT_EQ(0u, rect.x);
    EXPECT_EQ(0u, rect.y);
    EXPECT_EQ(0u, rect.width);
    EXPECT_EQ(0u, rect.height);
}

TEST(RectangleTest, specialConstructor)
{
    /// create rectangle with specified properties
    Rectangle rect(1, 2, 3, 4);

    /// check specified properties of rectangle
    EXPECT_EQ(1u, rect.x);
    EXPECT_EQ(2u, rect.y);
    EXPECT_EQ(3u, rect.width);
    EXPECT_EQ(4u, rect.height);
}

TEST(RectangleTest, assignOperator)
{
    /// create 3 rectangles with different properties
    Rectangle rect1;
    Rectangle rect2(1, 2, 3, 4);
    Rectangle rect3(9, 8, 7, 6);

    /// make sure, rectangle 1 and 2 have different properties
    EXPECT_NE(rect2.x, rect1.x);
    EXPECT_NE(rect2.y, rect1.y);
    EXPECT_NE(rect2.width, rect1.width);
    EXPECT_NE(rect2.height, rect1.height);

    /// assign specified rectangle 2 to rectangle 1
    rect1 = rect2;

    /// make sure, rectangle 1 and 2 have equal properties now
    EXPECT_EQ(rect2.x, rect1.x);
    EXPECT_EQ(rect2.y, rect1.y);
    EXPECT_EQ(rect2.width, rect1.width);
    EXPECT_EQ(rect2.height, rect1.height);

    /// make sure, rectangle 2 and 3 have different properties
    EXPECT_NE(rect3.x, rect2.x);
    EXPECT_NE(rect3.y, rect2.y);
    EXPECT_NE(rect3.width, rect2.width);
    EXPECT_NE(rect3.height, rect2.height);

    /// assign rectangle 3 to rectangle 2
    rect2 = rect3;

    /// make sure, rectangle 2 and 3 have equal properties now
    EXPECT_EQ(rect3.x, rect2.x);
    EXPECT_EQ(rect3.y, rect2.y);
    EXPECT_EQ(rect3.width, rect2.width);
    EXPECT_EQ(rect3.height, rect2.height);
}
