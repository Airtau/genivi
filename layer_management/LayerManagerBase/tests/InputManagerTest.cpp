/***************************************************************************
*
* Copyright 2012 Valeo
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
#include "InputManager.h"

class InputManagerTest : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pScene = new Scene();
        ASSERT_TRUE(m_pScene);

        m_pInputManager = new InputManager(m_pScene);
        ASSERT_TRUE(m_pInputManager);
    }

    void TearDown()
    {
        if (m_pScene)
        {
            delete m_pScene;
            m_pScene = 0;
        }
    }

    IScene* m_pScene;
    InputManager * m_pInputManager;
};



#define DUMMY_NATIVE_CONTENT   42

/*
 * Layer(1):
 *
 *     (0,0)       (100,0)                                           (800,0)
 *           +--------------------------------------------------------+
 *           |        |       Surface30                               |
 *           |        |                                               |
 *    (0,50) +--------------------------------------------------------+
 *           |        |                                               |
 *           |   S    |                                               |
 *           |   u    |                                               |
 *           |   f    |                                               |
 *           |   a    |        Surface 10                             |
 *           |   c    |                                               |
 *           |   e    |                                               |
 *           |   20   |                                               |
 *           |        |                                               |
 *           |        |                                               |
 *           |        |                                               |
 *           |        |                                               |
 *           |        |                                               |
 *           |        |                                               |
 *   (0,480) +--------+-----------------------------------------------+
 *
 * Surface10 can be seen as a layer wide background
 * Surface20 can be seen as a menu bar, taking all of the height of the layer
 * Surface30 can be seen as a status bar, taking all of the width of the layer
 *
 * Surface order : 10 -> 20 -> 30
 */
#define CPLX_SCREEN_WIDTH  800
#define CPLX_SCREEN_HEIGHT 480


#define CPLX_SCREEN_LAY1_ID                  1u
#define CPLX_SCREEN_LAY1_SURF_BACKGROUND_ID  10u
#define CPLX_SCREEN_LAY1_SURF_MENUBAR_ID     20u
#define CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID   30u

#define CPLX_SCREEN_LAY1_MENUBAR_WIDTH 100
#define CPLX_SCREEN_LAY1_STATUSBAR_HEIGHT 50

static Layer* createComplexLayer_1(IScene *m_pScene)
{
    Layer* pLayer;
    Surface* pSurface;

    pLayer = m_pScene->createLayer(CPLX_SCREEN_LAY1_ID, 0);
    pLayer->OriginalSourceWidth = CPLX_SCREEN_WIDTH;
    pLayer->OriginalSourceHeight = CPLX_SCREEN_HEIGHT;
    pLayer->setSourceRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_HEIGHT));
    pLayer->setDestinationRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_HEIGHT));
    pLayer->setOpacity(1.0);
    pLayer->setVisibility(true);


    /* Surface 10 : Background */
    pSurface = m_pScene->createSurface(CPLX_SCREEN_LAY1_SURF_BACKGROUND_ID, 0);
    pSurface->OriginalSourceWidth = CPLX_SCREEN_WIDTH;
    pSurface->OriginalSourceHeight = CPLX_SCREEN_HEIGHT;
    pSurface->setSourceRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_HEIGHT));
    pSurface->setDestinationRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_HEIGHT));
    pSurface->setOpacity(1.0);
    pSurface->setVisibility(true);
    pSurface->setNativeContent(DUMMY_NATIVE_CONTENT);
    pLayer->addSurface(pSurface);

    /* Surface 20 : Menubar */
    pSurface = m_pScene->createSurface(CPLX_SCREEN_LAY1_SURF_MENUBAR_ID, 0);
    pSurface->OriginalSourceWidth = CPLX_SCREEN_LAY1_MENUBAR_WIDTH;
    pSurface->OriginalSourceHeight = CPLX_SCREEN_HEIGHT;
    pSurface->setSourceRegion(Rectangle(0, 0, CPLX_SCREEN_LAY1_MENUBAR_WIDTH, CPLX_SCREEN_HEIGHT));
    pSurface->setDestinationRegion(Rectangle(0, 0, CPLX_SCREEN_LAY1_MENUBAR_WIDTH, CPLX_SCREEN_HEIGHT));
    pSurface->setOpacity(1.0);
    pSurface->setVisibility(true);
    pSurface->setNativeContent(DUMMY_NATIVE_CONTENT);
    pLayer->addSurface(pSurface);

    /* Surface 20 : Status */
    pSurface = m_pScene->createSurface(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID, 0);
    pSurface->OriginalSourceWidth = CPLX_SCREEN_WIDTH;
    pSurface->OriginalSourceHeight = CPLX_SCREEN_HEIGHT;
    pSurface->setSourceRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_LAY1_STATUSBAR_HEIGHT));
    pSurface->setDestinationRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_LAY1_STATUSBAR_HEIGHT));
    pSurface->setOpacity(1.0);
    pSurface->setVisibility(true);
    pSurface->setNativeContent(DUMMY_NATIVE_CONTENT);
    pLayer->addSurface(pSurface);


    m_pScene->getCurrentRenderOrder(0).push_back(pLayer);
    return pLayer;
}




