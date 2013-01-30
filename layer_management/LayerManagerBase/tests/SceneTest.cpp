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

#include "IScene.h"
#include "Scene.h"
#include "SurfaceMap.h"
#include "LmScreenList.h"
#include <vector>

class SceneTest : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pScene = new Scene();
        ASSERT_TRUE(m_pScene);

        LmScreenList& screenList = m_pScene->getScreenList();
        LmScreen* pScreen = new LmScreen();
        ASSERT_TRUE(pScreen);
        screenList.push_back(pScreen);
    }

    void TearDown()
    {
        if (m_pScene)
        {
            LmScreenList& screenList = m_pScene->getScreenList();
            LmScreenListIterator iter = screenList.begin();
            LmScreenListIterator iterEnd = screenList.end();
            for (; iter != iterEnd; ++iter)
            {
                delete (*iter);
            }
            delete m_pScene;
            m_pScene = 0;
        }
    }

    IScene* m_pScene;
};

TEST_F(SceneTest, createLayer)
{
    uint size;
    uint* array;
    Layer* pLayer;
    uint expected = 91;

    /// make sure, scene contains no layers
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)0, size) << "Scene should contain 0 layers";

    /// create layer with expected id
    pLayer = m_pScene->createLayer(expected, 0);
    ASSERT_TRUE(pLayer) << "Layer was not created.";

    /// make sure layer has expected id
    EXPECT_EQ(expected, pLayer->getID());

    /// make sure, scene contains one layer
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)1, size) << "Scene should contain 1 layer";

    /// make sure, layer contains no surfaces
    SurfaceList& slist = pLayer->getAllSurfaces();
    EXPECT_EQ((uint)0, slist.size());
}

TEST_F(SceneTest, createLayer_invalidId)
{
    uint size;
    uint* array;
    Layer* pLayer;

    /// make sure, scene contains no layers
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)0, size) << "Scene should contain 0 layers";

    /// create layer with invalid id
    pLayer = m_pScene->createLayer(GraphicalObject::INVALID_ID, 0);
    ASSERT_TRUE(pLayer) << "Layer was not created.";

    /// make sure new layer has valid id
    EXPECT_NE(GraphicalObject::INVALID_ID, pLayer->getID());

    /// make sure, scene contains one layer
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)1, size) << "Scene should contain 1 layer";
}

TEST_F(SceneTest, createLayer_twice)
{
    uint size;
    uint* array;
    uint expected = 55;
    double expectedOpacity = 0.322;
    Layer* pLayer1;
    Layer* pLayer2;

    /// create Layer with id 55
    pLayer1 = m_pScene->createLayer(expected, 0);
    ASSERT_EQ(expected, pLayer1->getID());

    /// make sure, scene contains one layer
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)1, size) << "Scene should contain 1 layer";

    /// try to create existing layer 55, handle to existing layer should be returned
    pLayer2 = m_pScene->createLayer(expected, 0);
    ASSERT_EQ(expected, pLayer2->getID());

    /// make sure, scene still contains one layer
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)1, size) << "Scene should contain 1 layer";

    /// change opacity using first layer handle
    pLayer1->setOpacity(expectedOpacity);

    /// check opacity of layer using second handle
    EXPECT_DOUBLE_EQ(expectedOpacity, pLayer2->getOpacity());
}

TEST_F(SceneTest, createSurface)
{
    uint size;
    uint* array;
    Surface* pSurface;
    uint expected = 131;

    /// make sure, scene contains no surfaces
    m_pScene->getSurfaceIDs(&size, &array);
    ASSERT_EQ((uint)0, size) << "Scene should contain 0 surfaces";

    /// create surface with expected id
    pSurface = m_pScene->createSurface(expected, 0);
    ASSERT_TRUE(pSurface) << "Surface was not created.";

    /// make sure surface has expected id
    EXPECT_EQ(expected, pSurface->getID());

    /// make sure, scene contains one surface
    m_pScene->getSurfaceIDs(&size, &array);
    ASSERT_EQ((uint)1, size) << "Scene should contain 1 surface";

    /// make sure, surface was not added to any layer
    EXPECT_EQ(GraphicalObject::INVALID_ID, pSurface->getContainingLayerId());
}

