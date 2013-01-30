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

#ifndef DBUSDBUSCommunicatorTest_H_
#define DBUSDBUSCommunicatorTest_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ICommunicator.h"
#include "GenericCommunicator.h"
#include "mock_CommandExecutor.h"
#include "mock_Layerlist.h"

#include "ICommandExecutor.h"
#include "CommitCommand.h"
#include "LayerCreateCommand.h"
#include "SurfaceCreateCommand.h"
#include "SurfaceGetDimensionCommand.h"
#include "LayerGetDimensionCommand.h"
#include "SurfaceGetOpacityCommand.h"
#include "LayerGetOpacityCommand.h"
#include "SurfaceGetPixelformatCommand.h"
#include "LayerGetVisibilityCommand.h"
#include "SurfaceGetVisibilityCommand.h"
#include "LayerAddSurfaceCommand.h"
#include "LayerRemoveSurfaceCommand.h"
#include "LayerRemoveCommand.h"
#include "SurfaceRemoveCommand.h"
#include "SurfaceGetOrientationCommand.h"
#include "LayerGetOrientationCommand.h"
#include "LayerSetDestinationRectangleCommand.h"
#include "SurfaceSetDestinationRectangleCommand.h"
#include "LayerSetOpacityCommand.h"
#include "SurfaceSetOpacityCommand.h"
#include "LayerSetSourceRectangleCommand.h"
#include "SurfaceSetSourceRectangleCommand.h"
#include "LayerSetOrientationCommand.h"
#include "SurfaceSetOrientationCommand.h"
#include "LayerSetVisibilityCommand.h"
#include "SurfaceSetVisibilityCommand.h"
#include "DebugCommand.h"
#include "ExitCommand.h"
#include "ScreenSetRenderOrderCommand.h"
#include "LayerSetRenderOrderCommand.h"
#include "LayerSetDimensionCommand.h"
#include "SurfaceSetDimensionCommand.h"
#include "LayerSetPositionCommand.h"
#include "SurfaceSetPositionCommand.h"
#include "LayerGetPositionCommand.h"
#include "SurfaceGetPositionCommand.h"
#include "ShaderCreateCommand.h"
#include "ShaderDestroyCommand.h"
#include "SurfaceSetShaderCommand.h"
#include "ShaderSetUniformsCommand.h"
#include "ScreenDumpCommand.h"
#include "LayerDumpCommand.h"
#include "SurfaceDumpCommand.h"
#include "SurfaceSetNativeContentCommand.h"
#include "SurfaceRemoveNativeContentCommand.h"
#include "SurfaceSetKeyboardFocusCommand.h"
#include "SurfaceGetKeyboardFocusCommand.h"
#include "SurfaceUpdateInputEventAcceptance.h"

#include <vector>
#include <pthread.h>

using ::testing::Field;
using ::testing::Property;
using ::testing::Matcher;
using ::testing::Pointee;
using ::testing::AllOf;
using ::testing::SafeMatcherCast;
using ::testing::MatcherCast;
using ::testing::DefaultValue;
using ::testing::Eq;
using ::testing::An;
using ::testing::ElementsAreArray;
using ::testing::ElementsAre;
using ::testing::NotNull;

std::string DBUSCOMMAND_SYSTEM =
        "dbus-send --system --type=method_call --print-reply --dest=org.genivi.layermanagementservice /org/genivi/layermanagementservice org.genivi.layermanagementservice.";
std::string DBUSCOMMAND_SESSION =
        "dbus-send --type=method_call --print-reply --dest=org.genivi.layermanagementservice /org/genivi/layermanagementservice org.genivi.layermanagementservice.";
std::string DBUSCOMMAND = DBUSCOMMAND_SYSTEM;

class DBUSCommunicatorTest: public ::testing::Test {
public:
    pthread_t processThread;
    bool running;
    ICommunicator* communicatorUnderTest;
    MockCommandExecutor mockCommandExecutor;
    MockLayerList layerlist;
    ApplicationReferenceMap refmap;
    
    DBUSCommunicatorTest()
    : processThread(0)
    , running(false)
    , communicatorUnderTest(NULL)
    {
    }

    virtual ~DBUSCommunicatorTest() {
    }