/*
 * Layer(2):
 *
 *     (0,0)                                                       (800,0)
 *           +--------------------------------------------------------+
 *           |                                                        |
 *           |                    Surface100                          |
 *           |    (100,50)                                            |
 *           |        +-----------------------------------------------+
 *           |        |                                               |
 *           |        |                Surface200                     |
 *           |        |                                               |
 *           |        |                                               |
 *           |(50,200)|                                               |
 *           |    +----------------------------------------------+    |
 *           |    |                                              |    |
 *           |    |               Surface300                     |    |
 *           |    |                                              |    |
 *           |    +----------------------------------------------+    |
 *           |        |                                     (750,250) |
 *           |        |                                               |
 *           |        |                                               |
 *           |        |                                               |
 *           |        |                                               |
 *   (0,480) +--------+-----------------------------------------------+
 *                 (50,480)                                       (800,480)
 *
 * Surface100 can be seen as a layer wide background, not visible
 * Surface200 can be seen as a content surface (nav, phone, ...), taking all of the layer minus menu & status bar from layer 1
 * Surface300 can be seen as a popup
 *
 * Surface order : 100 -> 200 -> 300
 */

#define CPLX_SCREEN_LAY2_ID                  2u

#define CPLX_SCREEN_LAY2_SURF_BACKGROUND_ID  100u
#define CPLX_SCREEN_LAY2_SURF_CONTENT_ID     200u
#define CPLX_SCREEN_LAY2_SURF_POPUP_ID       300u


#define CPLX_SCREEN_LAY2_CONTENT_X          CPLX_SCREEN_LAY1_MENUBAR_WIDTH
#define CPLX_SCREEN_LAY2_CONTENT_WIDTH      (CPLX_SCREEN_WIDTH - CPLX_SCREEN_LAY2_CONTENT_X)
#define CPLX_SCREEN_LAY2_CONTENT_Y          CPLX_SCREEN_LAY1_STATUSBAR_HEIGHT
#define CPLX_SCREEN_LAY2_CONTENT_HEIGHT     (CPLX_SCREEN_HEIGHT - CPLX_SCREEN_LAY2_CONTENT_Y)

#define CPLX_SCREEN_LAY2_POPUP_X            50
#define CPLX_SCREEN_LAY2_POPUP_WIDTH        700
#define CPLX_SCREEN_LAY2_POPUP_Y            200
#define CPLX_SCREEN_LAY2_POPUP_HEIGHT       50

