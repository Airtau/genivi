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

#include "Layermanager.h"
#include "Configuration.h"

class LayermanagerTest : public ::testing::Test
{
public:
    void SetUp()
    {
        Configuration config(0, NULL);
        m_pLayermanager = new Layermanager(config);
        ASSERT_TRUE(m_pLayermanager);
    }

    void TearDown()
    {
        if (m_pLayermanager)
        {
            delete m_pLayermanager;
            m_pLayermanager = 0;
        }
    }

    ICommandExecutor* m_pLayermanager;
};

TEST_F(LayermanagerTest, DISABLED_execute)
{

}

TEST_F(LayermanagerTest, DISABLED_startManagement)
{

}

TEST_F(LayermanagerTest, DISABLED_stopManagement)
{

}

TEST_F(LayermanagerTest, DISABLED_getScene)
{

}

TEST_F(LayermanagerTest, DISABLED_getRendererList)
{

}

TEST_F(LayermanagerTest, DISABLED_addRenderer)
{

}

TEST_F(LayermanagerTest, DISABLED_removeRenderer)
{

}

TEST_F(LayermanagerTest, DISABLED_getCommunicatorList)
{

}

TEST_F(LayermanagerTest, DISABLED_addCommunicator)
{

}

TEST_F(LayermanagerTest, DISABLED_removeCommunicator)
{

}

TEST_F(LayermanagerTest, DISABLED_getSceneProviderList)
{

}

TEST_F(LayermanagerTest, DISABLED_addSceneProvider)
{

}

TEST_F(LayermanagerTest, DISABLED_removeSceneProvider)
{

}

TEST_F(LayermanagerTest, DISABLED_getLayerTypeCapabilities)
{

}

TEST_F(LayermanagerTest, DISABLED_getNumberOfHardwareLayers)
{

}

TEST_F(LayermanagerTest, DISABLED_getScreenResolution)
{

}

TEST_F(LayermanagerTest, DISABLED_getScreenIDs)
{

}
