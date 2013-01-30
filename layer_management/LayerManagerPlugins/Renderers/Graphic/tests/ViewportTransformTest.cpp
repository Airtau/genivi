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
#include "Surface.h"
#include "Layer.h"
#include "ViewportTransform.h"
#include "Scene.h"

class ViewportTransformTest : public ::testing::Test {
public:

    ViewportTransformTest()
    {
    }

    virtual ~ViewportTransformTest()
    {
    }
    void SetUp() {
        m_scene = new Scene();
    }
    void TearDown() {
        delete m_scene;
    }
    Scene* m_scene;
};

// LAYER SOURCE TRANSFORMATION

TEST_F(ViewportTransformTest, doLayerSRCSurfaceCompletelyWithin){
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,10,10);
    FloatRectangle targetSurfaceDest = FloatRectangle(20,20,10,10);
    FloatRectangle layerSRC = FloatRectangle(10,10,30,30);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    ASSERT_EQ(10.0, targetSurfaceDest.x);
    ASSERT_EQ(10.0, targetSurfaceDest.width);
    ASSERT_EQ(10.0, targetSurfaceDest.y);
    ASSERT_EQ(10.0, targetSurfaceDest.height);

    ASSERT_EQ(0.0, targetSurfaceSrc.x);
    ASSERT_EQ(10.0, targetSurfaceSrc.width);
    ASSERT_EQ(0.0, targetSurfaceSrc.y);
    ASSERT_EQ(10.0, targetSurfaceSrc.height);

}

TEST_F(ViewportTransformTest, doLayerSRCSurfaceCroppedFromLeft){
    uint surfaceOriginalWidth = 20;
    uint surfaceOriginalHeight = 20;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,20,20);
    FloatRectangle targetSurfaceDest = FloatRectangle(20,20,20,20);
    FloatRectangle layerSRC = FloatRectangle(30,30,20,20);
    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_EQ(0.5,textureCoordinates[0]);
    ASSERT_EQ(1.0,textureCoordinates[2]);
    ASSERT_EQ(0.5,textureCoordinates[1]);
    ASSERT_EQ(1.0,textureCoordinates[3]);
    ASSERT_EQ(0.0, targetSurfaceDest.x);
    ASSERT_EQ(10.0, targetSurfaceDest.width);
    ASSERT_EQ(0.0, targetSurfaceDest.y);
    ASSERT_EQ(10.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerSRCSurfaceCroppedFromRight){
    uint surfaceOriginalWidth = 20;
    uint surfaceOriginalHeight = 20;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,20,20);
    FloatRectangle targetSurfaceDest = FloatRectangle(20,20,20,20);
    FloatRectangle layerSRC = FloatRectangle(0,0,35,35);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_EQ(0,textureCoordinates[0]);
    ASSERT_EQ(0.75,textureCoordinates[2]);
    ASSERT_EQ(0,textureCoordinates[1]);
    ASSERT_EQ(0.75,textureCoordinates[3]);
    ASSERT_EQ(20.0, targetSurfaceDest.x);
    ASSERT_EQ(15.0, targetSurfaceDest.width);
    ASSERT_EQ(20.0, targetSurfaceDest.y);
    ASSERT_EQ(15.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerSRCSurfaceCroppedFromBothSides){
    uint surfaceOriginalWidth = 20;
    uint surfaceOriginalHeight = 20;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,20,20);
    FloatRectangle targetSurfaceDest = FloatRectangle(20,20,20,20);
    FloatRectangle layerSRC = FloatRectangle(25,25,10,10);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_EQ(0.25,textureCoordinates[0]);
    ASSERT_EQ(0.75,textureCoordinates[2]);
    ASSERT_EQ(0.25,textureCoordinates[1]);
    ASSERT_EQ(0.75,textureCoordinates[3]);
    ASSERT_EQ(0.0, targetSurfaceDest.x);
    ASSERT_EQ(10.0, targetSurfaceDest.width);
    ASSERT_EQ(0.0, targetSurfaceDest.y);
    ASSERT_EQ(10.0, targetSurfaceDest.height);
}

// LAYER DESTINATION TRANSFORMATION