static Layer* createComplexLayer_2(IScene *m_pScene)
{
    Layer* pLayer;
    Surface* pSurface;

    pLayer = m_pScene->createLayer(CPLX_SCREEN_LAY2_ID, 0);
    pLayer->OriginalSourceWidth = CPLX_SCREEN_WIDTH;
    pLayer->OriginalSourceHeight = CPLX_SCREEN_HEIGHT;
    pLayer->setSourceRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_HEIGHT));
    pLayer->setDestinationRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_HEIGHT));
    pLayer->setOpacity(1.0);
    pLayer->setVisibility(true);

    /* Surface 100 : Background */
    pSurface = m_pScene->createSurface(CPLX_SCREEN_LAY2_SURF_BACKGROUND_ID, 0);
    pSurface->OriginalSourceWidth = CPLX_SCREEN_WIDTH;
    pSurface->OriginalSourceHeight = CPLX_SCREEN_HEIGHT;
    pSurface->setSourceRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_HEIGHT));
    pSurface->setDestinationRegion(Rectangle(0, 0, CPLX_SCREEN_WIDTH, CPLX_SCREEN_HEIGHT));
    pSurface->setOpacity(1.0);
    pSurface->setVisibility(false);
    pSurface->setNativeContent(DUMMY_NATIVE_CONTENT);
    pLayer->addSurface(pSurface);

    /* Surface 200 : Content */
    pSurface = m_pScene->createSurface(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, 0);
    pSurface->OriginalSourceWidth = CPLX_SCREEN_LAY2_CONTENT_WIDTH;
    pSurface->OriginalSourceHeight = CPLX_SCREEN_LAY2_CONTENT_HEIGHT;
    pSurface->setSourceRegion(Rectangle(0, 0, CPLX_SCREEN_LAY2_CONTENT_WIDTH, CPLX_SCREEN_LAY2_CONTENT_HEIGHT));
    pSurface->setDestinationRegion(Rectangle(CPLX_SCREEN_LAY2_CONTENT_X, CPLX_SCREEN_LAY2_CONTENT_Y, CPLX_SCREEN_LAY2_CONTENT_WIDTH, CPLX_SCREEN_LAY2_CONTENT_HEIGHT));
    pSurface->setOpacity(1.0);
    pSurface->setVisibility(true);
    pSurface->setNativeContent(DUMMY_NATIVE_CONTENT);
    pLayer->addSurface(pSurface);

    /* Surface 300 : Popup */
    pSurface = m_pScene->createSurface(CPLX_SCREEN_LAY2_SURF_POPUP_ID, 0);
    pSurface->OriginalSourceWidth = CPLX_SCREEN_LAY2_POPUP_WIDTH;
    pSurface->OriginalSourceHeight = CPLX_SCREEN_LAY2_POPUP_HEIGHT;
    pSurface->setSourceRegion(Rectangle(0, 0, CPLX_SCREEN_LAY2_POPUP_WIDTH, CPLX_SCREEN_LAY2_POPUP_HEIGHT));
    pSurface->setDestinationRegion(Rectangle(CPLX_SCREEN_LAY2_POPUP_X, CPLX_SCREEN_LAY2_POPUP_Y, CPLX_SCREEN_LAY2_POPUP_WIDTH, CPLX_SCREEN_LAY2_POPUP_HEIGHT));
    pSurface->setOpacity(1.0);
    pSurface->setVisibility(true);
    pSurface->setNativeContent(DUMMY_NATIVE_CONTENT);
    pLayer->addSurface(pSurface);


    m_pScene->getCurrentRenderOrder(0).push_back(pLayer);
    return pLayer;
}


static void createComplexScene(IScene *m_pScene)
{
    createComplexLayer_1(m_pScene);
    createComplexLayer_2(m_pScene);
}


/**
 * This test mostly validate the internal function createComplexScene. No functional requirements are validated here.
 */
TEST_F(InputManagerTest, CreateComplexeScene)
{
    Layer* pLayer;

    createComplexScene(m_pScene);

    LayerList& ll = m_pScene->getCurrentRenderOrder(0);
    EXPECT_EQ(ll.size(), (uint)2);

    // top layer to be rendered is LAY2
    pLayer = ll.back();
    EXPECT_EQ(CPLX_SCREEN_LAY2_ID, pLayer->getID());
    // Layer2 render order is, from top to bottom : PopUp, Content, background
    SurfaceList& sf2 = pLayer->getAllSurfaces();
    EXPECT_EQ((uint)3, sf2.size());
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_POPUP_ID, sf2.back()->getID());
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_BACKGROUND_ID, sf2.front()->getID());
    EXPECT_TRUE(pLayer->getVisibility());
    EXPECT_NE((double)0, pLayer->getOpacity());

    // bottom layer to be rendered is LAY1
    pLayer = ll.front();
    EXPECT_EQ(CPLX_SCREEN_LAY1_ID, pLayer->getID());
    SurfaceList& sf1 = pLayer->getAllSurfaces();
    EXPECT_EQ((uint)3, sf1.size());
    EXPECT_EQ(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID, sf1.back()->getID());
    EXPECT_EQ(CPLX_SCREEN_LAY1_SURF_BACKGROUND_ID, sf1.front()->getID());
    EXPECT_TRUE(pLayer->getVisibility());
    EXPECT_NE((double)0, pLayer->getOpacity());
}