    static void *processLoop(void * ptr) {
        while (((DBUSCommunicatorTest*) ptr)->running) {
            ((DBUSCommunicatorTest*) ptr)->communicatorUnderTest->process(100);
        }
        return NULL;
    }

    void SetUp() {
        char* useSessionBus = getenv("LM_USE_SESSION_BUS");
        if (NULL != useSessionBus && strcmp(useSessionBus, "enable") == 0) {
            DBUSCOMMAND = DBUSCOMMAND_SESSION;
        } else {
            DBUSCOMMAND = DBUSCOMMAND_SYSTEM;
        }
        Configuration config(0, NULL);
        communicatorUnderTest = new GenericCommunicator(mockCommandExecutor, config);
        this->communicatorUnderTest->start();
       
        DefaultValue<ApplicationReferenceMap*>::Set((ApplicationReferenceMap*) &this->refmap);
        running = true;
        pthread_create(&processThread, NULL, processLoop, (void*) this);
    }

    void TearDown() {
        running = false;
        if (communicatorUnderTest) {
            this->communicatorUnderTest->stop();
            pthread_join(processThread, NULL);
            delete communicatorUnderTest;
        }
    }
};

MATCHER_P(DebugCommandEq, onoff, "DebugCommand has onoff set to %(onoff)s") {
    return ((DebugCommand*)arg)->m_onoff == onoff;
}

TEST_F(DBUSCommunicatorTest, TurnDebugOnAndOff) {
    EXPECT_CALL(this->mockCommandExecutor, execute(DebugCommandEq(false) ) ).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("Debug boolean:false")).c_str()));
    EXPECT_CALL(this->mockCommandExecutor, execute(DebugCommandEq(true) ) ).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("Debug boolean:true")).c_str()));
    EXPECT_CALL(this->mockCommandExecutor, execute(DebugCommandEq(false) ) ).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("Debug boolean:false")).c_str()));
}

TEST_F(DBUSCommunicatorTest, ListAllLayerIDS) {
    DefaultValue<Scene*>::Set((Scene*) &this->layerlist);
    EXPECT_CALL(this->layerlist, getLayerIDs(NotNull(),NotNull() )).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("ListAllLayerIDS")).c_str()));
}

TEST_F(DBUSCommunicatorTest, listSurfaceoflayer) {

    Scene scene;
    DefaultValue<Layer*>::Set(scene.createLayer(234, 0));
    DefaultValue<Scene*>::Set((Scene*) &layerlist);
    EXPECT_CALL(this->layerlist, getLayer(Eq(234u) )).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("ListSurfaceofLayer uint32:234")).c_str()));
}

TEST_F(DBUSCommunicatorTest, getPropertiesOfSurface) {

    Scene scene;
    unsigned int newID = 0;
    DefaultValue<Scene*>::Set((Scene*) &layerlist);
    DefaultValue<Surface*>::Set(scene.createSurface(newID, 0));
    EXPECT_CALL(this->layerlist, getSurface(Eq(876u) )).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetPropertiesOfSurface uint32:876")).c_str()));
}

TEST_F(DBUSCommunicatorTest, getPropertiesOflayer) {

    Scene scene;
    DefaultValue<Layer*>::Set(scene.createLayer(0, 0));
    DefaultValue<Scene*>::Set((Scene*) &layerlist);
    EXPECT_CALL(this->layerlist, getLayer(Eq(876u) )).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetPropertiesOfLayer uint32:876")).c_str()));
}
/*
MATCHER_P4(SurfaceCreateCommandEq, nativeHandle, OriginalWidth, OriginalHeight, pixelformat, "%(*)s"){
    return ((SurfaceCreateCommand*)arg)->m_nativeHandle == nativeHandle
        && ((SurfaceCreateCommand*)arg)->m_originalHeight == OriginalHeight
        && ((SurfaceCreateCommand*)arg)->m_originalWidth == OriginalWidth
        && ((SurfaceCreateCommand*)arg)->m_pixelformat == pixelformat;
}
*/

/*
TEST_F(DBUSCommunicatorTest, CreateSurface) {

    EXPECT_CALL(this->mockCommandExecutor, execute( SurfaceCreateCommandEq(44u,33u,22u,PIXELFORMAT_RGBA8888))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("CreateSurface uint32:44 uint32:33 uint32:22 uint32:2 ")).c_str());

    EXPECT_CALL(this->mockCommandExecutor, execute( SurfaceCreateCommandEq(404u,303u,0u,PIXELFORMAT_RGB888))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("CreateSurface uint32:404 uint32:303 uint32:0 uint32:1 ")).c_str());
}
*/