TEST_F(SceneTest, createSurface_invalidId)
{
    uint size;
    uint* array;
    Surface* pSurface;

    /// make sure, scene contains no surfaces
    m_pScene->getSurfaceIDs(&size, &array);
    ASSERT_EQ((uint)0, size) << "Scene should contain 0 surfaces";

    /// create surface with invalid id
    pSurface = m_pScene->createSurface(GraphicalObject::INVALID_ID, 0);
    ASSERT_TRUE(pSurface) << "Surface was not created.";

    /// make sure new surface has valid id
    EXPECT_NE(GraphicalObject::INVALID_ID, pSurface->getID());

    /// make sure, scene contains one surface
    m_pScene->getSurfaceIDs(&size, &array);
    ASSERT_EQ((uint)1, size) << "Scene should contain 1 surface";
}

TEST_F(SceneTest, createSurface_twice)
{
    uint size;
    uint* array;
    Surface* pSurface1;
    Surface* pSurface2;
    uint expected = 135;
    double expectedOpacity = 0.718;

    /// make sure, scene contains no surfaces
    m_pScene->getSurfaceIDs(&size, &array);
    ASSERT_EQ((uint)0, size) << "Scene should contain 0 surfaces";

    /// create surface with expected id
    pSurface1 = m_pScene->createSurface(expected, 0);
    ASSERT_TRUE(pSurface1) << "Surface was not created.";

    /// make sure surface has expected id
    EXPECT_EQ(expected, pSurface1->getID());

    /// make sure, scene contains one surface
    m_pScene->getSurfaceIDs(&size, &array);
    ASSERT_EQ((uint)1, size) << "Scene should contain 1 surface";

    /// create surface with expected id again
    pSurface2 = m_pScene->createSurface(expected, 0);
    ASSERT_TRUE(pSurface2) << "Surface was not created.";

    /// make sure surface has expected id again
    EXPECT_EQ(expected, pSurface2->getID());

    /// make sure, scene still contains one surface
    m_pScene->getSurfaceIDs(&size, &array);
    ASSERT_EQ((uint)1, size) << "Scene should contain 1 surface";

    /// change opacity using first surface handle
    pSurface1->setOpacity(expectedOpacity);

    /// check opacity of surface using second surface handle
    EXPECT_DOUBLE_EQ(expectedOpacity, pSurface2->getOpacity());
}

TEST_F(SceneTest, removeLayer)
{
    uint expectedLayerId = 188;
    Layer* pLayer;

    /// create layer
    pLayer = m_pScene->createLayer(expectedLayerId, 0);
    ASSERT_TRUE(pLayer);

    /// make sure, layer exists
    ASSERT_EQ(pLayer, m_pScene->getLayer(expectedLayerId));

    /// remove layer
    m_pScene->removeLayer(pLayer);

    /// make sure, layer does not exist
    ASSERT_FALSE(m_pScene->getLayer(expectedLayerId));
}

TEST_F(SceneTest, removeSurface)
{
    uint expectedSurfaceId = 189;
    Surface* pSurface;

    /// create Surface
    pSurface = m_pScene->createSurface(expectedSurfaceId, 0);
    ASSERT_TRUE(pSurface);

    /// make sure, Surface exists
    ASSERT_EQ(pSurface, m_pScene->getSurface(expectedSurfaceId));

    /// remove Surface
    m_pScene->removeSurface(pSurface);

    /// make sure, Surface does not exist
    ASSERT_FALSE(m_pScene->getSurface(expectedSurfaceId));
}

TEST_F(SceneTest, getLayer)
{
    uint expectedLayerId = 198;
    Layer* pLayer;

    /// try to get non existing layer
    EXPECT_FALSE(m_pScene->getLayer(expectedLayerId));

    /// create layer
    pLayer = m_pScene->createLayer(expectedLayerId, 0);
    ASSERT_TRUE(pLayer);

    /// get layer
    ASSERT_EQ(pLayer, m_pScene->getLayer(expectedLayerId));

    /// remove layer
    m_pScene->removeLayer(pLayer);

    /// try to get removed layer
    ASSERT_FALSE(m_pScene->getLayer(expectedLayerId));
}