TEST_F(InputManagerTest, AtStartup)
{
    Surface* surf;
    createComplexScene(m_pScene);

    // Keyboard focus set to none
    EXPECT_EQ(GraphicalObject::INVALID_ID, m_pInputManager->getKeyboardFocusSurfaceId());
    m_pInputManager->setKeyboardFocusOn(CPLX_SCREEN_LAY2_SURF_CONTENT_ID);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, m_pInputManager->getKeyboardFocusSurfaceId());

    // All surfaces accept events from all kind of devices

    // CPLX_SCREEN_LAY1_SURF_BACKGROUND_ID
    surf = m_pScene->getSurface(CPLX_SCREEN_LAY1_SURF_BACKGROUND_ID);
    EXPECT_EQ(INPUT_DEVICE_ALL, surf->getInputEventAcceptanceOnDevices());
    // CPLX_SCREEN_LAY1_SURF_MENUBAR_ID
    surf = m_pScene->getSurface(CPLX_SCREEN_LAY1_SURF_MENUBAR_ID);
    EXPECT_EQ(INPUT_DEVICE_ALL, surf->getInputEventAcceptanceOnDevices());
    // CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID
    surf = m_pScene->getSurface(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID);
    EXPECT_EQ(INPUT_DEVICE_ALL, surf->getInputEventAcceptanceOnDevices());
    // CPLX_SCREEN_LAY2_SURF_BACKGROUND_ID
    surf = m_pScene->getSurface(CPLX_SCREEN_LAY2_SURF_BACKGROUND_ID);
    EXPECT_EQ(INPUT_DEVICE_ALL, surf->getInputEventAcceptanceOnDevices());
    // CPLX_SCREEN_LAY2_SURF_CONTENT_ID
    surf = m_pScene->getSurface(CPLX_SCREEN_LAY2_SURF_CONTENT_ID);
    EXPECT_EQ(INPUT_DEVICE_ALL, surf->getInputEventAcceptanceOnDevices());
    // CPLX_SCREEN_LAY2_SURF_POPUP_ID
    surf = m_pScene->getSurface(CPLX_SCREEN_LAY2_SURF_POPUP_ID);
    EXPECT_EQ(INPUT_DEVICE_ALL, surf->getInputEventAcceptanceOnDevices());
}

/**
 * @ref<LM_INPUT_REQ_02>
 *   Keyboard pressed events will be dispatched to the surface elected as being the "keyboard focused" surface.
 * @ref<LM_INPUT_REQ_03>
 *   A surface gain the Keyboard focus if the LM command "surfaceSetKeyboardFocus"
 *   is called on that suface. The elected surface can be changed at any time.
 */
TEST_F(InputManagerTest, KeyboardEvent_Pressed)
{
    Surface* surf;

    createComplexScene(m_pScene);

    // First key pressed, but focus not set on any surface
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 42ul);
    EXPECT_FALSE(surf);

    // Set kbd focus on a surface
    m_pInputManager->setKeyboardFocusOn(CPLX_SCREEN_LAY2_SURF_CONTENT_ID);
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 42ul);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    // Set kbd focus on an other surface
    m_pInputManager->setKeyboardFocusOn(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID);
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 43ul);
    EXPECT_EQ(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID, surf->getID());
}

/**
 * @ref<LM_INPUT_REQ_02>
 *   Keyboard pressed events will be dispatched to the surface elected as being
 *   the "keyboard focused" surface.
 *   Keyboard released events will be dispatched to the surface which received
 *   the same key pressed event.
 * @ref<LM_INPUT_REQ_03>
 *   A surface gain the Keyboard focus if the LM command "surfaceSetKeyboardFocus"
 *   is called on that suface. The elected surface can be changed at any time.
 */
TEST_F(InputManagerTest, KeyboardEvent_Relased)
{
    Surface* surf;

    createComplexScene(m_pScene);

    // Send a 1st key released without any key pressed before and no focus
    // Should never happen in real case, just a robustness test
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_RELEASED, 1000ul);
    EXPECT_EQ(NULL, surf);

    // Set kbd focus on a surface
    m_pInputManager->setKeyboardFocusOn(CPLX_SCREEN_LAY2_SURF_CONTENT_ID);

    // Send a 1st key released without any key pressed before
    // Should never happen in real case, just a robustness test
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_RELEASED, 2000ul);
    EXPECT_EQ(NULL, surf);

    // Send a key pressed (key id == 10)
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 10ul);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    // Send an other key pressed event (key id == 20)
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 20ul);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    // Send an other key pressed event (key id == 30)
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 30ul);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    // Send a 2nd key pressed with (key id == 30).
    // Should never happen in real case (2 consecutive pressed)
    // But just to make to make there is no problem
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 30ul);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    // Key released (key id == 30).
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_RELEASED, 30ul);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    // Set kbd focus on an other surface
    m_pInputManager->setKeyboardFocusOn(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID);

    // Send a key pressed event (key id == 30)
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 30ul);
    EXPECT_EQ(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID, surf->getID());

    // Now released previously key pressed. (10 & 20)
    // Elected surface should be the one which receive the pressed event, not the
    // one which has the focus
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_RELEASED, 20ul);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_RELEASED, 10ul);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

}


/**
 * @ref<LM_INPUT_REQ_03>
 *   A surface gain the Keyboard focus if the LM command "surfaceSetKeyboardFocus"
 *   is called on that suface. The elected surface can be changed at any time.
 *
 * @ref<LM_INPUT_REQ_06>
 * A surface can request to not receive particular input events. In this case, the surface should not be considered for focus election & the events
 *  must be dispatched to an other surface, if relevant. *
 *  Only the Keyboard Event are tested here.
 */
