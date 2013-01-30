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

#include "Layer.h"

class LayerTest : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pLayer = new Layer(0);
        ASSERT_TRUE(m_pLayer);
    }

    void TearDown()
    {
        if (m_pLayer)
        {
            delete m_pLayer;
            m_pLayer = 0;
        }
    }

    Layer* m_pLayer;
};

TEST_F(LayerTest, defaultConstructor)
{
    Layer layer(0);

    /// make sure, Layer was not added to any layer
    //EXPECT_EQ(INVALID_ID, Layer.getContainingLayerId());

    /// make sure, Layer has default size
    EXPECT_EQ(0, layer.OriginalSourceHeight);
    EXPECT_EQ(0, layer.OriginalSourceWidth);

    /// make sure, Layer has default visibility
    EXPECT_FALSE(layer.getVisibility());

    /// make sure, Layer has default opacity
    EXPECT_DOUBLE_EQ(1.0, layer.getOpacity());

    /// make sure, Layer has default orientation
    EXPECT_EQ(Zero, layer.getOrientation());

    /// make sure, Layer has default source rectangle
    const Rectangle& srcRect = layer.getSourceRegion();
    EXPECT_EQ((uint)0, srcRect.height);
    EXPECT_EQ((uint)0, srcRect.width);
    EXPECT_EQ((uint)0, srcRect.x);
    EXPECT_EQ((uint)0, srcRect.y);

    /// make sure, Layer has default destination rectangle
    const Rectangle& destRect = layer.getDestinationRegion();
    EXPECT_EQ((uint)0, destRect.height);
    EXPECT_EQ((uint)0, destRect.width);
    EXPECT_EQ((uint)0, destRect.x);
    EXPECT_EQ((uint)0, destRect.y);

    /// make sure, layer layer has default type
    EXPECT_EQ(Software_2D, layer.getLayerType());
}

TEST_F(LayerTest, specialConstructor)
{
    unsigned int expectedId = 144;
    Layer layer(expectedId);

    /// make sure Layer has specified id
    EXPECT_EQ(expectedId, layer.getID());

    /// make sure, Layer was not added to any layer
    //EXPECT_EQ(INVALID_ID, Layer.getContainingLayerId());

    /// make sure, Layer has default size
    EXPECT_EQ(0, layer.OriginalSourceHeight);
    EXPECT_EQ(0, layer.OriginalSourceWidth);

    /// make sure, Layer has default visibility
    EXPECT_FALSE(layer.getVisibility());

    /// make sure, Layer has default opacity
    EXPECT_DOUBLE_EQ(1.0, layer.getOpacity());

    /// make sure, Layer has default orientation
    EXPECT_EQ(Zero, layer.getOrientation());

    /// make sure, Layer has default source rectangle
    const Rectangle& srcRect = layer.getSourceRegion();
    EXPECT_EQ((uint)0, srcRect.height);
    EXPECT_EQ((uint)0, srcRect.width);
    EXPECT_EQ((uint)0, srcRect.x);
    EXPECT_EQ((uint)0, srcRect.y);

    /// make sure, Layer has default destination rectangle
    const Rectangle& destRect = layer.getDestinationRegion();
    EXPECT_EQ((uint)0, destRect.height);
    EXPECT_EQ((uint)0, destRect.width);
    EXPECT_EQ((uint)0, destRect.x);
    EXPECT_EQ((uint)0, destRect.y);

    /// make sure, layer layer has default type
    EXPECT_EQ(Software_2D, layer.getLayerType());
}

TEST_F(LayerTest, getLayerType)
{
    /// make sure, layer has default layer type
    EXPECT_EQ(Software_2D, m_pLayer->getLayerType());

    /// set different layer type
    m_pLayer->setLayerType(Software_2_5D);

    /// make sure, layer type of layer was updated
    EXPECT_EQ(Software_2_5D, m_pLayer->getLayerType());
}

TEST_F(LayerTest, setLayerType)
{
    /// make sure, layer has default layer type
    EXPECT_EQ(Software_2D, m_pLayer->getLayerType());

    /// set different layer type
    m_pLayer->setLayerType(Software_2_5D);

    /// make sure, layer type of layer was updated
    EXPECT_EQ(Software_2_5D, m_pLayer->getLayerType());

    /// set layer type to default value
    m_pLayer->setLayerType(Software_2D);

    /// make sure, layer type of layer was updated
    EXPECT_EQ(Software_2D, m_pLayer->getLayerType());
}

TEST_F(LayerTest, setLayerCapabilities)
{
    unsigned int expectedCapabilities = LayerScalable | LayerOrientable;

    /// make sure, layer has default capabilities set
    EXPECT_EQ((uint)0, m_pLayer->getCapabilities());

    /// enable expected capabilities of layer
    m_pLayer->setLayerCapabilities(expectedCapabilities);

    /// make sure, the capabilities of layer were updated
    EXPECT_EQ(expectedCapabilities, m_pLayer->getCapabilities());

    /// reset capabilities of layer to default value
    m_pLayer->setLayerCapabilities(0);

    /// make sure, the capabilities of layer were updated
    EXPECT_EQ((uint)0, m_pLayer->getCapabilities());
}