TEST_F(SceneTest, getSurface)
{
    uint expectedSurfaceId = 198;
    Surface* pSurface;

    /// try to get non existing Surface
    EXPECT_FALSE(m_pScene->getSurface(expectedSurfaceId));

    /// create Surface
    pSurface = m_pScene->createSurface(expectedSurfaceId, 0);
    ASSERT_TRUE(pSurface);

    /// get Surface
    ASSERT_EQ(pSurface, m_pScene->getSurface(expectedSurfaceId));

    /// remove Surface
    m_pScene->removeSurface(pSurface);

    /// try to get removed Surface
    ASSERT_FALSE(m_pScene->getSurface(expectedSurfaceId));
}

TEST_F(SceneTest, getLayerIDs)
{
    unsigned int layerId1 = 101;
    unsigned int layerId2 = 102;
    unsigned int layerId3 = 103;
    unsigned int layerId4 = 104;
    unsigned int size;
    unsigned int* array;

    /// make sure, scene contains no layers
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)0, size);

    /// create 4 layers in scene
    m_pScene->createLayer(layerId1, 0);
    m_pScene->createLayer(layerId2, 0);
    m_pScene->createLayer(layerId3, 0);
    m_pScene->createLayer(layerId4, 0);

    /// make sure, scene contains these 4 layers
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)4, size);
    EXPECT_EQ(layerId1, array[0]);
    EXPECT_EQ(layerId2, array[1]);
    EXPECT_EQ(layerId3, array[2]);
    EXPECT_EQ(layerId4, array[3]);
}

TEST_F(SceneTest, getLayerIDsOfScreen)
{
    unsigned int screenId = 0;
    unsigned int layerId1 = 121;
    unsigned int layerId2 = 122;
    unsigned int layerId3 = 123;
    unsigned int layerId4 = 124;
    unsigned int size;
    unsigned int* array;
    Layer* l1;
    Layer* l2;
    Layer* l3;
    Layer* l4;

    /// make sure, scene contains no layers
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)0, size);

    /// create 4 layers in scene, but dont add them to render order
    l1 = m_pScene->createLayer(layerId1, 0);
    l2 = m_pScene->createLayer(layerId2, 0);
    l3 = m_pScene->createLayer(layerId3, 0);
    l4 = m_pScene->createLayer(layerId4, 0);

    /// make sure, scene contains these 4 layers
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)4, size);
    EXPECT_EQ(layerId1, array[0]);
    EXPECT_EQ(layerId2, array[1]);
    EXPECT_EQ(layerId3, array[2]);
    EXPECT_EQ(layerId4, array[3]);

    /// make sure, screen still has no layers applied
    m_pScene->getLayerIDsOfScreen(screenId, &size, &array);
    ASSERT_EQ((uint)0, size);

    /// add 3 layers to screen
    LayerList& llist = m_pScene->getCurrentRenderOrder(screenId);
    llist.push_back(l1);
    llist.push_back(l3);
    llist.push_back(l4);

    /// make sure, screen now has 3 layers
    m_pScene->getLayerIDsOfScreen(screenId, &size, &array);
    ASSERT_EQ((uint)3, size);
    EXPECT_EQ(l1->getID(), array[0]);
    EXPECT_EQ(l3->getID(), array[1]);
    EXPECT_EQ(l4->getID(), array[2]);

    /// add 4th layer to screen
    llist.push_back(l2);

    /// make sure, screen now has 4 layers
    m_pScene->getLayerIDsOfScreen(screenId, &size, &array);
    ASSERT_EQ((uint)4, size);
    EXPECT_EQ(l1->getID(), array[0]);
    EXPECT_EQ(l3->getID(), array[1]);
    EXPECT_EQ(l4->getID(), array[2]);
    EXPECT_EQ(l2->getID(), array[3]);
}