TEST_F(InputManagerTest, KeyboardEvent_InputEventAcceptance)
{
    bool ret;
    Surface * surf;
    Surface * pContent;

    createComplexScene(m_pScene);

    pContent = m_pScene->getSurface(CPLX_SCREEN_LAY2_SURF_CONTENT_ID);
    pContent->updateInputEventAcceptanceFrom(INPUT_DEVICE_KEYBOARD, false);

    // Set focus on a surface which accept kbd event
    ret = m_pInputManager->setKeyboardFocusOn(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID);
    EXPECT_TRUE(ret);
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 42ul);
    EXPECT_TRUE(surf);

    ret = m_pInputManager->setKeyboardFocusOn(CPLX_SCREEN_LAY2_SURF_CONTENT_ID);
    EXPECT_FALSE(ret);
    surf = m_pInputManager->reportKeyboardEvent(INPUT_STATE_PRESSED, 42ul);
    EXPECT_FALSE(surf);

}




/**
 * @ref<LM_INPUT_REQ_05>
 * If first event to be reported is not Pressed, then no surface has the focus
 */
TEST_F(InputManagerTest, PointerEvent_Focus_On_Pressed)
{
    Surface* surf;
    Point p;

    createComplexScene(m_pScene);

    p = (Point) {INPUT_STATE_MOTION, 42, 43};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(surf, (Surface*)NULL);  // No focus, so NULL is returned
    EXPECT_EQ(42, p.x);         // Coordinates are kept unchanged
    EXPECT_EQ(43, p.y);

    p = (Point) {INPUT_STATE_RELEASED, 44, 45};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ((Surface*)NULL, surf);
    EXPECT_EQ(44, p.x);
    EXPECT_EQ(45, p.y);

    p = (Point) {INPUT_STATE_OTHER, 46, 47};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ((Surface*)NULL, surf);
    EXPECT_EQ(46, p.x);
    EXPECT_EQ(47, p.y);

    // Pressed somewhere in Content
    p = (Point) {INPUT_STATE_PRESSED, 700, 400};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_NE(surf, (Surface*)NULL);  // Pressed under a surface, so not NULL is returned
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());  // Make sure the elected surface is the appropriate one
    EXPECT_NE(700, p.x);  // Coordinates must have changed
    EXPECT_NE(400, p.y);
}

/**
 * @ref<LM_INPUT_REQ_05>
 * As soon as a surface is elected as pointed surface, all events other than pressed go to that surface
 */
TEST_F(InputManagerTest, PointerEvent_Focus_Remain)
{
    Surface* surf;
    Point p;

    createComplexScene(m_pScene);

    // Pressed somewhere in Content
    p = (Point) {INPUT_STATE_PRESSED, 700, 400};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_NE(surf, (Surface*)NULL);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());
    EXPECT_NE(700, p.x);
    EXPECT_NE(400, p.y);

    // motion somewhere outside of Content
    p = (Point) {INPUT_STATE_MOTION, 10, 20};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());
    EXPECT_NE(10, p.x);
    EXPECT_NE(20, p.y);

    // motion somewhere outside of Content
    p = (Point) {INPUT_STATE_MOTION, 0, 200};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());
    EXPECT_NE(0, p.x);
    EXPECT_NE(200, p.y);

    // release somewhere outside of Content
    p = (Point) {INPUT_STATE_RELEASED, 500, 30};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());
    EXPECT_NE(500, p.x);
    EXPECT_NE(30, p.y);

    // Let's do some click on the background layer

    // Pressed somewhere in Status bar
    p = (Point) {INPUT_STATE_PRESSED, 100, 30};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID, surf->getID());
    EXPECT_EQ(100, p.x);
    EXPECT_EQ(30, p.y);

    // Motion somewhere outside of Status bar
    p = (Point) {INPUT_STATE_MOTION, 500, 500};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID, surf->getID());

    // Released somewhere outside of Status bar
    p = (Point) {INPUT_STATE_RELEASED, 800, 480};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY1_SURF_STATUSBAR_ID, surf->getID());
}


/**
 * @ref<LM_INPUT_REQ_05>
 *  The conditions to gain this status is that the surface or its containing layer should:
 *     + be visible
 *     + be opaque (opacity != 0).
 *     + has a native content
 *
 *  Only the surface conditions are tested here
 */