MATCHER_P2(LayerCreateCommandEq, OriginalWidth, OriginalHeight, "%(*)s"){
    return ((LayerCreateCommand*)arg)->m_originalHeight == OriginalHeight
        && ((LayerCreateCommand*)arg)->m_originalWidth == OriginalWidth;
}

TEST_F(DBUSCommunicatorTest, CreateLayer) {

    uint resolution[2] = { 0, 0 };
    DefaultValue<uint*>::Set(resolution);

    EXPECT_CALL(this->mockCommandExecutor, execute( LayerCreateCommandEq(0u,0u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("CreateLayer")).c_str()));
}

MATCHER_P(SurfaceRemoveCommandEq, idToRemove, "%(*)s"){
    return ((SurfaceRemoveCommand*)arg)->m_idToRemove == idToRemove;
}

TEST_F(DBUSCommunicatorTest, RemoveSurface) {

    EXPECT_CALL(this->mockCommandExecutor, execute( SurfaceRemoveCommandEq(8u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("RemoveSurface uint32:8")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute( SurfaceRemoveCommandEq(5u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("RemoveSurface uint32:5")).c_str()));
}

MATCHER_P(LayerRemoveCommandEq, idToRemove, "%(*)s"){
    return ((LayerRemoveCommand*)arg)->m_idToRemove == idToRemove;
}

TEST_F(DBUSCommunicatorTest, Removelayer) {
    EXPECT_CALL(this->mockCommandExecutor, execute( LayerRemoveCommandEq(8u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("RemoveLayer uint32:8")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute( LayerRemoveCommandEq(5u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("RemoveLayer uint32:5")).c_str()));
}

MATCHER_P2(SurfaceSetOpacityCommandEq, id, Opacity, "%(*)s"){
    return ((SurfaceSetOpacityCommand*)arg)->m_id == id
        && ((SurfaceSetOpacityCommand*)arg)->m_opacity == Opacity;
}

TEST_F(DBUSCommunicatorTest, SetSurfaceOpacity) {
    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetOpacityCommandEq(36u,0.88) )).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceOpacity uint32:36 double:0.88")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetOpacityCommandEq(44u,0.001) )).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceOpacity uint32:44 double:0.001")).c_str()));
}

/*
TEST_F(DBUSCommunicatorTest, Exit) {
    EXPECT_CALL(this->mockCommandExecutor, execute(Field(&Command::commandType,Eq(Command::Exit)) ) ).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("Exit")).c_str());

    //EXPECT_EXIT(ASSERT_NE(-1, system((DBUSCOMMAND + std::string("Exit")).c_str());, ::testing::ExitedWithCode(0), "");
}
*/

/*
TEST_F(DBUSCommunicatorTest, Commit) {
    EXPECT_CALL(this->mockCommandExecutor, execute(Field(&ICommand::commandType, Eq(ICommand::CommitChanges)) ) ).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("CommitChanges")).c_str());
}
*/

MATCHER_P5(SurfaceSetSourceRectangleCommandEq, id, x, y, width, height, "%(*)s"){
    return ((SurfaceSetSourceRectangleCommand*)arg)->m_id == id
        && ((SurfaceSetSourceRectangleCommand*)arg)->m_x == x
        && ((SurfaceSetSourceRectangleCommand*)arg)->m_y == y
        && ((SurfaceSetSourceRectangleCommand*)arg)->m_width == width
        && ((SurfaceSetSourceRectangleCommand*)arg)->m_height == height;
}

TEST_F(DBUSCommunicatorTest, SetSurfaceSourceRegion) {

    EXPECT_CALL(this->mockCommandExecutor, execute( SurfaceSetSourceRectangleCommandEq(36u,1u,2u,3u,4u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceSourceRegion uint32:36 uint32:1 uint32:2 uint32:3 uint32:4")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute( SurfaceSetSourceRectangleCommandEq(44u,15u,25u,35u,45u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceSourceRegion uint32:44 uint32:15 uint32:25 uint32:35 uint32:45 ")).c_str()));
}

