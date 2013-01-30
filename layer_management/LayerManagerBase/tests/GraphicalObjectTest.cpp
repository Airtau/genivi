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

#include "GraphicalObject.h"

class GraphicalObjectTest : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pSurface = new GraphicalObject(TypeSurface, 1.0, true, 0);
        m_pLayer = new GraphicalObject(TypeLayer, 1.0, true, 0);
        ASSERT_TRUE(m_pSurface);
        ASSERT_TRUE(m_pLayer);
    }

    void TearDown()
    {
        if (m_pSurface)
        {
            delete m_pSurface;
            m_pSurface = 0;
        }
        if (m_pLayer)
        {
            delete m_pLayer;
            m_pLayer = 0;
        }
    }

    GraphicalObject* m_pSurface;
    GraphicalObject* m_pLayer;
};

TEST_F(GraphicalObjectTest, constructor_LayerWithoutId)
{
    unsigned int expectedOpacity = 0.7;
    unsigned int expectedVisibility = 1;

    /// Create graphical object of type layer without specific id
    GraphicalObject layer(TypeLayer, expectedOpacity, expectedVisibility, 0);

    /// make sure, layer has a valid id assigned
    EXPECT_NE(GraphicalObject::INVALID_ID, layer.getID());

    /// make sure, layer has expected opacity
    EXPECT_EQ(expectedOpacity, layer.getOpacity());

    /// make sure, layer has expected visibility
    EXPECT_EQ(expectedVisibility, layer.getVisibility());
}

TEST_F(GraphicalObjectTest, constructor_SurfaceWithoutId)
{
    unsigned int expectedOpacity = 0.3;
    unsigned int expectedVisibility = 0;
    unsigned int expectedChromaKeyEnabled = 0;
    unsigned char expectedChromaKeyRed = 0;
    unsigned char expectedChromaKeyGreen = 0;
    unsigned char expectedChromaKeyBlue = 0;
    unsigned char chromaKeyRed = 3;
    unsigned char chromaKeyGreen = 22;
    unsigned char chromaKeyBlue = 111;

    /// Create graphical object of type surface without specific id
    GraphicalObject surface(TypeSurface, expectedOpacity, expectedVisibility, 0);

    /// make sure, surface has a valid id assigned
    EXPECT_NE(GraphicalObject::INVALID_ID, surface.getID());

    /// make sure, surface has expected opacity
    EXPECT_EQ(expectedOpacity, surface.getOpacity());

    /// make sure, surface has expected visibility
    EXPECT_EQ(expectedVisibility, surface.getVisibility());

    /// make sure, surface has expected chromaKeyEnabled
    EXPECT_EQ(expectedChromaKeyEnabled, surface.getChromaKeyEnabled());

    /// make sure, surface has expected chromaKey
    surface.getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(expectedChromaKeyRed, chromaKeyRed);
    EXPECT_EQ(expectedChromaKeyGreen, chromaKeyGreen);
    EXPECT_EQ(expectedChromaKeyBlue, chromaKeyBlue);
}

TEST_F(GraphicalObjectTest, constructor_LayerWithId)
{
    unsigned int expectedOpacity = 0.7;
    unsigned int expectedVisibility = 1;
    unsigned int expectedId = 411;
    unsigned int expectedChromaKeyEnabled = 0;
    unsigned char expectedChromaKeyRed = 0;
    unsigned char expectedChromaKeyGreen = 0;
    unsigned char expectedChromaKeyBlue = 0;
    unsigned char chromaKeyRed = 3;
    unsigned char chromaKeyGreen = 22;
    unsigned char chromaKeyBlue = 111;

    /// Create graphical object of type layer without specific id
    GraphicalObject layer(expectedId, TypeLayer, expectedOpacity, expectedVisibility, 0);

    /// make sure, layer has a expected id assigned
    EXPECT_EQ(expectedId, layer.getID());

    /// make sure, layer has expected opacity
    EXPECT_EQ(expectedOpacity, layer.getOpacity());

    /// make sure, layer has expected visibility
    EXPECT_EQ(expectedVisibility, layer.getVisibility());

    /// make sure, layer has expected chromaKeyEnabled
    EXPECT_EQ(expectedChromaKeyEnabled, layer.getChromaKeyEnabled());

    /// make sure, layer has expected chromaKey
    layer.getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(expectedChromaKeyRed, chromaKeyRed);
    EXPECT_EQ(expectedChromaKeyGreen, chromaKeyGreen);
    EXPECT_EQ(expectedChromaKeyBlue, chromaKeyBlue);
}