TEST_F(InputManagerTest, PointerEvent_Focus_Conditions)
{
    Point p;
    Surface* surf;
    Surface* m_pPopup;

    createComplexScene(m_pScene);

    m_pPopup = m_pScene->getSurface(CPLX_SCREEN_LAY2_SURF_POPUP_ID);
    m_pPopup->setOpacity(1);
    m_pPopup->setVisibility(true);
    m_pPopup->setNativeContent(DUMMY_NATIVE_CONTENT);

    // (100,225) is in the middle of the popup
    p = (Point) {INPUT_STATE_PRESSED, 100, 225};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_POPUP_ID, surf->getID());

    // popup not visible
    m_pPopup->setVisibility(false);
    p = (Point) {INPUT_STATE_PRESSED, 100, 225};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    // popup visible and small opacity
    m_pPopup->setVisibility(true);
    m_pPopup->setOpacity(0.1);
    p = (Point) {INPUT_STATE_PRESSED, 100, 225};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_POPUP_ID, surf->getID());

    // popup visible and no opacity
    m_pPopup->setVisibility(true);
    m_pPopup->setOpacity(0);
    p = (Point) {INPUT_STATE_PRESSED, 100, 225};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    // popup visible, with opacity, but no native content
    m_pPopup->setVisibility(true);
    m_pPopup->setOpacity(1);
    m_pPopup->removeNativeContent();
    p = (Point) {INPUT_STATE_PRESSED, 100, 225};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

}


/**
 * @ref<LM_INPUT_REQ_05>
 *  The conditions to gain this status is that the surface or its containing layer should:
 *     + be visible
 *     + be opaque (opacity != 0).
 *     + has a native content
 *
 *  Only the layer conditions are tested here
 */
TEST_F(InputManagerTest, PointerEvent_Election_Conditions_Check)
{
    Point p;
    Surface* surf;
    Layer* layerTop;

    createComplexScene(m_pScene);

    layerTop = m_pScene->getLayer(CPLX_SCREEN_LAY2_ID);

    // Pressed somewhere in Content
    p = (Point) {INPUT_STATE_PRESSED, 700, 400};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_NE(surf, (Surface*)NULL);
    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    // No visibility
    layerTop->setVisibility(false);
    // Pressed somewhere in Content
    p = (Point) {INPUT_STATE_PRESSED, 700, 400};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_NE(surf, (Surface*)NULL);
    EXPECT_EQ(CPLX_SCREEN_LAY1_SURF_BACKGROUND_ID, surf->getID());


    // Visibility but no opacity
    layerTop->setVisibility(true);
    layerTop->setOpacity(0);
    // Pressed somewhere in Content
    p = (Point) {INPUT_STATE_PRESSED, 700, 400};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_NE(surf, (Surface*)NULL);
    EXPECT_EQ(CPLX_SCREEN_LAY1_SURF_BACKGROUND_ID, surf->getID());
}



/**
 * @ref<LM_INPUT_REQ_04>
 *  Coordinates will be adjusted relatively to the surface.
 */
TEST_F(InputManagerTest, PointerEvent_Coordinates_translation)
{
    Point p;
    Surface* surf;
    Surface* newSurf;
    Layer* layerTop;

    static const unsigned int sid = 42;  // surface id
    static const unsigned int sw = 200;  // surface width
    static const unsigned int sh = 100;  // surface height
    createComplexScene(m_pScene);

    layerTop = m_pScene->getLayer(CPLX_SCREEN_LAY2_ID);

    // Let's create a new surface & add it to the top layer
    newSurf = m_pScene->createSurface(sid, 0);
    newSurf->OriginalSourceWidth = sw;
    newSurf->OriginalSourceHeight = sh;
    newSurf->setSourceRegion(Rectangle(0, 0, sw, sh));
    newSurf->setDestinationRegion(Rectangle(0, 0, sw, sh));
    newSurf->setOpacity(1.0);
    newSurf->setVisibility(true);
    newSurf->setNativeContent(DUMMY_NATIVE_CONTENT);
    layerTop->addSurface(newSurf);

    // Waouh,the surface is now rendered at (0,0) without any transformation, scaling, croping, ...

    // Let send a first pointer pressed to get it's focus
    // Since no transformation, screen wide (10,20) coordinate should become ... (10,20) in the surface coordinate system
    p = (Point) {INPUT_STATE_PRESSED, 10, 20};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(sid, surf->getID());
    EXPECT_EQ(10, p.x);
    EXPECT_EQ(20, p.y);

    // Let's move newSurf by 10 to the left and 100 down and retry the same test
    newSurf->setDestinationRegion(Rectangle(10, 100, sw, sh));
    p = (Point) {INPUT_STATE_PRESSED, 10, 150};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(sid, surf->getID());
    EXPECT_EQ(0, p.x);  // 10 global is 0 local
    EXPECT_EQ(50, p.y); // 150 global is 50 local

    // motion at (0,40) global should still elect newSurf at (-10,-60)
    p = (Point) {INPUT_STATE_MOTION, 0, 40};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(sid, surf->getID());
    EXPECT_EQ(-10, p.x);  // 10 global is 0 local
    EXPECT_EQ(-60, p.y);

    // release at (800,100) global should still elect newSurf at (790,0)
    p = (Point) {INPUT_STATE_RELEASED, 800, 100};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(sid, surf->getID());
    EXPECT_EQ(790, p.x);  // 10 global is 0 local
    EXPECT_EQ(0, p.y);
}


