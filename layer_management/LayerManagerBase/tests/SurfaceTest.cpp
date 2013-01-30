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

#include "Surface.h"
#include "Layer.h"

class SurfaceTest : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pSurface = new Surface(0);
        ASSERT_TRUE(m_pSurface);
    }

    void TearDown()
    {
        if (m_pSurface)
        {
            delete m_pSurface;
            m_pSurface = 0;
        }
    }

    Surface* m_pSurface;
};

TEST_F(SurfaceTest, defaultConstructor)
{
    Surface surface(0);

    /// make sure, surface was not added to any layer
    EXPECT_EQ(GraphicalObject::INVALID_ID, surface.getContainingLayerId());

    /// make sure, surface has default size
    EXPECT_EQ(0, surface.OriginalSourceHeight);
    EXPECT_EQ(0, surface.OriginalSourceWidth);

    /// make sure, surface has default visibility
    EXPECT_FALSE(surface.getVisibility());

    /// make sure, surface has default opacity
    EXPECT_DOUBLE_EQ(1.0, surface.getOpacity());

    /// make sure, surface has default orientation
    EXPECT_EQ(Zero, surface.getOrientation());

    /// make sure, surface has default pixel format
    EXPECT_EQ(PIXELFORMAT_UNKNOWN, surface.getPixelFormat());

    /// make sure, surface has default source rectangle
    const Rectangle& srcRect = surface.getSourceRegion();
    EXPECT_EQ(0u, srcRect.height);
    EXPECT_EQ(0u, srcRect.width);
    EXPECT_EQ(0u, srcRect.x);
    EXPECT_EQ(0u, srcRect.y);

    /// make sure, surface has default destination rectangle
    const Rectangle& destRect = surface.getDestinationRegion();
    EXPECT_EQ(0u, destRect.height);
    EXPECT_EQ(0u, destRect.width);
    EXPECT_EQ(0u, destRect.x);
    EXPECT_EQ(0u, destRect.y);

    /// make sure, surface has no native content associated
    EXPECT_FALSE(m_pSurface->hasNativeContent());
}

TEST_F(SurfaceTest, specialConstructor)
{
    unsigned int expectedId = 144;
    Surface surface(expectedId);

    /// make sure surface has specified id
    EXPECT_EQ(expectedId, surface.getID());

    /// make sure, surface was not added to any layer
    EXPECT_EQ(GraphicalObject::INVALID_ID, surface.getContainingLayerId());

    /// make sure, surface has default size
    EXPECT_EQ(0, surface.OriginalSourceHeight);
    EXPECT_EQ(0, surface.OriginalSourceWidth);

    /// make sure, surface has default visibility
    EXPECT_FALSE(surface.getVisibility());

    /// make sure, surface has default opacity
    EXPECT_DOUBLE_EQ(1.0, surface.getOpacity());

    /// make sure, surface has default orientation
    EXPECT_EQ(Zero, surface.getOrientation());

    /// make sure, surface has default pixel format
    EXPECT_EQ(PIXELFORMAT_UNKNOWN, surface.getPixelFormat());

    /// make sure, surface has default source rectangle
    const Rectangle& srcRect = surface.getSourceRegion();
    EXPECT_EQ(0u, srcRect.height);
    EXPECT_EQ(0u, srcRect.width);
    EXPECT_EQ(0u, srcRect.x);
    EXPECT_EQ(0u, srcRect.y);

    /// make sure, surface has default destination rectangle
    const Rectangle& destRect = surface.getDestinationRegion();
    EXPECT_EQ(0u, destRect.height);
    EXPECT_EQ(0u, destRect.width);
    EXPECT_EQ(0u, destRect.x);
    EXPECT_EQ(0u, destRect.y);

    /// make sure, surface has no native content associated
    EXPECT_FALSE(m_pSurface->hasNativeContent());
}

TEST_F(SurfaceTest, getPixelFormat)
{
    PixelFormat expected1 = PIXELFORMAT_RGBA5551;
    PixelFormat expected2 = PIXELFORMAT_RGB565;

    /// set pixel format of surface to expected value
    m_pSurface->setPixelFormat(expected1);

    /// make sure, surface has expected pixel format
    EXPECT_EQ(expected1, m_pSurface->getPixelFormat());

    /// set pixel format of surface to other expected value
    m_pSurface->setPixelFormat(expected2);

    /// make sure, surface has other expected pixel format
    EXPECT_EQ(expected2, m_pSurface->getPixelFormat());
}

TEST_F(SurfaceTest, setPixelFormat)
{
    PixelFormat expected1 = PIXELFORMAT_R8;
    PixelFormat expected2 = PIXELFORMAT_RGBA8888;

    /// set pixel format of surface to expected value
    m_pSurface->setPixelFormat(expected1);

    /// make sure, surface has expected pixel format
    EXPECT_EQ(expected1, m_pSurface->getPixelFormat());

    /// set pixel format of surface to other expected value
    m_pSurface->setPixelFormat(expected2);

    /// make sure, surface has other expected pixel format
    EXPECT_EQ(expected2, m_pSurface->getPixelFormat());
}