MATCHER_P5(LayerSetSourceRectangleCommandEq, id, x, y, width, height, "%(*)s"){
    return ((LayerSetSourceRectangleCommand*)arg)->m_id == id
        && ((LayerSetSourceRectangleCommand*)arg)->m_x == x
        && ((LayerSetSourceRectangleCommand*)arg)->m_y == y
        && ((LayerSetSourceRectangleCommand*)arg)->m_width == width
        && ((LayerSetSourceRectangleCommand*)arg)->m_height == height;
}

TEST_F(DBUSCommunicatorTest, SetlayerSourceRegion) {

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetSourceRectangleCommandEq(36u,1u,2u,3u,4u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerSourceRegion uint32:36 uint32:1 uint32:2 uint32:3 uint32:4")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetSourceRectangleCommandEq(44u,15u,25u,35u,45u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerSourceRegion uint32:44 uint32:15 uint32:25 uint32:35 uint32:45 ")).c_str()));
}

MATCHER_P5(LayerSetDestinationRectangleCommandEq, id, x, y, width, height, "%(*)s") {
    return ((LayerSetDestinationRectangleCommand*)arg)->m_id == id
        && ((LayerSetDestinationRectangleCommand*)arg)->m_x == x
        && ((LayerSetDestinationRectangleCommand*)arg)->m_y == y
        && ((LayerSetDestinationRectangleCommand*)arg)->m_width == width
        && ((LayerSetDestinationRectangleCommand*)arg)->m_height == height;
}

TEST_F(DBUSCommunicatorTest, SetlayerDestinationRegion) {

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetDestinationRectangleCommandEq(36u,1u,2u,3u,4u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerDestinationRegion uint32:36 uint32:1 uint32:2 uint32:3 uint32:4")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetDestinationRectangleCommandEq(44u,15u,25u,35u,45u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerDestinationRegion uint32:44 uint32:15 uint32:25 uint32:35 uint32:45 ")).c_str()));
}

MATCHER_P5(SurfaceSetDestinationRectangleCommandEq, id, x, y, width, height, "%(*)s") {
    return ((SurfaceSetDestinationRectangleCommand*)arg)->m_id == id
        && ((SurfaceSetDestinationRectangleCommand*)arg)->m_x == x
        && ((SurfaceSetDestinationRectangleCommand*)arg)->m_y == y
        && ((SurfaceSetDestinationRectangleCommand*)arg)->m_width == width
        && ((SurfaceSetDestinationRectangleCommand*)arg)->m_height == height;
}

TEST_F(DBUSCommunicatorTest, SetSurfaceDestinationRegion) {

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetDestinationRectangleCommandEq(36u,1u,2u,3u,4u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceDestinationRegion uint32:36 uint32:1 uint32:2 uint32:3 uint32:4")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetDestinationRectangleCommandEq(44u,15u,25u,35u,45u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceDestinationRegion uint32:44 uint32:15 uint32:25 uint32:35 uint32:45 ")).c_str()));
}

MATCHER_P3(LayerSetPositionCommandEq, id, x, y, "%(*)s"){
    return ((LayerSetPositionCommand*)arg)->m_id == id
        && ((LayerSetPositionCommand*)arg)->m_x == x
        && ((LayerSetPositionCommand*)arg)->m_y == y;
}

TEST_F(DBUSCommunicatorTest, SetlayerPosition) {
    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetPositionCommandEq(36u,1u,2u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerPosition uint32:36 uint32:1 uint32:2")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetPositionCommandEq(44u,15u,25u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerPosition uint32:44 uint32:15 uint32:25 ")).c_str()));
}

MATCHER_P3(SurfaceSetPositionCommandEq, id, x, y, "%(*)s"){
    return ((SurfaceSetPositionCommand*)arg)->m_id == id
        && ((SurfaceSetPositionCommand*)arg)->m_x == x
        && ((SurfaceSetPositionCommand*)arg)->m_y == y;
}

TEST_F(DBUSCommunicatorTest, SetSurfacePosition) {
    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetPositionCommandEq(36u,3u,4u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfacePosition uint32:36 uint32:3 uint32:4")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetPositionCommandEq(44u,35u,45u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfacePosition uint32:44 uint32:35 uint32:45 ")).c_str()));
}