/**
 * @ref<LM_INPUT_REQ_04>
 *  Coordinates will be adjusted relatively to the surface.
 */
TEST_F(InputManagerTest, PointerEvent_Coordinates_croping)
{
    Point p;
    Surface* surf;
    Surface* newSurf;
    Layer* layerTop;

    static const unsigned int sid = 42;  // surface id
    static const unsigned int sw = 200;  // surface width
    static const unsigned int sh = 100;  // surface height
    createComplexScene(m_pScene);

    layerTop = m_pScene->getLayer(CPLX_SCREEN_LAY2_ID);

    // Let's create a new surface & add it to the top layer
    newSurf = m_pScene->createSurface(sid, 0);
    newSurf->OriginalSourceWidth = sw;
    newSurf->OriginalSourceHeight = sh;
    newSurf->setSourceRegion(Rectangle(0, 0, sw, sh));
    newSurf->setDestinationRegion(Rectangle(0, 0, sw, sh));
    newSurf->setOpacity(1.0);
    newSurf->setVisibility(true);
    newSurf->setNativeContent(DUMMY_NATIVE_CONTENT);
    layerTop->addSurface(newSurf);
    // Waouh,the surface is now rendered at (0,0) without any transformation, scaling, croping, ...

    // Crope newSurf, no scalling
    newSurf->setSourceRegion(Rectangle(50, 50, 50, 50));
    newSurf->setDestinationRegion(Rectangle(0, 0, 50, 50));

    p = (Point) {INPUT_STATE_PRESSED, 10, 20};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(sid, surf->getID());
    EXPECT_EQ(60, p.x);
    EXPECT_EQ(70, p.y);

    // Let's move newSurf by 10 to the left and 100 down and retry the same test
    newSurf->setDestinationRegion(Rectangle(10, 100, 50, 50));
    p = (Point) {INPUT_STATE_PRESSED, 20, 120};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(sid, surf->getID());
    EXPECT_EQ(60, p.x);
    EXPECT_EQ(70, p.y);

    // motion at (10,40) global should still elect newSurf at (40,-10)
   p = (Point) {INPUT_STATE_MOTION, 0, 40};
   surf = m_pInputManager->reportPointerEvent(p);
   EXPECT_EQ(sid, surf->getID());
   EXPECT_EQ(40, p.x);  // 10 global is 0 local
   EXPECT_EQ(-10, p.y);
}



/**
 * @ref<LM_INPUT_REQ_04>
 *  Coordinates will be adjusted relatively to the surface.
 */
TEST_F(InputManagerTest, PointerEvent_Coordinates_scaling)
{
    Point p;
    Surface* surf;
    Surface* newSurf;
    Layer* layerTop;

    static const unsigned int sid = 42;     // surface id
    static const unsigned int sw = 200;  // surface width
    static const unsigned int sh = 100;  // surface height
    createComplexScene(m_pScene);

    layerTop = m_pScene->getLayer(CPLX_SCREEN_LAY2_ID);

    // Let's create a new surface & add it to the top layer
    newSurf = m_pScene->createSurface(sid, 0);
    newSurf->OriginalSourceWidth = sw;
    newSurf->OriginalSourceHeight = sh;
    newSurf->setSourceRegion(Rectangle(0, 0, sw, sh));
    newSurf->setDestinationRegion(Rectangle(0, 0, sw, sh));
    newSurf->setOpacity(1.0);
    newSurf->setVisibility(true);
    newSurf->setNativeContent(DUMMY_NATIVE_CONTENT);
    layerTop->addSurface(newSurf);
    // Waouh,the surface is now rendered at (0,0) without any transformation, scaling, croping, ...

    // Scale newSurf x2
    newSurf->setSourceRegion(Rectangle(0, 0, sw/2, sh/2));
    newSurf->setDestinationRegion(Rectangle(0, 0, sw, sh));

    p = (Point) {INPUT_STATE_PRESSED, 10, 20};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(sid, surf->getID());
    EXPECT_EQ(5, p.x);
    EXPECT_EQ(10, p.y);

    // Scale & Move newSurf
    newSurf->setDestinationRegion(Rectangle(10, 100, sw, sh));
    p = (Point) {INPUT_STATE_PRESSED, 20, 120};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(sid, surf->getID());
    EXPECT_EQ(5, p.x);
    EXPECT_EQ(10, p.y);

    // Scale & Move & Crope surface
    newSurf->setSourceRegion(Rectangle(10, 10, sw/2, sh/2));
    p = (Point) {INPUT_STATE_PRESSED, 30, 140};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ(sid, surf->getID());
    EXPECT_EQ(20, p.x);
    EXPECT_EQ(30, p.y);
}