TEST_F(ViewportTransformTest, doLayerDESTScaleUp){
    uint surfaceOriginalWidth = 20;
    uint surfaceOriginalHeight = 20;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,20,20);
    FloatRectangle targetSurfaceDest = FloatRectangle(10,10,100,100);
    FloatRectangle layerSrc = FloatRectangle(0,0,200,200);
    FloatRectangle layerDest = FloatRectangle(50,50,600,600);

    ViewportTransform::applyLayerDestination(layerDest,layerSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_EQ(0.0,textureCoordinates[0]);
    ASSERT_EQ(1.0,textureCoordinates[2]);
    ASSERT_EQ(0.0,textureCoordinates[1]);
    ASSERT_EQ(1.0,textureCoordinates[3]);
    ASSERT_EQ(80.0, targetSurfaceDest.x);
    ASSERT_EQ(300.0, targetSurfaceDest.width);
    ASSERT_EQ(80.0, targetSurfaceDest.y);
    ASSERT_EQ(300.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerDESTScaleDown){
    uint surfaceOriginalWidth = 20;
    uint surfaceOriginalHeight = 20;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,20,20);
    FloatRectangle targetSurfaceDest = FloatRectangle(10,10,100,100);
    FloatRectangle layerSRC = FloatRectangle(0,0,200,200);
    FloatRectangle layerDest = FloatRectangle(50,50,20,20);

    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_EQ(0.0,textureCoordinates[0]);
    ASSERT_EQ(1.0,textureCoordinates[2]);
    ASSERT_EQ(0.0,textureCoordinates[1]);
    ASSERT_EQ(1.0,textureCoordinates[3]);
    ASSERT_EQ(51.0, targetSurfaceDest.x);
    ASSERT_EQ(10.0, targetSurfaceDest.width);
    ASSERT_EQ(51.0, targetSurfaceDest.y);
    ASSERT_EQ(10.0, targetSurfaceDest.height);
}


