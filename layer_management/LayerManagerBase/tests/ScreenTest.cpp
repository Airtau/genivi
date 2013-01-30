/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#include "LmScreen.h"

class ScreenTest : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pScreen = new LmScreen(10, "/dev/dummy");
        ASSERT_TRUE(m_pScreen);
    }

    void TearDown()
    {
        if (m_pScreen)
        {
            delete m_pScreen;
            m_pScreen= 0;
        }
    }

    LmScreen* m_pScreen;
};

TEST_F(ScreenTest, constructor_withoutId)
{
    /// Create screen without specific id
    LmScreen* lmScreen = new LmScreen();

    /// make sure, screen has a default id assigned
    EXPECT_EQ(0u, lmScreen->getID());

    /// make sure, screen hasn't device name
    EXPECT_EQ(NULL, lmScreen->getDeviceName());

    /// make sure, screen hasn't element of CurrentRenderOrder
    EXPECT_EQ(true, lmScreen->getCurrentRenderOrder().empty());

    delete lmScreen;
}

TEST_F(ScreenTest, constructor_withId)
{
    unsigned int expectedId = 411;
    const char expectedDeviceName[] = "/dev/dummy";

    /// Create screen with specific id
    LmScreen* lmScreen = new LmScreen(expectedId, expectedDeviceName);

    /// make sure, screen has a expected id assigned
    EXPECT_EQ(expectedId, lmScreen->getID());

    /// make sure, screen has a expected device name
    EXPECT_STREQ(expectedDeviceName, lmScreen->getDeviceName());

    /// make sure, screen hasn't element of CurrentRenderOrder
    EXPECT_EQ(true, lmScreen->getCurrentRenderOrder().empty());

    delete lmScreen;
}

TEST_F(ScreenTest, getCurrentRenderOrder)
{
    // TODO: how to test? return by typically reference can't be invalid.

    /// get render order
    LayerList& llist = m_pScreen->getCurrentRenderOrder();

    /// make sure, list of layers is returned
    ASSERT_TRUE(&llist);
}