/**
 * @ref<LM_INPUT_REQ_06>
 * A surface can request to not receive particular input events. In this case, the surface should not be considered for focus election & the events
 *  must be dispatched to an other surface, if relevant. *
 *  Only the PointerEvent are tested here.
 */
TEST_F(InputManagerTest, PointerEvent_InputEventAcceptance)
{
    Point p;
    Surface* surf;
    Surface* pPopup;
    Surface* pContent;
    Surface* pBackground;

    createComplexScene(m_pScene);

    pPopup = m_pScene->getSurface(CPLX_SCREEN_LAY2_SURF_POPUP_ID);
    pPopup->setOpacity(1);
    pPopup->setVisibility(true);

    // (100,225) is in the middle of the popup
    p = (Point) {INPUT_STATE_PRESSED, 100, 225};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ((uint)CPLX_SCREEN_LAY2_SURF_POPUP_ID, surf->getID());

    pPopup->updateInputEventAcceptanceFrom(INPUT_DEVICE_POINTER, false);

    // (100,225) is in the middle of the popup, but popup now refuse pointer event
    p = (Point) {INPUT_STATE_PRESSED, 100, 225};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_NE((uint)CPLX_SCREEN_LAY2_SURF_POPUP_ID, surf->getID());
    EXPECT_EQ((uint)CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());

    pContent = m_pScene->getSurface(CPLX_SCREEN_LAY2_SURF_CONTENT_ID);
    pContent->updateInputEventAcceptanceFrom(INPUT_DEVICE_POINTER, false);

    // (100,225) is in the middle of the popup, but popup refuse pointer event
    // so event should be dispatched to Content, but content now refuse pointer event
    // Finally event should be dispatched to Background of Layer 1
    p = (Point) {INPUT_STATE_PRESSED, 100, 225};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_EQ((uint)CPLX_SCREEN_LAY1_SURF_BACKGROUND_ID, surf->getID());

    pBackground = m_pScene->getSurface(CPLX_SCREEN_LAY1_SURF_BACKGROUND_ID);
    pBackground->updateInputEventAcceptanceFrom(INPUT_DEVICE_POINTER, false);

    // (100,225) is in the middle of the popup.
    // popup -> content -> Background -> NULL
    p = (Point) {INPUT_STATE_PRESSED, 100, 225};
    surf = m_pInputManager->reportPointerEvent(p);
    EXPECT_FALSE(surf);

}

/**
 * @ref<LM_INPUT_REQ_04>
 *  Coordinates will be adjusted relatively to the surface.
 *  Test for touch event
 */
TEST_F(InputManagerTest, TouchEvent)
{
    Point p;
    Surface* surf;
    PointVect pv;

    createComplexScene(m_pScene);

    // In real use case (WindowSystem), PointVect vector will mostly be pre allocated
    // to guarentee good performances. So let's do this in the test.
    pv.reserve(10);
    pv.resize(4);
    pv[0] = (Point) {INPUT_STATE_MOTION, 100,  50};
    pv[1] = (Point) {INPUT_STATE_MOTION, 800, 480};
    pv[2] = (Point) {INPUT_STATE_MOTION, 300, 200};
    pv[3] = (Point) {INPUT_STATE_MOTION, 0,  10};

    // Pressed somewhere in Content to set the pointer focus
    p = (Point) {INPUT_STATE_PRESSED, 700, 400};
    m_pInputManager->reportPointerEvent(p);

    // set popup not visible
    m_pScene->getSurface(CPLX_SCREEN_LAY2_SURF_POPUP_ID)->setVisibility(false);

    // Content surface is being rendered at (100,50) to (800,480), no scalling, translation or croping. So
    // (100,50)  screen wide is (0,0)     surface wide
    // (800,480) screen wide is (700,430) surface wide
    // (300,200) screen wide is (200,150) surface wide
    // (0,10)  screen wide is   (-100,-40)   surface wide
    surf = m_pInputManager->reportTouchEvent(pv);

    EXPECT_EQ(CPLX_SCREEN_LAY2_SURF_CONTENT_ID, surf->getID());
    EXPECT_EQ(0   , pv[0].x);
    EXPECT_EQ(0   , pv[0].y);
    EXPECT_EQ(700 , pv[1].x);
    EXPECT_EQ(430 , pv[1].y);
    EXPECT_EQ(200 , pv[2].x);
    EXPECT_EQ(150 , pv[2].y);
    EXPECT_EQ(-100, pv[3].x);
    EXPECT_EQ(-40 , pv[3].y);
}