TEST_F(LayerTest, getCapabilities)
{
    unsigned int expectedCapabilities = LayerScalable | LayerOrientable;

    /// make sure, layer has default capabilities set
    EXPECT_EQ((uint)0, m_pLayer->getCapabilities());

    /// enable expected capabilities of layer
    m_pLayer->setLayerCapabilities(expectedCapabilities);

    /// make sure, the capabilities of layer were updated
    EXPECT_EQ(expectedCapabilities, m_pLayer->getCapabilities());
}

TEST_F(LayerTest, addSurface)
{
    unsigned int expectedSurfaceId = 32;
    Surface surface(expectedSurfaceId);

    /// make sure, layer contains no surfaces
    SurfaceList& slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());

    /// add surface to layer
    m_pLayer->addSurface(&surface);

    /// make sure, layer contains one surface
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)1, slist.size());
}

TEST_F(LayerTest, addSurface_twice)
{
    unsigned int expectedSurfaceId = 33;
    Surface surface(expectedSurfaceId);

    /// make sure, layer contains no surfaces
    SurfaceList& slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());

    /// add surface to layer
    m_pLayer->addSurface(&surface);

    /// make sure, layer contains one surface
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)1, slist.size());

    /// add surface to layer again
    m_pLayer->addSurface(&surface);

    /// make sure, layer still contains one surface
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)1, slist.size());
}

TEST_F(LayerTest, addSurface_alreadyOnOtherLayer)
{
    unsigned int expectedSurfaceId = 34;
    Surface surface(expectedSurfaceId);
    Layer layer(0);

    /// make sure, layer contains no surfaces
    SurfaceList& slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());

    /// add surface to layer
    m_pLayer->addSurface(&surface);

    /// make sure, layer contains one surface
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)1, slist.size());

    /// add surface to other layer
    layer.addSurface(&surface);

    /// make sure, layer still contains one surface
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)1, slist.size());

    /// make sure, other layer contains no surfaces
    slist = layer.getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());
}

TEST_F(LayerTest, removeSurface)
{
    Surface s1(0), s2(0), s3(0);

    /// make sure, layer contains no surfaces
    SurfaceList& slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());

    /// add 3 surfaces to layer
    m_pLayer->addSurface(&s1);
    m_pLayer->addSurface(&s2);
    m_pLayer->addSurface(&s3);

    /// make sure, layer contains the 3 surfaces
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)3, slist.size());
    SurfaceListIterator iter = slist.begin();
    EXPECT_EQ(&s1, *iter++);
    EXPECT_EQ(&s2, *iter++);
    EXPECT_EQ(&s3, *iter++);
    EXPECT_EQ(slist.end(), iter);

    /// remove 2 surfaces from layer
    m_pLayer->removeSurface(&s1);
    m_pLayer->removeSurface(&s3);

    /// make sure, layer contains 1 surface
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)1, slist.size());
    iter = slist.begin();
    EXPECT_EQ(&s2, *iter++);
    EXPECT_EQ(slist.end(), iter);

    /// remove 1 surface from layer
    m_pLayer->removeSurface(&s2);

    /// make sure, layer contains no surfaces
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());
    EXPECT_EQ(slist.end(), slist.begin());
}

TEST_F(LayerTest, getAllSurfaces)
{
    Surface s1(0), s2(0), s3(0), s4(0), s5(0);

    /// make sure, getAllSurfaces returns empty list, if layer contains no surfaces
    SurfaceList& slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());

    /// add 5 surfaces to layer
    m_pLayer->addSurface(&s1);
    m_pLayer->addSurface(&s2);
    m_pLayer->addSurface(&s3);
    m_pLayer->addSurface(&s4);
    m_pLayer->addSurface(&s5);

    /// check if all added surfaces are returned by getAllSurfaces
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)5, slist.size());
    SurfaceListIterator iter = slist.begin();
    EXPECT_EQ(&s1, *iter++);
    EXPECT_EQ(&s2, *iter++);
    EXPECT_EQ(&s3, *iter++);
    EXPECT_EQ(&s4, *iter++);
    EXPECT_EQ(&s5, *iter++);
    EXPECT_EQ(slist.end(), iter);
}

TEST_F(LayerTest, removeAllSurfaces)
{
    Surface s1(0), s2(0), s3(0), s4(0), s5(0);

    /// make sure, getAllSurfaces returns empty list, if layer contains no surfaces
    SurfaceList& slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());

    /// add 5 surfaces to layer
    m_pLayer->addSurface(&s1);
    m_pLayer->addSurface(&s2);
    m_pLayer->addSurface(&s3);
    m_pLayer->addSurface(&s4);
    m_pLayer->addSurface(&s5);

    /// make sure, layer contains 5 surfaces
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)5, slist.size());

    /// remove all surfaces from layer
    m_pLayer->removeAllSurfaces();

    /// make sure, layer contains no surfaces
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());

    /// add 3 surfaces to layer
    m_pLayer->addSurface(&s1);
    m_pLayer->addSurface(&s2);
    m_pLayer->addSurface(&s3);

    /// make sure, layer contains 3 surfaces
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)3, slist.size());

    /// remove all surfaces from layer
    m_pLayer->removeAllSurfaces();

    /// make sure, layer contains no surfaces
    slist = m_pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());
}