TEST_F(SceneTest, getSurfaceIDs)
{
    unsigned int surfaceId1 = 141;
    unsigned int surfaceId2 = 142;
    unsigned int surfaceId3 = 143;
    unsigned int surfaceId4 = 144;
    unsigned int size;
    unsigned int* array;

    /// make sure, scene contains no surfaces
    m_pScene->getSurfaceIDs(&size, &array);
    ASSERT_EQ((uint)0, size);

    /// create 4 surfaces in scene
    m_pScene->createSurface(surfaceId1, 0);
    m_pScene->createSurface(surfaceId2, 0);
    m_pScene->createSurface(surfaceId3, 0);
    m_pScene->createSurface(surfaceId4, 0);

    /// make sure, scene contains these 4 surfaces
    m_pScene->getSurfaceIDs(&size, &array);
    ASSERT_EQ((uint)4, size);
    EXPECT_EQ(surfaceId1, array[0]);
    EXPECT_EQ(surfaceId2, array[1]);
    EXPECT_EQ(surfaceId3, array[2]);
    EXPECT_EQ(surfaceId4, array[3]);
}

TEST_F(SceneTest, DISABLED_lockScene)
{
}

TEST_F(SceneTest, DISABLED_unlockScene)
{
}

TEST_F(SceneTest, getCurrentRenderOrder)
{
    // TODO: how to test? return by typically reference can't be invalid.

    /// get render order
    LayerList& llist = m_pScene->getCurrentRenderOrder(0);

    /// make sure, list of layers is returned
    ASSERT_TRUE(&llist);
}

TEST_F(SceneTest, getAllSurfaces)
{
    uint expectedId1 = 241;
    uint expectedId2 = 242;
    uint expectedId3 = 243;
    Surface* s1;
    Surface* s2;
    Surface* s3;

    /// try to get surfaces, when no surfaces exist in scene
    ASSERT_EQ((uint)0, m_pScene->getAllSurfaces().size());

    /// add 3 surfaces to scene
    s1 = m_pScene->createSurface(expectedId1, 0);
    s2 = m_pScene->createSurface(expectedId2, 0);
    s3 = m_pScene->createSurface(expectedId3, 0);

    /// get all surfaces
    const SurfaceMap& smap1 = m_pScene->getAllSurfaces();
    ASSERT_EQ((uint)3, smap1.size());
    // order is undefined here, but each surface must be contained once
    EXPECT_EQ((uint)1, smap1.count(expectedId1));
    EXPECT_EQ((uint)1, smap1.count(expectedId2));
    EXPECT_EQ((uint)1, smap1.count(expectedId3));

    /// remove 2 surfaces again
    m_pScene->removeSurface(s1);
    m_pScene->removeSurface(s3);

    /// check, if the remaining surface is the expected one
    const SurfaceMap& smap2 = m_pScene->getAllSurfaces();
    ASSERT_EQ((uint)1, smap2.size());
    EXPECT_EQ((uint)1, smap2.count(expectedId2));
    EXPECT_EQ(s2, smap2.at(expectedId2));
}


TEST_F(SceneTest, isLayerInCurrentRenderOrder)
{
    unsigned int layerId1 = 121;
    unsigned int size;
    unsigned int* array;
    Layer* l1;

    /// create layer in scene, but dont add it to render order
    l1 = m_pScene->createLayer(layerId1, 0);

    /// make sure, scene contains the new layers
    m_pScene->getLayerIDs(&size, &array);
    ASSERT_EQ((uint)1, size);
    EXPECT_EQ(layerId1, array[0]);

    /// make sure, layer is not in render order
    ASSERT_FALSE(m_pScene->isLayerInCurrentRenderOrder(layerId1));

    /// add layer to render order
    LayerList& llist = m_pScene->getCurrentRenderOrder(0);
    llist.push_back(l1);

    /// make sure, layer is in render order
    ASSERT_TRUE(m_pScene->isLayerInCurrentRenderOrder(layerId1));

    /// remove layer from render order
    llist.remove(l1);

    /// make sure, layer is not in render order
    ASSERT_FALSE(m_pScene->isLayerInCurrentRenderOrder(layerId1));
}