TEST_F(SurfaceTest, getContainingLayerId)
{
    unsigned int expectedLayerId = 202;
    Layer layer(expectedLayerId);

    /// make sure, surface has no containg layer
    EXPECT_EQ(GraphicalObject::INVALID_ID, m_pSurface->getContainingLayerId());

    /// add surface to layer
    layer.addSurface(m_pSurface);

    /// make sure, surface is contained by layer
    EXPECT_EQ(expectedLayerId, m_pSurface->getContainingLayerId());

    /// remove surface from layer
    layer.removeSurface(m_pSurface);

    /// make sure, surface is no longer contained by layer
    EXPECT_EQ(GraphicalObject::INVALID_ID, m_pSurface->getContainingLayerId());
}

TEST_F(SurfaceTest, setContainingLayerId)
{
    unsigned int expectedLayerId = 207;

    /// make sure, surface has no containing layer
    EXPECT_EQ(GraphicalObject::INVALID_ID, m_pSurface->getContainingLayerId());

    /// set surface to be contained by expected layer
    m_pSurface->setContainingLayerId(expectedLayerId);

    /// make sure, surface is contained by layer
    EXPECT_EQ(expectedLayerId, m_pSurface->getContainingLayerId());

    /// set surface to be contained by no layer
    m_pSurface->setContainingLayerId(GraphicalObject::INVALID_ID);

    /// make sure, surface is no longer contained by layer
    EXPECT_EQ(GraphicalObject::INVALID_ID, m_pSurface->getContainingLayerId());
}

TEST_F(SurfaceTest, hasNativeContent)
{
    unsigned int expectedNativeHandle = 211;

    /// make sure, surface has no native content associated
    EXPECT_FALSE(m_pSurface->hasNativeContent());

    /// set expected native content of surface
    m_pSurface->setNativeContent(expectedNativeHandle);

    /// make sure, surface has native content associated
    EXPECT_TRUE(m_pSurface->hasNativeContent());

    /// remove native content of surface
    m_pSurface->removeNativeContent();

    /// make sure, surface has no native content associated
    EXPECT_FALSE(m_pSurface->hasNativeContent());
}

TEST_F(SurfaceTest, removeNativeContent)
{
    unsigned int expectedNativeHandle = 213;

    /// set expected native content of surface
    m_pSurface->setNativeContent(expectedNativeHandle);

    /// make sure, surface has native content associated
    EXPECT_TRUE(m_pSurface->hasNativeContent());

    /// remove native content of surface
    m_pSurface->removeNativeContent();

    /// make sure, surface has no native content associated
    EXPECT_FALSE(m_pSurface->hasNativeContent());
}

TEST_F(SurfaceTest, setNativeContent)
{
    unsigned int expectedNativeHandle1 = 217;
    unsigned int expectedNativeHandle2 = 218;

    /// make sure, surface has no native content associated
    EXPECT_FALSE(m_pSurface->hasNativeContent());

    /// set 1st expected native content of surface
    m_pSurface->setNativeContent(expectedNativeHandle1);

    /// make sure, surface has 1st expected native content associated
    EXPECT_TRUE(m_pSurface->hasNativeContent());
    EXPECT_EQ((long)expectedNativeHandle1, m_pSurface->getNativeContent());

    /// set 2nd expected native content of surface
    m_pSurface->setNativeContent(expectedNativeHandle2);

    /// make sure, surface still has 1st expected native content associated
    EXPECT_TRUE(m_pSurface->hasNativeContent());
    EXPECT_EQ((long)expectedNativeHandle1, m_pSurface->getNativeContent());

    /// remove native content of surface
    m_pSurface->removeNativeContent();

    /// set 2nd expected native content of surface again
    m_pSurface->setNativeContent(expectedNativeHandle2);

    /// make sure, surface has 2nd expected native content associated
    EXPECT_TRUE(m_pSurface->hasNativeContent());
    EXPECT_EQ((long)expectedNativeHandle2, m_pSurface->getNativeContent());
}

TEST_F(SurfaceTest, getNativeContent)
{
    unsigned int expectedNativeHandle1 = 217;
    unsigned int expectedNativeHandle2 = 218;

    /// make sure, surface has no native content associated
    EXPECT_FALSE(m_pSurface->hasNativeContent());

    /// set 1st expected native content of surface
    m_pSurface->setNativeContent(expectedNativeHandle1);

    /// make sure, surface has 1st expected native content associated
    EXPECT_TRUE(m_pSurface->hasNativeContent());
    EXPECT_EQ((long)expectedNativeHandle1, m_pSurface->getNativeContent());

    /// remove native content of surface
    m_pSurface->removeNativeContent();

    /// set 2nd expected native content of surface again
    m_pSurface->setNativeContent(expectedNativeHandle2);

    /// make sure, surface has 2nd expected native content associated
    EXPECT_TRUE(m_pSurface->hasNativeContent());
    EXPECT_EQ((long)expectedNativeHandle2, m_pSurface->getNativeContent());
}