TEST_F(GraphicalObjectTest, constructor_SurfaceWithId)
{
    unsigned int expectedOpacity = 0.7;
    unsigned int expectedVisibility = 1;
    unsigned int expectedId = 411;
    unsigned int expectedChromaKeyEnabled = 0;
    unsigned char expectedChromaKeyRed = 0;
    unsigned char expectedChromaKeyGreen = 0;
    unsigned char expectedChromaKeyBlue = 0;
    unsigned char chromaKeyRed = 3;
    unsigned char chromaKeyGreen = 22;
    unsigned char chromaKeyBlue = 111;

    /// Create graphical object of type surface without specific id
    GraphicalObject surface(expectedId, TypeSurface, expectedOpacity, expectedVisibility, 0);

    /// make sure, surface has a expected id assigned
    EXPECT_EQ(expectedId, surface.getID());

    /// make sure, surface has expected opacity
    EXPECT_EQ(expectedOpacity, surface.getOpacity());

    /// make sure, surface has expected visibility
    EXPECT_EQ(expectedVisibility, surface.getVisibility());

    /// make sure, surface has expected chromaKeyEnabled
    EXPECT_EQ(expectedChromaKeyEnabled, surface.getChromaKeyEnabled());

    /// make sure, surface has expected chromaKey
    surface.getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(expectedChromaKeyRed, chromaKeyRed);
    EXPECT_EQ(expectedChromaKeyGreen, chromaKeyGreen);
    EXPECT_EQ(expectedChromaKeyBlue, chromaKeyBlue);
}

TEST_F(GraphicalObjectTest, setOpacity_Layer)
{
    double expectedOpacity = 0.12;

    /// make sure, layer has default opacity
    EXPECT_DOUBLE_EQ(1.0, m_pLayer->getOpacity());

    /// set opacity of layer to expected value
    m_pLayer->setOpacity(expectedOpacity);

    /// make sure, layer has expected opacity
    EXPECT_DOUBLE_EQ(expectedOpacity, m_pLayer->getOpacity());
}

TEST_F(GraphicalObjectTest, setOpacity_Surface)
{
    double expectedOpacity = 0.14;

    /// make sure, surface has default opacity
    EXPECT_DOUBLE_EQ(1.0, m_pSurface->getOpacity());

    /// set opacity of surface to expected value
    m_pSurface->setOpacity(expectedOpacity);

    /// make sure, surface has expected opacity
    EXPECT_DOUBLE_EQ(expectedOpacity, m_pSurface->getOpacity());
}

TEST_F(GraphicalObjectTest, getOpacity_Layer)
{
    double expectedOpacity = 0.77;

    /// make sure, layer has default opacity
    EXPECT_DOUBLE_EQ(1.0, m_pLayer->getOpacity());

    /// set opacity of layer to expected value
    m_pLayer->setOpacity(expectedOpacity);

    /// make sure, layer has expected opacity
    EXPECT_DOUBLE_EQ(expectedOpacity, m_pLayer->getOpacity());
}

TEST_F(GraphicalObjectTest, getOpacity_Surface)
{
    double expectedOpacity = 0.67;

    /// make sure, surface has default opacity
    EXPECT_DOUBLE_EQ(1.0, m_pSurface->getOpacity());

    /// set opacity of surface to expected value
    m_pSurface->setOpacity(expectedOpacity);

    /// make sure, surface has expected opacity
    EXPECT_DOUBLE_EQ(expectedOpacity, m_pSurface->getOpacity());
}

TEST_F(GraphicalObjectTest, setVisibility_Layer)
{
    /// set layer to visible
    m_pLayer->setVisibility(true);

    /// make sure, layer has expected visibility
    EXPECT_TRUE(m_pLayer->getVisibility());

    /// set layer to invisible
    m_pLayer->setVisibility(false);

    /// make sure, layer has expected visibility
    EXPECT_FALSE(m_pLayer->getVisibility());
}

TEST_F(GraphicalObjectTest, setVisibility_Surface)
{
    /// set surface to visible
    m_pSurface->setVisibility(true);

    /// make sure, surface has expected visibility
    EXPECT_TRUE(m_pSurface->getVisibility());

    /// set surface to invisible
    m_pSurface->setVisibility(false);

    /// make sure, surface has expected visibility
    EXPECT_FALSE(m_pSurface->getVisibility());
}