MATCHER_P3(LayerSetDimensionCommandEq, id, width, height, "%(*)s"){
    return ((LayerSetDimensionCommand*)arg)->m_id == id
        && ((LayerSetDimensionCommand*)arg)->m_width == width
        && ((LayerSetDimensionCommand*)arg)->m_height == height;
}

TEST_F(DBUSCommunicatorTest, SetlayerDimension) {
    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetDimensionCommandEq(8554u,400u,444u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerDimension uint32:8554 uint32:400 uint32:444")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetDimensionCommandEq(34589u,400u,444u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerDimension uint32:34589 uint32:400 uint32:444")).c_str()));
}

MATCHER_P3(SurfaceSetDimensionCommandEq, id, width, height, "%(*)s"){
    return ((SurfaceSetDimensionCommand*)arg)->m_id == id
        && ((SurfaceSetDimensionCommand*)arg)->m_width == width
        && ((SurfaceSetDimensionCommand*)arg)->m_height == height;
}

TEST_F(DBUSCommunicatorTest, SetSurfaceDimension) {
    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetDimensionCommandEq(36u,3u,4u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceDimension uint32:36 uint32:3 uint32:4")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetDimensionCommandEq(44u,35u,45u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceDimension uint32:44 uint32:35 uint32:45 ")).c_str()));
}

MATCHER_P2(LayerSetOpacityCommandEq, id, Opacity, "%(*)s"){
    return ((LayerSetOpacityCommand*)arg)->m_id == id
        && ((LayerSetOpacityCommand*)arg)->m_opacity == Opacity;
}

TEST_F(DBUSCommunicatorTest, SetlayerOpacity) {

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetOpacityCommandEq(36u,0.88))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerOpacity uint32:36 double:0.88")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetOpacityCommandEq(44u,0.001))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerOpacity uint32:44 double:0.001")).c_str()));
}

MATCHER_P(LayerGetOpacityCommandEq, id, "%(*)s"){
    return ((LayerGetOpacityCommand*)arg)->m_id == id;
}

TEST_F(DBUSCommunicatorTest, GetlayerOpacity) {

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerGetOpacityCommandEq(36u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetLayerOpacity uint32:36")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerGetOpacityCommandEq(44u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetLayerOpacity uint32:44 ")).c_str()));
}

MATCHER_P2(SurfaceSetOrientationCommandEq, id, Orientation, "%(*)s") {
    return ((SurfaceSetOrientationCommand*)arg)->m_id == id
        && ((SurfaceSetOrientationCommand*)arg)->m_orientation == Orientation;
}

TEST_F(DBUSCommunicatorTest, SetSurfaceOrientation) {

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetOrientationCommandEq(36u,0))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceOrientation uint32:36 uint32:0")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetOrientationCommandEq(44u,1))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceOrientation uint32:44 uint32:1")).c_str()));
}

MATCHER_P2(LayerSetOrientationCommandEq, id, Orientation, "%(*)s") {
    return ((LayerSetOrientationCommand*)arg)->m_id == id
        && ((LayerSetOrientationCommand*)arg)->m_orientation == Orientation;
}

TEST_F(DBUSCommunicatorTest, SetlayerOrientation) {

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetOrientationCommandEq(36u,0))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerOrientation uint32:36 uint32:0")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetOrientationCommandEq(44u,1))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerOrientation uint32:44 uint32:1")).c_str()));
}

MATCHER_P2(SurfaceSetVisibilityCommandEq, id, visibility, "%(*)s"){
    return ((SurfaceSetVisibilityCommand*)arg)->m_idtoSet == id
        && ((SurfaceSetVisibilityCommand*)arg)->m_visibility == visibility;
}

TEST_F(DBUSCommunicatorTest, SetSurfaceVisibility) {

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetVisibilityCommandEq(36u,false))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceVisibility uint32:36 boolean:false")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetVisibilityCommandEq(44u,true))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceVisibility uint32:44 boolean:true")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetVisibilityCommandEq(36u,false))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceVisibility uint32:36 boolean:false")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetVisibilityCommandEq(44u,true))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetSurfaceVisibility uint32:44 boolean:true")).c_str()));
}