TEST_F(ViewportTransformTest, doLayerSRCTransformationTest1){
    uint surfaceOriginalWidth = 10;
    uint surfaceOriginalHeight = 10;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,10,10);
    FloatRectangle targetSurfaceDest = FloatRectangle(20,20,10,10);
    FloatRectangle layerSRC = FloatRectangle(25,25,75,75);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.5,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.5,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
    ASSERT_EQ(0.0, targetSurfaceDest.x);
    ASSERT_EQ(5.0, targetSurfaceDest.width);
    ASSERT_EQ(0.0, targetSurfaceDest.y);
    ASSERT_EQ(5.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerSRCTransformationTest2){
    uint surfaceOriginalWidth = 20;
    uint surfaceOriginalHeight = 20;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,20,20);
    FloatRectangle targetSurfaceDest = FloatRectangle(30,30,30,30);
    FloatRectangle layerSRC = FloatRectangle(25,25,75,75);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.0,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
    ASSERT_EQ(5.0, targetSurfaceDest.x);
    ASSERT_EQ(30.0, targetSurfaceDest.width);
    ASSERT_EQ(5.0, targetSurfaceDest.y);
    ASSERT_EQ(30.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerSRCTransformationTest3){
    uint surfaceOriginalWidth = 10;
    uint surfaceOriginalHeight = 10;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,10,10);
    FloatRectangle targetSurfaceDest = FloatRectangle(70,70,10,10);
    FloatRectangle layerSRC = FloatRectangle(25,25,50,50);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.0,textureCoordinates[0],0.01);
    ASSERT_NEAR(0.5,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(0.5,textureCoordinates[3],0.01);
    ASSERT_EQ(45.0, targetSurfaceDest.x);
    ASSERT_EQ(5.0, targetSurfaceDest.width);
    ASSERT_EQ(45.0, targetSurfaceDest.y);
    ASSERT_EQ(5.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerSRCTransformationTest4){
    uint surfaceOriginalWidth = 100;
    uint surfaceOriginalHeight = 100;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,100,100);
    FloatRectangle targetSurfaceDest = FloatRectangle(0,0,100,100);
    FloatRectangle layerSRC = FloatRectangle(25,25,50,50);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.25,textureCoordinates[0],0.01);
    ASSERT_NEAR(0.75,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.25,textureCoordinates[1],0.01);
    ASSERT_NEAR(0.75,textureCoordinates[3],0.01);
    ASSERT_EQ(0.0, targetSurfaceDest.x);
    ASSERT_EQ(50.0, targetSurfaceDest.width);
    ASSERT_EQ(0.0, targetSurfaceDest.y);
    ASSERT_EQ(50.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerSRCTransformationTest5){
    uint surfaceOriginalWidth = 10;
    uint surfaceOriginalHeight = 10;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,10,10);
    FloatRectangle targetSurfaceDest = FloatRectangle(30,30,10,10);
    FloatRectangle layerSRC = FloatRectangle(20,20,20,20);
    FloatRectangle layerDest = FloatRectangle(0,0,40,40);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.0,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
    ASSERT_EQ(20.0, targetSurfaceDest.x);
    ASSERT_EQ(20.0, targetSurfaceDest.width);
    ASSERT_EQ(20.0, targetSurfaceDest.y);
    ASSERT_EQ(20.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerDESTTransformationTest1){
    uint surfaceOriginalWidth = 10;
    uint surfaceOriginalHeight = 10;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,10,10);
    FloatRectangle targetSurfaceDest = FloatRectangle(0,0,100,100);
    FloatRectangle layerSRC = FloatRectangle(0,0,100,100);
    FloatRectangle layerDest = FloatRectangle(0,0,100,100);

    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.0,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
    ASSERT_EQ(0.0, targetSurfaceDest.x);
    ASSERT_EQ(100.0, targetSurfaceDest.width);
    ASSERT_EQ(0.0, targetSurfaceDest.y);
    ASSERT_EQ(100.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerDESTTransformationTest2){
    uint surfaceOriginalWidth = 100;
    uint surfaceOriginalHeight = 100;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,100,100);
    FloatRectangle targetSurfaceDest = FloatRectangle(0,0,100,100);
    FloatRectangle layerSRC = FloatRectangle(0,0,100,100);
    FloatRectangle layerDest = FloatRectangle(0,0,200,200);

    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.0,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
    ASSERT_EQ(0.0, targetSurfaceDest.x);
    ASSERT_EQ(200.0, targetSurfaceDest.width);
    ASSERT_EQ(0.0, targetSurfaceDest.y);
    ASSERT_EQ(200.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerDESTTransformationTest3){
    uint surfaceOriginalWidth = 100;
    uint surfaceOriginalHeight = 100;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,100,100);
    FloatRectangle targetSurfaceDest = FloatRectangle(0,0,100,100);
    FloatRectangle layerSRC = FloatRectangle(0,0,100,100);
    FloatRectangle layerDest = FloatRectangle(50,50,200,200);

    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.0,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
    ASSERT_EQ(50.0, targetSurfaceDest.x);
    ASSERT_EQ(200.0, targetSurfaceDest.width);
    ASSERT_EQ(50.0, targetSurfaceDest.y);
    ASSERT_EQ(200.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, doLayerDESTTransformationTest4){
    uint surfaceOriginalWidth = 50;
    uint surfaceOriginalHeight = 50;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,50,50);
    FloatRectangle targetSurfaceDest = FloatRectangle(50,50,50,50);
    FloatRectangle layerSrc = FloatRectangle(50,50,100,100);
    FloatRectangle layerDest = FloatRectangle(50,50,200,200);

    ViewportTransform::applyLayerDestination(layerDest,layerSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.0,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
    ASSERT_EQ(150.0, targetSurfaceDest.x);
    ASSERT_EQ(100.0, targetSurfaceDest.width);
    ASSERT_EQ(150.0, targetSurfaceDest.y);
    ASSERT_EQ(100.0, targetSurfaceDest.height);
}