TEST_F(GraphicalObjectTest, getVisibility_Layer)
{
    /// set layer to visible
    m_pLayer->setVisibility(true);

    /// make sure, layer has expected visibility
    EXPECT_TRUE(m_pLayer->getVisibility());

    /// set layer to invisible
    m_pLayer->setVisibility(false);

    /// make sure, layer has expected visibility
    EXPECT_FALSE(m_pLayer->getVisibility());
}

TEST_F(GraphicalObjectTest, getVisibility_Surface)
{
    /// set surface to visible
    m_pSurface->setVisibility(true);

    /// make sure, surface has expected visibility
    EXPECT_TRUE(m_pSurface->getVisibility());

    /// set surface to invisible
    m_pSurface->setVisibility(false);

    /// make sure, surface has expected visibility
    EXPECT_FALSE(m_pSurface->getVisibility());
}

TEST_F(GraphicalObjectTest, getID_Layer)
{
    unsigned int expectedLayerId = 464;

    /// make sure, pre-defined layer has valid id
    EXPECT_NE(GraphicalObject::INVALID_ID, m_pLayer->getID());

    /// create graphical object of type layer with expected id
    GraphicalObject layer(expectedLayerId, TypeLayer, 1.0, 1, 0);

    /// make sure, graphical object has expected id
    EXPECT_EQ(expectedLayerId, layer.getID());
}

TEST_F(GraphicalObjectTest, getID_Surface)
{
    unsigned int expectedSurfaceId = 466;

    /// make sure, pre-defined surface has valid id
    EXPECT_NE(GraphicalObject::INVALID_ID, m_pSurface->getID());

    /// create graphical object of type surface with expected id
    GraphicalObject surface(expectedSurfaceId, TypeSurface, 1.0, 1, 0);

    /// make sure, graphical object has expected id
    EXPECT_EQ(expectedSurfaceId, surface.getID());
}

TEST_F(GraphicalObjectTest, setChromaKeyEnabled_Surface)
{
    /// set surface to enable chromakey
    m_pSurface->setChromaKeyEnabled(true);

    /// make sure, surface has expected chromakey enabled
    EXPECT_TRUE(m_pSurface->getChromaKeyEnabled());

    /// set surface to disable chromakey
    m_pSurface->setChromaKeyEnabled(false);

    /// make sure, surface has expected
    EXPECT_FALSE(m_pSurface->getChromaKeyEnabled());
}

TEST_F(GraphicalObjectTest, getChromaKeyEnabled_Surface)
{
    /// set surface to enable chromakey
    m_pSurface->setChromaKeyEnabled(true);

    /// make sure, surface has expected chromakey enabled
    EXPECT_TRUE(m_pSurface->getChromaKeyEnabled());

    /// set surface to disable chromakey
    m_pSurface->setChromaKeyEnabled(false);

    /// make sure, surface has expected chromakey enabled
    EXPECT_FALSE(m_pSurface->getChromaKeyEnabled());
}

TEST_F(GraphicalObjectTest, setChromaKey_Surface)
{
    unsigned char expectedChromaKeyRed   = 3;
    unsigned char expectedChromaKeyGreen = 22;
    unsigned char expectedChromaKeyBlue  = 111;
    unsigned char chromaKeyRed = 0xFF;
    unsigned char chromaKeyGreen = 0xFF;
    unsigned char chromaKeyBlue = 0xFF;

    /// make sure, surface has default chromakey
    m_pSurface->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(0, chromaKeyRed);
    EXPECT_EQ(0, chromaKeyGreen);
    EXPECT_EQ(0, chromaKeyBlue);

    /// set chromakey of surface to expected value
    m_pSurface->setChromaKey(expectedChromaKeyRed,
                             expectedChromaKeyGreen,
                             expectedChromaKeyBlue);

    /// make sure, surface has expected chromakey
    m_pSurface->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(expectedChromaKeyRed, chromaKeyRed);
    EXPECT_EQ(expectedChromaKeyGreen, chromaKeyGreen);
    EXPECT_EQ(expectedChromaKeyBlue, chromaKeyBlue);
}