MATCHER_P2(LayerSetVisibilityCommandEq, id, visibility, "%(*)s"){
    return ((LayerSetVisibilityCommand*)arg)->m_idtoSet == id
        && ((LayerSetVisibilityCommand*)arg)->m_visibility == visibility;
}

TEST_F(DBUSCommunicatorTest, SetlayerVisibility) {

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetVisibilityCommandEq(36u,false))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerVisibility uint32:36 boolean:false")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetVisibilityCommandEq(44u,true))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerVisibility uint32:44 boolean:true")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetVisibilityCommandEq(36u,false))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerVisibility uint32:36 boolean:false")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerSetVisibilityCommandEq(44u,true))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetLayerVisibility uint32:44 boolean:true")).c_str()));
}

MATCHER_P2(LayerAddSurfaceCommandEq, surfaceid, layerid, "%(*)s"){
    return ((LayerAddSurfaceCommand*)arg)->m_layerid == layerid
        && ((LayerAddSurfaceCommand*)arg)->m_surfaceid == surfaceid;
}

TEST_F(DBUSCommunicatorTest, AddSurfaceTolayer) {

    EXPECT_CALL(this->mockCommandExecutor, execute( LayerAddSurfaceCommandEq(36u,77u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("AddSurfaceToLayer uint32:36 uint32:77")).c_str()));
}

MATCHER_P2(LayerRemoveSurfaceCommandEq, surfaceid, layerid, "%(*)s"){
    return ((LayerRemoveSurfaceCommand*)arg)->m_layerid == layerid
        && ((LayerRemoveSurfaceCommand*)arg)->m_surfaceid == surfaceid;
}

TEST_F(DBUSCommunicatorTest, RemoveSurfaceFromlayer) {

    EXPECT_CALL(this->mockCommandExecutor, execute( LayerRemoveSurfaceCommandEq(36u,77u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("RemoveSurfaceFromLayer uint32:36 uint32:77")).c_str()));
}

MATCHER_P(LayerGetDimensionCommandEq, id, "%(*)s") {
    return ((LayerGetDimensionCommand*)arg)->m_id == id;
}

TEST_F(DBUSCommunicatorTest, GetlayerDimension) {
    EXPECT_CALL(this->mockCommandExecutor, execute(LayerGetDimensionCommandEq(8554u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetLayerDimension uint32:8554")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerGetDimensionCommandEq(34589u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetLayerDimension uint32:34589")).c_str()));
}

MATCHER_P(SurfaceGetDimensionCommandEq, id, "%(*)s") {
    return ((SurfaceGetDimensionCommand*)arg)->m_id == id;
}

TEST_F(DBUSCommunicatorTest, GetSurfaceDimension) {

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceGetDimensionCommandEq(8554u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetSurfaceDimension uint32:8554")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceGetDimensionCommandEq(34589u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetSurfaceDimension uint32:34589")).c_str()));
}

MATCHER_P(SurfaceGetPixelformatCommandEq, id, "%(*)s"){
    return ((SurfaceGetPixelformatCommand*)arg)->m_id == id;
}

TEST_F(DBUSCommunicatorTest, GetSurfacePixelformat) {
    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceGetPixelformatCommandEq(8554u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetSurfacePixelformat uint32:8554")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceGetPixelformatCommandEq(34589u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetSurfacePixelformat uint32:34589")).c_str()));
}

MATCHER_P(SurfaceGetOpacityCommandEq, id, "%(*)s"){
    return ((SurfaceGetOpacityCommand*)arg)->m_id == id;
}

TEST_F(DBUSCommunicatorTest, GetSurfaceOpacity) {
    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceGetOpacityCommandEq(8554u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetSurfaceOpacity uint32:8554")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceGetOpacityCommandEq(34589u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetSurfaceOpacity uint32:34589")).c_str()));
}

MATCHER_P(SurfaceGetVisibilityCommandEq, id, "%(*)s") {
    return ((SurfaceGetVisibilityCommand*)arg)->m_id == id;
}

TEST_F(DBUSCommunicatorTest, GetSurfaceVisibility) {
    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceGetVisibilityCommandEq(8554u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetSurfaceVisibility uint32:8554")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceGetVisibilityCommandEq(34589u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetSurfaceVisibility uint32:34589")).c_str()));
}