TEST_F(ViewportTransformTest, completeExample1){
    int surfaceOriginalWidth =60;
    int surfaceOriginalHeight = 60;
    FloatRectangle targetSurfaceSrc = FloatRectangle(20,20,20,20);
    FloatRectangle targetSurfaceDest = FloatRectangle(20,20,60,60);

    FloatRectangle layerSRC = FloatRectangle(50,50,50,50);
    FloatRectangle layerDest = FloatRectangle(0,0,200,200);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);

    ASSERT_EQ(30.0,targetSurfaceSrc.x);
    ASSERT_EQ(10.0,targetSurfaceSrc.width);
    ASSERT_EQ(30.0,targetSurfaceSrc.y);
    ASSERT_EQ(10.0,targetSurfaceSrc.height);
    ASSERT_NEAR(0.5,textureCoordinates[0],0.01);
    ASSERT_NEAR(0.66,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.5,textureCoordinates[1],0.01);
    ASSERT_NEAR(0.66,textureCoordinates[3],0.01);

    ASSERT_EQ(0.0,targetSurfaceDest.x);
    ASSERT_EQ(30.0,targetSurfaceDest.width);
    ASSERT_EQ(0.0,targetSurfaceDest.y);
    ASSERT_EQ(30.0,targetSurfaceDest.height);

    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_EQ(120.0,targetSurfaceDest.width);
    ASSERT_EQ(0.0,targetSurfaceDest.x);
    ASSERT_EQ(120.0,targetSurfaceDest.height);
    ASSERT_EQ(0.0,targetSurfaceDest.y);
    ASSERT_NEAR(0.5,textureCoordinates[0],0.01);
    ASSERT_NEAR(0.66,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.5,textureCoordinates[1],0.01);
    ASSERT_NEAR(0.66,textureCoordinates[3],0.01);

}

TEST_F(ViewportTransformTest, completeExample2){
    int surfaceOriginalWidth =100;
    int surfaceOriginalHeight = 100;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,100,100);
    FloatRectangle targetSurfaceDest = FloatRectangle(100,100,100,100);

    FloatRectangle layerDest = FloatRectangle(0,0,200,200);
    FloatRectangle layerSRC = FloatRectangle(100,100,100,100);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);

    ASSERT_EQ(200.0,targetSurfaceDest.width);
    ASSERT_EQ(0.0,targetSurfaceDest.x);
    ASSERT_EQ(200.0,targetSurfaceDest.height);
    ASSERT_EQ(0.0,targetSurfaceDest.y);
    ASSERT_NEAR(0.0,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
}

TEST_F(ViewportTransformTest, completeExample3){
    int surfaceOriginalWidth =100;
    int surfaceOriginalHeight = 100;
    FloatRectangle targetSurfaceSrc = FloatRectangle(50,50,50,50);
    FloatRectangle targetSurfaceDest = FloatRectangle(100,100,100,100);

    FloatRectangle layerDest = FloatRectangle(0,0,200,200);
    FloatRectangle layerSRC = FloatRectangle(100,100,100,100);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);

    ASSERT_EQ(200.0,targetSurfaceDest.width);
    ASSERT_EQ(0.0,targetSurfaceDest.x);
    ASSERT_EQ(200.0,targetSurfaceDest.height);
    ASSERT_EQ(0.0,targetSurfaceDest.y);
    ASSERT_NEAR(0.5,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.5,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
}

TEST_F(ViewportTransformTest, completeExample4){
    int surfaceOriginalWidth =100;
    int surfaceOriginalHeight = 100;
    FloatRectangle targetSurfaceSrc = FloatRectangle(50,50,50,50);
    FloatRectangle targetSurfaceDest = FloatRectangle(100,100,100,100);

    FloatRectangle layerDest = FloatRectangle(50,50,200,200);
    FloatRectangle layerSRC = FloatRectangle(100,100,100,100);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);

    ASSERT_EQ(200.0,targetSurfaceDest.width);
    ASSERT_EQ(50.0,targetSurfaceDest.x);
    ASSERT_EQ(200.0,targetSurfaceDest.height);
    ASSERT_EQ(50.0,targetSurfaceDest.y);
    ASSERT_NEAR(0.5,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.5,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
}

TEST_F(ViewportTransformTest, completeExample5){
    int surfaceOriginalWidth =320;
    int surfaceOriginalHeight = 320;

    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,320,320);
    FloatRectangle targetSurfaceDest = FloatRectangle(0,0,320,320);

    FloatRectangle layerSRC = FloatRectangle(100,100,1280,1280);
    FloatRectangle layerDest = FloatRectangle(0,0,1280,1280);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);
    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);

    ASSERT_EQ(0.0,targetSurfaceDest.x);
    ASSERT_EQ(220.0,targetSurfaceDest.width);
    ASSERT_EQ(0.0,targetSurfaceDest.y);
    ASSERT_EQ(220.0,targetSurfaceDest.height);
    ASSERT_NEAR(0.3125,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.3125,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
}