TEST_F(GraphicalObjectTest, getChromaKey_Surface)
{
    unsigned char expectedChromaKeyRed   = 3;
    unsigned char expectedChromaKeyGreen = 22;
    unsigned char expectedChromaKeyBlue  = 111;
    unsigned char chromaKeyRed = 0xFF;
    unsigned char chromaKeyGreen = 0xFF;
    unsigned char chromaKeyBlue = 0xFF;

    /// make sure, surface has default chromakey
    m_pSurface->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(0, chromaKeyRed);
    EXPECT_EQ(0, chromaKeyGreen);
    EXPECT_EQ(0, chromaKeyBlue);

    /// set chromakey of surface to expected value
    m_pSurface->setChromaKey(expectedChromaKeyRed,
                             expectedChromaKeyGreen,
                             expectedChromaKeyBlue);

    /// make sure, surface has expected chromakey
    m_pSurface->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(expectedChromaKeyRed, chromaKeyRed);
    EXPECT_EQ(expectedChromaKeyGreen, chromaKeyGreen);
    EXPECT_EQ(expectedChromaKeyBlue, chromaKeyBlue);
}

TEST_F(GraphicalObjectTest, setChromaKeyEnabled_Layer)
{
    /// set layer to enable chromakey
    m_pLayer->setChromaKeyEnabled(true);

    /// make sure, layer has expected chromakey enabled
    EXPECT_TRUE(m_pLayer->getChromaKeyEnabled());

    /// set layer to disable chromakey
    m_pLayer->setChromaKeyEnabled(false);

    /// make sure, layer has expected
    EXPECT_FALSE(m_pLayer->getChromaKeyEnabled());
}

TEST_F(GraphicalObjectTest, getChromaKeyEnabled_Layer)
{
    /// set layer to enable chromakey
    m_pLayer->setChromaKeyEnabled(true);

    /// make sure, layer has expected chromakey enabled
    EXPECT_TRUE(m_pLayer->getChromaKeyEnabled());

    /// set layer to disable chromakey
    m_pLayer->setChromaKeyEnabled(false);

    /// make sure, layer has expected chromakey enabled
    EXPECT_FALSE(m_pLayer->getChromaKeyEnabled());
}

TEST_F(GraphicalObjectTest, setChromaKey_Layer)
{
    unsigned char expectedChromaKeyRed   = 3;
    unsigned char expectedChromaKeyGreen = 22;
    unsigned char expectedChromaKeyBlue  = 111;
    unsigned char chromaKeyRed   = 0xFF;
    unsigned char chromaKeyGreen = 0xFF;
    unsigned char chromaKeyBlue  = 0xFF;

    /// make sure, layer has default chromakey
    m_pLayer->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(0, chromaKeyRed);
    EXPECT_EQ(0, chromaKeyGreen);
    EXPECT_EQ(0, chromaKeyBlue);

    /// set chromakey of layer to expected value
    m_pLayer->setChromaKey(expectedChromaKeyRed,
                           expectedChromaKeyGreen,
                           expectedChromaKeyBlue);

    /// make sure, layer has expected chromakey
    m_pLayer->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(expectedChromaKeyRed,   chromaKeyRed);
    EXPECT_EQ(expectedChromaKeyGreen, chromaKeyGreen);
    EXPECT_EQ(expectedChromaKeyBlue,  chromaKeyBlue);
}

TEST_F(GraphicalObjectTest, getChromaKey_Layer)
{
    unsigned char expectedChromaKeyRed   = 3;
    unsigned char expectedChromaKeyGreen = 22;
    unsigned char expectedChromaKeyBlue  = 111;
    unsigned char chromaKeyRed   = 0xFF;
    unsigned char chromaKeyGreen = 0xFF;
    unsigned char chromaKeyBlue  = 0xFF;

    /// make sure, layer has default chromakey
    m_pLayer->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(0, chromaKeyRed);
    EXPECT_EQ(0, chromaKeyGreen);
    EXPECT_EQ(0, chromaKeyBlue);

    /// set chromakey of layer to expected value
    m_pLayer->setChromaKey(expectedChromaKeyRed,
                           expectedChromaKeyGreen,
                           expectedChromaKeyBlue);

    /// make sure, layer has expected chromakey
    m_pLayer->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);
    EXPECT_EQ(expectedChromaKeyRed,   chromaKeyRed);
    EXPECT_EQ(expectedChromaKeyGreen, chromaKeyGreen);
    EXPECT_EQ(expectedChromaKeyBlue,  chromaKeyBlue);
}

TEST_F(GraphicalObjectTest, DISABLED_setShader)
{
    // TODO: requires Shader object and shader code in files
}

TEST_F(GraphicalObjectTest, DISABLED_getShader)
{
    // TODO: requires Shader object and shader code in files
}