MATCHER_P(LayerGetVisibilityCommandEq, id, "%(*)s"){
    return ((LayerGetVisibilityCommand*)arg)->m_id == id;
}

TEST_F(DBUSCommunicatorTest, GetlayerVisibility) {
    EXPECT_CALL(this->mockCommandExecutor, execute(LayerGetVisibilityCommandEq(8554u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetLayerVisibility uint32:8554")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(LayerGetVisibilityCommandEq(34589u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetLayerVisibility uint32:34589")).c_str()));
}

/*
MATCHER_P2(RenderOrderTester,commandtype,list, "") {
    LOG_ERROR("in tester",0);
    commandtype_type command = (commandtype_type)arg;
    testing::internal::ElementsAreArrayMatcher<int> matcher = ElementsAreArray(command->array,3);
    std::vector<uint> l;
    testing::Matcher<std::vector<uint, std::allocator<uint> > > matcher2 = matcher.operator Matcher<std::vector<uint> >();
    bool result = matcher2.Matches(list);
    return result;
}

TEST_F(DBUSCommunicatorTest, SetRenderOrderOflayers) {

//  DBUSClient* client = dbus_helper();
//  if (NULL== client)
//      LOG_ERROR("TEST", "client is null");
//
//  std::vector<uint> expected;
//  expected.push_back(4);
//  expected.push_back(47);
//  expected.push_back(49);
//  SetLayerOrderCommand * p;
// EXPECT_CALL(this->mockCommandExecutor,execute(RenderOrderTester(p,expected))).Times(1);
//  uint display = 0;
//  client->SetRenderOrderOfLayers(expected,display);

}

TEST_F(DBUSCommunicatorTest, SetRenderOrderWithinlayer) {
//  DBUSClient* client = dbus_helper();
//
//  std::vector<uint> expected;
//  expected.push_back(4);
//  expected.push_back(47);
//  expected.push_back(49);
//  SetOrderWithinLayerCommand * p;
// EXPECT_CALL(this->mockCommandExecutor,execute(AllOf(
//                          RenderOrderTester(p,expected),
//                          FieldCast(&SetOrderWithinLayerCommand::layerid,55))
//                          )).Times(1);
//  client->SetSurfaceRenderOrderWithinLayer(55,expected);
}
*/

