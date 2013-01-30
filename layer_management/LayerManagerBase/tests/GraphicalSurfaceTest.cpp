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

#include "GraphicalSurface.h"

class GraphicalSurfaceTest : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pGraphicalSurface = new GraphicalSurface(TypeSurface, 0);
        ASSERT_TRUE(m_pGraphicalSurface);
    }

    void TearDown()
    {
        if (m_pGraphicalSurface)
        {
            delete m_pGraphicalSurface;
            m_pGraphicalSurface = 0;
        }
    }

    GraphicalSurface* m_pGraphicalSurface;
};

TEST_F(GraphicalSurfaceTest, DISABLED_construction)
{
    //GraphicalSurface(ObjectType type);
    //GraphicalSurface(int externalId,ObjectType type);
    //virtual ~GraphicalSurface() {}
}

TEST_F(GraphicalSurfaceTest, DISABLED_setOrientation)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_getOrientation)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_setSourceRegion)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_getSourceRegion)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_setDestinationRegion)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_setPosition)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_getPosition)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_setDimension)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_getDestinationRegion)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_getDimension)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_isInside)
{

}

TEST_F(GraphicalSurfaceTest, DISABLED_DestToSourceCoordinates)
{

}