TEST_F(ViewportTransformTest, completeExample6){
    int surfaceOriginalWidth =320;
    int surfaceOriginalHeight =320;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,320,320);
    FloatRectangle targetSurfaceDest = FloatRectangle(320,320,320,320);

    FloatRectangle layerSRC = FloatRectangle(100,100,1280,1280);
    FloatRectangle layerDest = FloatRectangle(0,0,1280,1280);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);

    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_EQ(220.0,targetSurfaceDest.x);
    ASSERT_EQ(320.0,targetSurfaceDest.width);
    ASSERT_EQ(220.0,targetSurfaceDest.y);
    ASSERT_EQ(320.0,targetSurfaceDest.height);

    ASSERT_EQ(0.0,targetSurfaceSrc.x);
    ASSERT_EQ(320.0,targetSurfaceSrc.width);
    ASSERT_EQ(0.0,targetSurfaceSrc.y);
    ASSERT_EQ(320.0,targetSurfaceSrc.height);

    ASSERT_NEAR(0.0,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
}


TEST_F(ViewportTransformTest, completeExample7){
    int surfaceOriginalWidth =320;
    int surfaceOriginalHeight =240;
    FloatRectangle targetSurfaceSrc = FloatRectangle(0,0,320,240);
    FloatRectangle targetSurfaceDest = FloatRectangle(0,0,320,240);

    FloatRectangle layerSRC = FloatRectangle(100,0,80,240);
    FloatRectangle layerDest = FloatRectangle(100,0,640,480);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);

    ASSERT_EQ(100.0,targetSurfaceSrc.x);
    ASSERT_EQ(80.0,targetSurfaceSrc.width);
    ASSERT_EQ(0.0,targetSurfaceSrc.y);
    ASSERT_EQ(240.0,targetSurfaceSrc.height);

    ASSERT_EQ(100.0,targetSurfaceDest.x);
    ASSERT_EQ(640.0,targetSurfaceDest.width);
    ASSERT_EQ(0.0,targetSurfaceDest.y);
    ASSERT_EQ(480.0,targetSurfaceDest.height);

    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.3125,textureCoordinates[0],0.01);
    ASSERT_NEAR(0.5625,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
}

TEST_F(ViewportTransformTest, completeExample8){
    int surfaceOriginalWidth =320;
    int surfaceOriginalHeight =240;
    FloatRectangle targetSurfaceSrc = FloatRectangle(1,0,320,240);
    FloatRectangle targetSurfaceDest = FloatRectangle(0,0,320,240);

    FloatRectangle layerSRC = FloatRectangle(0,0,1280,480);
    FloatRectangle layerDest = FloatRectangle(0,0,1280,480);

    ViewportTransform::applyLayerSource(layerSRC,targetSurfaceSrc,targetSurfaceDest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurfaceDest);

    ASSERT_EQ(1.0,targetSurfaceSrc.x);
    ASSERT_EQ(320.0,targetSurfaceSrc.width);
    ASSERT_EQ(0.0,targetSurfaceSrc.y);
    ASSERT_EQ(240.0,targetSurfaceSrc.height);

    ASSERT_EQ(0.0,targetSurfaceDest.x);
    ASSERT_EQ(320.0,targetSurfaceDest.width);
    ASSERT_EQ(0.0,targetSurfaceDest.y);
    ASSERT_EQ(240.0,targetSurfaceDest.height);

    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSrc, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.003125,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);
}

TEST_F(ViewportTransformTest, layersourceZoomOnTwoSurfaces){
    int surfaceOriginalWidth = 800;
    int surfaceOriginalHeight = 480;
    FloatRectangle layerSRC = FloatRectangle(100, 0, 600, 480);
    FloatRectangle layerDest = FloatRectangle(0, 0, 800, 480);

    FloatRectangle targetSurface1Src = FloatRectangle(0, 0, 800, 480);
    FloatRectangle targetSurface1Dest = FloatRectangle(0, 0, 400, 480);

    ViewportTransform::applyLayerSource(layerSRC,targetSurface1Src,targetSurface1Dest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurface1Dest);

    ASSERT_EQ(200.0, targetSurface1Src.x);
    ASSERT_EQ(600.0, targetSurface1Src.width);

    ASSERT_EQ(0.0,targetSurface1Dest.x);
    ASSERT_EQ(400.0,targetSurface1Dest.width);

    float* textureCoordinates = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurface1Src, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates);
    ASSERT_NEAR(0.25,textureCoordinates[0],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates[3],0.01);

    FloatRectangle targetSurface2Src = FloatRectangle(0, 0, 800, 480);
    FloatRectangle targetSurface2Dest = FloatRectangle(400, 0, 400, 480);

    ViewportTransform::applyLayerSource(layerSRC,targetSurface2Src,targetSurface2Dest);
    ViewportTransform::applyLayerDestination(layerDest,layerSRC,targetSurface2Dest);

    ASSERT_EQ(0.0, targetSurface2Src.x);
    ASSERT_EQ(600.0, targetSurface2Src.width);

    ASSERT_EQ(400.0,targetSurface2Dest.x);
    ASSERT_EQ(400.0,targetSurface2Dest.width);

    float* textureCoordinates2 = new float[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurface2Src, surfaceOriginalWidth, surfaceOriginalHeight, textureCoordinates2);
    ASSERT_NEAR(0,textureCoordinates2[0],0.01);
    ASSERT_NEAR(0.75,textureCoordinates2[2],0.01);
    ASSERT_NEAR(0.0,textureCoordinates2[1],0.01);
    ASSERT_NEAR(1.0,textureCoordinates2[3],0.01);
}