TEST_F(DBUSCommunicatorTest, GetlayerType) {
    DefaultValue<Scene*>::Set((Scene*) &layerlist);
    EXPECT_CALL(this->layerlist, getLayer(Eq(8554u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetLayerType uint32:8554")).c_str()));
}

TEST_F(DBUSCommunicatorTest, GetlayertypeCapabilities) {
    EXPECT_CALL(this->mockCommandExecutor, getLayerTypeCapabilities(Eq(367))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetLayertypeCapabilities uint32:367")).c_str()));
}

TEST_F(DBUSCommunicatorTest, GetlayerCapabilities) {
    Scene scene;
    DefaultValue<Scene*>::Set((Scene*) &layerlist);
    EXPECT_CALL(this->layerlist, getLayer(Eq(367u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetLayerCapabilities uint32:367")).c_str()));

}

MATCHER_P2(ShaderCreateCommandEq, _vertName,_fragName, "%(*)s") {
    return ((ShaderCreateCommand*)arg)->getVertName() == _vertName
        && ((ShaderCreateCommand*)arg)->getFragName() == _fragName;
}

TEST_F(DBUSCommunicatorTest, CreateShader) {
    EXPECT_CALL(this->mockCommandExecutor, execute(ShaderCreateCommandEq(std::string("test123.glslv"),std::string("differentshader.glslv")))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("CreateShader string:test123.glslv string:differentshader.glslv")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(ShaderCreateCommandEq(std::string("/test/path/shadertest.glslv"),std::string("foobar")))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("CreateShader string:/test/path/shadertest.glslv string:foobar")).c_str()));
}

MATCHER_P(ShaderDestroyCommandEq, id, "%(*)s") {
    return ((ShaderDestroyCommand*)arg)->getShaderID() == id;
}

TEST_F(DBUSCommunicatorTest, DestroyShaderCommand) {
    EXPECT_CALL(this->mockCommandExecutor, execute(ShaderDestroyCommandEq(567u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("DestroyShader uint32:567")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(ShaderDestroyCommandEq(185u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("DestroyShader uint32:185")).c_str()));
}

MATCHER_P2(SurfaceSetShaderCommandEq, id, shaderid, "%(*)s") {
    return ((SurfaceSetShaderCommand*)arg)->getID() == id
        && ((SurfaceSetShaderCommand*)arg)->getShaderID() == shaderid;
}

TEST_F(DBUSCommunicatorTest, SetShaderCommand) {

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetShaderCommandEq(987u, 567u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetShader uint32:987 uint32:567")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(SurfaceSetShaderCommandEq(1u, 998877u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetShader uint32:1 uint32:998877")).c_str()));
}

MATCHER_P(ShaderUniformsMatcher,expectedlist,"") {
    ShaderSetUniformsCommand* command = (ShaderSetUniformsCommand*)arg;
    std::vector<std::string> uniforms = command->getUniforms();

    EXPECT_THAT(uniforms,ElementsAreArray(*expectedlist));
    //EXPECT_THAT(uniforms,ElementsAreArray(expectedlist));
    return true;
}

MATCHER_P(ShaderSetUniformsCommandEq, _shaderid, "%(*)s"){
    return ((ShaderSetUniformsCommand*)arg)->getShaderId() == _shaderid;
}

TEST_F(DBUSCommunicatorTest, DISABLED_ShaderSetUniformsCommand) {

    std::string expected1[] = { "uRotAngle 1f 1 0", "uRotVector 3f 1 1 0 0" };
    EXPECT_CALL(this->mockCommandExecutor, execute( AllOf(ShaderSetUniformsCommandEq(1u), ShaderUniformsMatcher(&expected1)))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetUniforms uint32:1 array:string:\"uRotAngle 1f 1 0\",\"uRotVector 3f 1 1 0 0\"")).c_str()));

    std::string expected2[] = { "teststring foobar" };
    EXPECT_CALL(this->mockCommandExecutor, execute(AllOf(ShaderSetUniformsCommandEq(17346u), ShaderUniformsMatcher(&expected2)))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetUniforms uint32:17346 array:string:\"teststring foobar\"")).c_str()));
}



MATCHER_P(SetKeyboardFocusOnCommandEq, surfId, "%(*)s"){
    return ((SurfaceSetKeyboardFocusCommand*)arg)->m_surfId == surfId;
}

TEST_F(DBUSCommunicatorTest, SetKeyboardFocusOn)
{
    EXPECT_CALL(this->mockCommandExecutor, execute(SetKeyboardFocusOnCommandEq(42u))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("SetKeyboardFocusOn uint32:42")).c_str()));
}



MATCHER(GetKeyboardFocusSurfaceIdCommandEq, "%(*)s"){
    return ((SurfaceGetKeyboardFocusCommand*)arg)->m_pSurfId != NULL;
}

TEST_F(DBUSCommunicatorTest, GetKeyboardFocusSurfaceId)
{

    EXPECT_CALL(this->mockCommandExecutor, execute(GetKeyboardFocusSurfaceIdCommandEq())).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("GetKeyboardFocusSurfaceId")).c_str()));
}


MATCHER_P3(UpdateInputEventAcceptanceOnCommandEq, surfId, devices, acceptance, "%(*)s"){
    return (
                (((SurfaceUpdateInputEventAcceptance*)arg)->m_surfId == surfId)
             && (((SurfaceUpdateInputEventAcceptance*)arg)->m_devices == devices)
             && (((SurfaceUpdateInputEventAcceptance*)arg)->m_accept == acceptance)
           );
}

TEST_F(DBUSCommunicatorTest, UpdateInputEventAcceptanceOn)
{
    EXPECT_CALL(this->mockCommandExecutor, execute(UpdateInputEventAcceptanceOnCommandEq(42u, (InputDevice)1, true))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("UpdateInputEventAcceptanceOn uint32:42 uint32:1 boolean:true")).c_str()));

    EXPECT_CALL(this->mockCommandExecutor, execute(UpdateInputEventAcceptanceOnCommandEq(1000u, (InputDevice)10, false))).Times(1);
    ASSERT_NE(-1, system((DBUSCOMMAND + std::string("UpdateInputEventAcceptanceOn uint32:1000 uint32:10 boolean:false")).c_str()));
}



#endif /* DBUSCommunicatorTest_H_ */
