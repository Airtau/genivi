/***************************************************************************
 *
 * Copyright 2012 BMW Car IT GmbH
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

#include "ilm_client.h"
#include "LMControl.h"

#include <cmath>
using std::min;
using std::max;

#include <cstdlib>


#include <iostream>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;

#include <iomanip>
using std::dec;
using std::hex;

#include <unistd.h>

#include <vector>
using std::vector;



namespace
{

typedef void (*t_pDemoAnimatorFunc)(t_scene_data* pInitialScene, t_scene_data* pDemoScene, bool* pStopDemo);

void demoAnimatorDownwards(t_scene_data* pInitialScene, t_scene_data* pDemoScene, bool* pStopDemo)
{
    //get rendered surfaces
    vector<t_ilm_surface> renderedSurfaces = getSceneRenderOrder(pDemoScene);

    //make random scale factors to scale down surfaces
    float maxScale = 0.7;
    float minScale = 0.2;
    float rangeScale = maxScale - minScale;

    srand((unsigned) time(0));
    map<t_ilm_surface, t_ilm_float> surfaceScale;

    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        surfaceScale[surface] = minScale + rangeScale * (1.0 * rand() / RAND_MAX);
    }

    //make random speeds (in pixel)
    t_ilm_int maxSpeed = 6;
    t_ilm_int minSpeed = 2;
    t_ilm_int rangeSpeed = maxSpeed - minSpeed;

    srand((unsigned) time(0));
    map<t_ilm_surface, t_ilm_int> surfaceSpeed;
    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        surfaceSpeed[surface] = minSpeed + static_cast<t_ilm_int>(rangeScale* (1.0 * rand() / RAND_MAX));
        surfaceSpeed[surface] = minSpeed +
                (static_cast<t_ilm_int>(surfaceSpeed[surface] * (pow(2 + surfaceScale[surface], 2) - 4)));
    }

    //set random direction
    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        //set 50% of surfaces to move in -ve y direction
        surfaceSpeed[surface] *= rand() % 2 ? -1 : 1;
    }


    //get surface positions and set initial positions
    map<t_ilm_surface, tuple4> surfaceCoordinates;

    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        ilmSurfaceProperties& surfaceProperties =
                pDemoScene->surfaceProperties[surface];

        //get current destination region AND scale surface dimensions
        tuple4 coordinates(surfaceProperties.destX,
                surfaceProperties.destY,
                surfaceProperties.destX + static_cast<int>(surfaceScale[surface] * surfaceProperties.destWidth),
                surfaceProperties.destY + static_cast<int>(surfaceScale[surface] * surfaceProperties.destHeight));

        //make random X position to make surfaces spread in the screen
        int screenWidthMidpoint = pDemoScene->screenWidth / 2;
        int surfaceWidth = coordinates.z - coordinates.x;

        //make a random X position so that the surface stays totally displayable inside the screen
        int maxX = pDemoScene->screenWidth - surfaceWidth;
        int xRandom = static_cast<int>(maxX * 1.0 * rand() / RAND_MAX);

        coordinates.x = xRandom;
        coordinates.z = xRandom + surfaceWidth;

        surfaceCoordinates[surface] = coordinates;
    }

    //sleeping time
    long sleepTimeMillis = 25;

    //start animation !
    while(! *pStopDemo)
    {
        for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
                it != renderedSurfaces.end(); ++it)
        {
            t_ilm_surface surface = *it;
            ilmSurfaceProperties& properties = pDemoScene->surfaceProperties[surface];
            tuple4& coordinates = surfaceCoordinates[surface];
            int speed = surfaceSpeed[surface];

            //if out: get back to screen
            if (coordinates.y >= static_cast<int>(pDemoScene->screenHeight))
            {
                //reset to top
                coordinates.y = coordinates.y - coordinates.w;
                coordinates.w = 0;
            }

            //move
            coordinates.y += abs(surfaceSpeed[surface]);
            coordinates.w += abs(surfaceSpeed[surface]);

            //if the upper part is not visible remove it from the source and destination regions
            if (coordinates.y <= 0)
            {
                //set source region to only the visible part of the surface
                int ySource = static_cast<int>(-coordinates.y / surfaceScale[surface]);
                ilm_surfaceSetSourceRectangle(surface, 0, ySource, properties.sourceWidth, properties.sourceHeight - ySource);

                //set the destination region AT THE TOP of the layer used for displaying the surface
                ilm_surfaceSetDestinationRectangle(surface,
                        coordinates.x,
                        0,
                        max(0, coordinates.z - coordinates.x),
                        max(0, coordinates.w));
            }
            else
            {
                //set source region to whole surface
                ilm_surfaceSetSourceRectangle(surface, 0, 0, properties.sourceWidth, properties.sourceHeight);

                //set destination region to the region on the layer used for displaying the surface
                ilm_surfaceSetDestinationRectangle(surface,
                        coordinates.x,
                        coordinates.y,
                        max(0, coordinates.z - coordinates.x),
                        max(0, coordinates.w - coordinates.y));
            }

            surfaceCoordinates[surface] = coordinates;
        }

        ilm_commitChanges();
        usleep(sleepTimeMillis * 1000);
    }
}

void demoAnimatorRandomDirections(t_scene_data* pInitialScene, t_scene_data* pDemoScene, bool* pStopDemo)
{
    //get rendered surfaces
    vector<t_ilm_surface> renderedSurfaces = getSceneRenderOrder(pDemoScene);

    //make random scale factors to scale down surfaces
    float maxScale = 0.7;
    float minScale = 0.2;
    float rangeScale = maxScale - minScale;

    srand((unsigned) time(0));
    map<t_ilm_surface, t_ilm_float> surfaceScale;
    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        surfaceScale[surface] = minScale + rangeScale * (1.0 * rand() / RAND_MAX);
    }

    //make random speeds (in pixel)
    t_ilm_int maxSpeed = 12;
    t_ilm_int minSpeed = 2;
    t_ilm_int rangeSpeed = maxSpeed - minSpeed;

    srand((unsigned) time(0));
    map<t_ilm_surface, t_ilm_int> surfaceSpeed;
    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        surfaceSpeed[surface] = minSpeed + static_cast<t_ilm_int>(rangeScale * (1.0 * rand() / RAND_MAX));
        surfaceSpeed[surface] = minSpeed
                + (static_cast<t_ilm_int>(surfaceSpeed[surface] * (pow(2 + surfaceScale[surface], 2) - 4)));
    }

    //set random direction
    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        //set 50% of surfaces to move in -ve y direction
        surfaceSpeed[surface] *= rand() % 2 ? -1 : 1;
    }

    //get surface positions and set initial positions
    map<t_ilm_surface, tuple4> surfaceCoordinates;

    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        ilmSurfaceProperties& surfaceProperties = pDemoScene->surfaceProperties[surface];

        //get current destination region AND scale surface dimensions
        tuple4 coordinates(surfaceProperties.destX,
                surfaceProperties.destY,
                surfaceProperties.destX + static_cast<int>(surfaceScale[surface] * surfaceProperties.destWidth),
                surfaceProperties.destY + static_cast<int>(surfaceScale[surface] * surfaceProperties.destHeight));

        //make random X position to make surfaces spread in the screen
        int screenWidthMidpoint = pDemoScene->screenWidth / 2;
        int surfaceWidth = coordinates.z - coordinates.x;

        //make a random X position so that the surface stays totally displayable inside the screen
        int maxX = pDemoScene->screenWidth - surfaceWidth;
        int xRandom = static_cast<int>(maxX * 1.0 * rand() / RAND_MAX);

        coordinates.x = xRandom;
        coordinates.z = xRandom + surfaceWidth;

        surfaceCoordinates[surface] = coordinates;
    }

    //sleeping time
    long sleepTimeMillis = 25;

    //start animation !
    while(! *pStopDemo)
    {
        //set transparency to be inversly proportional to scale
        for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
                it != renderedSurfaces.end(); ++it)
        {
            t_ilm_surface surface = *it;

            float opacity = min(1.0, 1.1 - surfaceScale[surface]);
            ilm_surfaceSetOpacity(surface, opacity);
        }

        for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
                it != renderedSurfaces.end(); ++it)
        {
            t_ilm_surface surface = *it;
            ilmSurfaceProperties& properties = pDemoScene->surfaceProperties[surface];
            tuple4& coordinates = surfaceCoordinates[surface];
            int speed = surfaceSpeed[surface];

            //if out: get back to screen
            if (coordinates.y >= static_cast<int>(pDemoScene->screenHeight) && speed > 0)
            {
                //reset to top
                coordinates.y = coordinates.y - coordinates.w;
                coordinates.w = 0;
            }
            else if (coordinates.w <= 0 && speed < 0)
            {
                coordinates.w = pDemoScene->screenHeight + coordinates.w - coordinates.y;
                coordinates.y = pDemoScene->screenHeight;
            }

            //move
            coordinates.y += surfaceSpeed[surface];
            coordinates.w += surfaceSpeed[surface];


            //if the upper part is not visible remove it from the source and destination regions
            if (coordinates.y <= 0)
            {
                //set source region to only the visible part of the surface
                int ySource = static_cast<int>(-coordinates.y / surfaceScale[surface]);
                ilm_surfaceSetSourceRectangle(surface, 0, ySource, properties.sourceWidth, properties.sourceHeight - ySource);

                //set the destination region AT THE TOP of the layer used for displaying the surface
                ilm_surfaceSetDestinationRectangle(surface,
                        coordinates.x,
                        0,
                        max(0, coordinates.z - coordinates.x),
                        max(0, coordinates.w));
            }
            else
            {
                //set source region to whole surface
                ilm_surfaceSetSourceRectangle(surface, 0, 0, properties.sourceWidth, properties.sourceHeight);

                //set destination region to the region on the layer used for displaying the surface
                ilm_surfaceSetDestinationRectangle(surface,
                        coordinates.x,
                        coordinates.y,
                        max(0, coordinates.z - coordinates.x),
                        max(0, coordinates.w - coordinates.y));
            }

            surfaceCoordinates[surface] = coordinates;
        }

        ilm_commitChanges();
        usleep(sleepTimeMillis * 1000);
    }
}

void demoAnimatorWaterfall(t_scene_data* pInitialScene, t_scene_data* pDemoScene, bool* pStopDemo)
{
    //get rendered surfaces
    vector<t_ilm_surface> renderedSurfaces = getSceneRenderOrder(pDemoScene);

    //make random scale factors to scale down surfaces
    float maxScale = 0.7;
    float minScale = 0.2;
    float rangeScale = maxScale - minScale;

    srand((unsigned) time(0));
    map<t_ilm_surface, t_ilm_float> surfaceScale;
    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        surfaceScale[surface] = minScale + rangeScale * (1.0 * rand() / RAND_MAX);
    }

    //make random speeds (in pixel)
    t_ilm_int maxSpeed = 12;
    t_ilm_int minSpeed = 2;
    t_ilm_int rangeSpeed = maxSpeed - minSpeed;

    srand((unsigned) time(0));
    map<t_ilm_surface, t_ilm_int> surfaceSpeed;
    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        surfaceSpeed[surface] = minSpeed + static_cast<t_ilm_int>(rangeScale * (1.0 * rand() / RAND_MAX));
        surfaceSpeed[surface] = minSpeed
                + (static_cast<t_ilm_int>(surfaceSpeed[surface] * (pow(2 + surfaceScale[surface], 2) - 4)));
    }

    //set random direction
    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        //set 50% of surfaces to move in -ve y direction
        surfaceSpeed[surface] *= rand() % 2 ? -1 : 1;
    }

    //get surface positions and set initial positions
    map<t_ilm_surface, tuple4> surfaceCoordinates;

    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        ilmSurfaceProperties& surfaceProperties = pDemoScene->surfaceProperties[surface];

        //get current destination region AND scale surface dimensions
        tuple4 coordinates(surfaceProperties.destX,
                surfaceProperties.destY,
                surfaceProperties.destX + static_cast<int>(surfaceScale[surface] * surfaceProperties.destWidth),
                surfaceProperties.destY + static_cast<int>(surfaceScale[surface] * surfaceProperties.destHeight));

        //make random X position to make surfaces spread in the screen
        int screenWidthMidpoint = pDemoScene->screenWidth / 2;
        int surfaceWidth = coordinates.z - coordinates.x;

        //make a random X position so that the surface stays totally displayable inside the screen
        int maxX = pDemoScene->screenWidth - surfaceWidth;
        int xRandom = static_cast<int>(maxX * 1.0 * rand() / RAND_MAX);

        coordinates.x = xRandom;
        coordinates.z = xRandom + surfaceWidth;

        surfaceCoordinates[surface] = coordinates;
    }

    //sleeping time
    long sleepTimeMillis = 25;

    //start animation !
    while(! *pStopDemo)
    {
        for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
                it != renderedSurfaces.end(); ++it)
        {
            t_ilm_surface surface = *it;
            ilmSurfaceProperties& properties = pDemoScene->surfaceProperties[surface];
            tuple4& coordinates = surfaceCoordinates[surface];
            int speed = surfaceSpeed[surface];

            //if out: get back to screen
            if (coordinates.y >= static_cast<int>(pDemoScene->screenHeight))
            {
                //reset to top
                coordinates.y = coordinates.y - coordinates.w;
                coordinates.w = 0;
            }

            t_ilm_float fraction = max(0.0, 1.0 * (coordinates.w) / (pDemoScene->screenHeight + coordinates.w - coordinates.y));
            t_ilm_float t = pow(3, 0.0251 + fraction);
            int displacement = static_cast<int>(t * abs(surfaceSpeed[surface]));

            t_ilm_float opacity = min(1.0, max(0.0, 1 - fraction)); //between 0 and 1
            ilm_surfaceSetOpacity(surface, opacity);

            //move
            coordinates.y += displacement;
            coordinates.w += displacement;

            //if the upper part is not visible remove it from the source and destination regions
            if (coordinates.y <= 0)
            {
                //set source region to only the visible part of the surface
                int ySource = static_cast<int>(-coordinates.y / surfaceScale[surface]);
                ilm_surfaceSetSourceRectangle(surface,
                        0,
                        ySource,
                        properties.sourceWidth,
                        properties.sourceHeight - ySource);

                //set the destination region AT THE TOP of the layer used for displaying the surface
                ilm_surfaceSetDestinationRectangle(surface,
                        coordinates.x,
                        0,
                        max(0, coordinates.z - coordinates.x),
                        max(0, coordinates.w));
            }
            else
            {
                //set source region to whole surface
                ilm_surfaceSetSourceRectangle(surface, 0, 0, properties.sourceWidth, properties.sourceHeight);

                //set destination region to the region on the layer used for displaying the surface
                ilm_surfaceSetDestinationRectangle(surface,
                        coordinates.x,
                        coordinates.y,
                        max(0, coordinates.z - coordinates.x),
                        max(0, coordinates.w - coordinates.y));
            }

            surfaceCoordinates[surface] = coordinates;
        }

        ilm_commitChanges();
        usleep(sleepTimeMillis * 1000);
    }
}

void demoAnimatorZooming(t_scene_data* pInitialScene, t_scene_data* pDemoScene, bool* pStopDemo)
{
    //get rendered surfaces
    vector<t_ilm_surface> renderedSurfaces = getSceneRenderOrder(pDemoScene);

    //set surface initial states
    int screenHorizontalMidpoint = pDemoScene->screenWidth / 2;
    int screenVerticalMidpoint = pDemoScene->screenHeight / 2;

    for (vector<t_ilm_surface>::iterator it = renderedSurfaces.begin();
            it != renderedSurfaces.end(); ++it)
    {
        t_ilm_surface surface = *it;
        ilmSurfaceProperties& surfaceProperties = pDemoScene->surfaceProperties[surface];

        surfaceProperties.destX = screenHorizontalMidpoint;
        surfaceProperties.destY = screenVerticalMidpoint;
        surfaceProperties.destHeight = 0;
        surfaceProperties.destWidth = 0;

        ilm_surfaceSetDestinationRectangle(surface,
                surfaceProperties.destX,
                surfaceProperties.destY,
                surfaceProperties.destWidth,
                surfaceProperties.destHeight);
    }

    //sleeping time
    long sleepTimeMillis = 25;

    //start animation !

    t_ilm_float t = 1;

    int currentSurfaceIndex = 0;

    while (!*pStopDemo)
    {
        t_ilm_surface surface = renderedSurfaces[currentSurfaceIndex];

        ilmSurfaceProperties& surfaceProperties = pDemoScene->surfaceProperties[surface];
        //if currentsurface dimensions touch the screen: change surface and reset scale factor
        if (surfaceProperties.destX == 0 || surfaceProperties.destY == 0)
        {
            t = 1;
            surfaceProperties.destX = screenHorizontalMidpoint;
            surfaceProperties.destY = screenVerticalMidpoint;
            surfaceProperties.destHeight = 0;
            surfaceProperties.destWidth = 0;

            currentSurfaceIndex = (currentSurfaceIndex + 1) % renderedSurfaces.size();
            t_ilm_layer layer = pDemoScene->screenLayers.begin()->second[0];

            ilm_surfaceSetDestinationRectangle(surface,
                    surfaceProperties.destX,
                    surfaceProperties.destY,
                    surfaceProperties.destWidth,
                    surfaceProperties.destHeight);

            ilm_layerSetRenderOrder(layer, renderedSurfaces.data() + currentSurfaceIndex, 1);
        }
        else
        {
            t += t * 0.1;
            int change = (int) t;
            surfaceProperties.destX = max(0, (int) surfaceProperties.destX - change);
            surfaceProperties.destY = max(0, (int) surfaceProperties.destY - change);
            surfaceProperties.destWidth += 2 * change;
            surfaceProperties.destHeight += 2 * change;

            ilm_surfaceSetDestinationRectangle(surface,
                    surfaceProperties.destX,
                    surfaceProperties.destY,
                    surfaceProperties.destWidth,
                    surfaceProperties.destHeight);

            float opacity = 1 / pow(t, 0.4);
            ilm_surfaceSetOpacity(surface, opacity);
        }

        ilm_commitChanges();
        usleep(sleepTimeMillis * 1000);
    }
}

void demoAnimatorCascadedZooming(t_scene_data* pInitialScene, t_scene_data* pDemoScene, bool* pStopDemo)
{
    //get rendered surfaces
    vector<t_ilm_surface> renderedSurfaces = getSceneRenderOrder(pDemoScene);

    map<t_ilm_surface, float> scaleFactors;
    //set surface initial states
    int screenHorizontalMidpoint = pDemoScene->screenWidth / 2;
    int screenVerticalMidpoint = pDemoScene->screenHeight / 2;

    for (int i = 0; i < renderedSurfaces.size(); ++i)
    {
        t_ilm_surface surface = renderedSurfaces[i];
        ilmSurfaceProperties& surfaceProperties = pDemoScene->surfaceProperties[surface];

        surfaceProperties.destX = screenHorizontalMidpoint;
        surfaceProperties.destY = screenVerticalMidpoint;
        surfaceProperties.destHeight = 0;
        surfaceProperties.destWidth = 0;

        ilm_surfaceSetDestinationRectangle(surface,
                surfaceProperties.destX,
                surfaceProperties.destY,
                surfaceProperties.destWidth,
                surfaceProperties.destHeight);

        scaleFactors[surface] = 1;
    }

    //sleeping time
    long sleepTimeMillis = 25;

    //start animation !

    while (!*pStopDemo)
    {
        for (int i = 0; i < renderedSurfaces.size(); ++i)
        {
            t_ilm_surface surface = renderedSurfaces[i];
            ilmSurfaceProperties& surfaceProperties = pDemoScene->surfaceProperties[surface];
            //if currentsurface dimensions touch the screen: change surface and reset scale factor
            if (surfaceProperties.destX == 0 || surfaceProperties.destY == 0)
            {
                scaleFactors[surface] = 1;

                surfaceProperties.destX = screenHorizontalMidpoint;
                surfaceProperties.destY = screenVerticalMidpoint;
                surfaceProperties.destHeight = 0;
                surfaceProperties.destWidth = 0;

                t_ilm_layer layer = pDemoScene->screenLayers.begin()->second[0];

                ilm_surfaceSetDestinationRectangle(surface,
                        surfaceProperties.destX,
                        surfaceProperties.destY,
                        surfaceProperties.destWidth,
                        surfaceProperties.destHeight);

                ilm_commitChanges();
                //update render order
                t_ilm_surface firstSurface = renderedSurfaces[0];
                for (int j = 1; j < renderedSurfaces.size(); ++j)
                {
                    renderedSurfaces[j - 1] = renderedSurfaces[j];
                }

                renderedSurfaces[renderedSurfaces.size() - 1] = firstSurface;

                ilm_layerSetRenderOrder(layer, renderedSurfaces.data(), renderedSurfaces.size());
                ilm_commitChanges();
            }
            else
            {
                //just some fancy function math that gives a special effect
                scaleFactors[surface] = (1.0125, i + scaleFactors[surface] * 0.5);

                int change = (int) scaleFactors[surface];
                surfaceProperties.destX = max(0, (int) surfaceProperties.destX - change);
                surfaceProperties.destY = max(0, (int) surfaceProperties.destY - change);
                surfaceProperties.destWidth += 2 * change;
                surfaceProperties.destHeight += 2 * change;

                ilm_surfaceSetDestinationRectangle(surface,
                        surfaceProperties.destX,
                        surfaceProperties.destY,
                        surfaceProperties.destWidth,
                        surfaceProperties.destHeight);

                float opacity = 1 / pow(scaleFactors[surface], 0.4f);
                ilm_surfaceSetOpacity(surface, opacity);
            }
        }

        ilm_commitChanges();
        usleep(sleepTimeMillis * 1000);
    }
}

void* demoThreadCallback(void* param)
{
    //call function on parameters
    map<string,void*>* paramMap = (map<string,void*>*) param;

    t_pDemoAnimatorFunc animator = (t_pDemoAnimatorFunc) paramMap->at("pAnimator");
    t_scene_data* pInitialScene = (t_scene_data*) paramMap->at("pInitialScene");
    t_scene_data* pDemoScene = (t_scene_data*) paramMap->at("pDemoScene");
    bool*  pStopDemo= (bool*) paramMap->at("pStopDemo");

    //call demo animator function !
    (*animator)(pInitialScene, pDemoScene, pStopDemo);

    return NULL;
}

} //end of anonymous namespace

void demo(int mode)
{
    vector<t_pDemoAnimatorFunc> animators;
    animators.push_back(&demoAnimatorZooming);
    animators.push_back(&demoAnimatorCascadedZooming);
    animators.push_back(&demoAnimatorDownwards);
    animators.push_back(&demoAnimatorRandomDirections);
    animators.push_back(&demoAnimatorWaterfall);

    //if valid mode entered
    if (mode >= 0 && mode < animators.size())
    {
        //capture initial scene
        t_scene_data initialScene;
        captureSceneData(&initialScene);

        setSceneToRestore(&initialScene);

        //set dummy scene
        t_scene_data demoScene = cloneToUniLayerScene(&initialScene);
        setScene(&demoScene);

        //start animation
        volatile bool stopDemo = false;

        map<string, void*> paramMap;
        paramMap["pAnimator"] = (void*) animators[mode];
        paramMap["pInitialScene"] = (void*) &initialScene;
        paramMap["pDemoScene"] = (void*) &demoScene;
        paramMap["pStopDemo"] = (void*) &stopDemo;

        pthread_t demoThread;
        pthread_create(&demoThread, NULL, demoThreadCallback, (void*) &paramMap);

        //set initial scene (reset)
        cout<<"Press ENTER to stop demo...";
        cin.get();

        cout<<"Resetting scene...";
        cout.flush();

        stopDemo = true; //this variable has to be checked in the animator function !!
        pthread_join(demoThread, NULL);

        t_scene_data finalScene;
        captureSceneData(&finalScene);
        transformScene(&finalScene, &initialScene, 1000, 50);
        setScene(&initialScene, true);
        cout<<"done"<<endl;
    }
    else
    {
        cerr << "fatal error :" << mode << " is not a valid mode, please choose a mode between " << 0
                << " and " << animators.size() - 1 << endl;
        exit(0);
    }
}