TEST_F(ViewportTransformTest, IsFullyCroppedLeft){
    Rectangle surfaceDestination(5,30,5,50);
    Rectangle layerSource(20,0,20,100);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_TRUE(result);
}

TEST_F(ViewportTransformTest, IsFullyCroppedRight){
    Rectangle surfaceDestination(30,30,50,50);
    Rectangle layerSource(100,0,20,100);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_TRUE(result);
}

TEST_F(ViewportTransformTest, IsFullyCroppedAbove){
    Rectangle surfaceDestination(0,5,100,15);
    Rectangle layerSource(0,25,100,80);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_TRUE(result);
}

TEST_F(ViewportTransformTest, IsFullyCroppedBelow){
    Rectangle surfaceDestination(0,110,100,50);
    Rectangle layerSource(0,50,100,50);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_TRUE(result);
}

TEST_F(ViewportTransformTest, IsFullyCroppedLeftBoundary){
    Rectangle surfaceDestination(5,30,5,50);
    Rectangle layerSource(10,0,20,100);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_TRUE(result);
}

TEST_F(ViewportTransformTest, IsFullyCroppedRightBoundary){
    Rectangle surfaceDestination(30,30,50,50);
    Rectangle layerSource(80,0,20,100);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_TRUE(result);
}

TEST_F(ViewportTransformTest, IsFullyCroppedAboveBoundary){
    Rectangle surfaceDestination(0,5,100,15);
    Rectangle layerSource(0,20,100,80);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_TRUE(result);
}

TEST_F(ViewportTransformTest, IsFullyCroppedBelowBoundary){
    Rectangle surfaceDestination(0,100,100,50);
    Rectangle layerSource(0,50,100,50);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_TRUE(result);
}

TEST_F(ViewportTransformTest, IsNotFullyCroppedLeft){
    Rectangle surfaceDestination(0, 20, 20, 10);
    Rectangle layerSource(10, 10, 30, 30);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_FALSE(result);
}

TEST_F(ViewportTransformTest, IsNotFullyCroppedRight){
    Rectangle surfaceDestination(30, 20, 20, 10);
    Rectangle layerSource(10, 10, 30, 30);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_FALSE(result);
}

TEST_F(ViewportTransformTest, IsNotFullyCroppedTop){
    Rectangle surfaceDestination(20, 0, 10, 20);
    Rectangle layerSource(10, 10, 30, 30);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_FALSE(result);
}

TEST_F(ViewportTransformTest, IsNotFullyCroppedBottom){
    Rectangle surfaceDestination(20, 30, 10, 20);
    Rectangle layerSource(10, 10, 30, 30);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_FALSE(result);
}

TEST_F(ViewportTransformTest, IsNotFullyCropped_SurfaceWithinLayerSource){
    Rectangle surfaceDestination(12,12,5,5);
    Rectangle layerSource(10, 10, 10, 10);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_FALSE(result);
}

TEST_F(ViewportTransformTest, IsNotFullyCropped_SurfaceLargerThanLayersource){
    Rectangle surfaceDestination(5,5,10,10);
    Rectangle layerSource(10, 10, 10, 10);

    bool result = ViewportTransform::isFullyCropped(surfaceDestination,layerSource);

    ASSERT_FALSE(result);
}
