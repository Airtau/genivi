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
#include "ilm_client.h"
#include "LayerScene.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LAYERWIDTH 800
#define LAYERHEIGHT 480

void doSceneLayeout(t_ilm_int LayerWidth, t_ilm_int LayerHeight)
{
    printf("Do Scene Layout\n");
    ilm_surfaceSetDestinationRectangle(SURFACE_EXAMPLE_VIDEO_APPLICATION, 0, 0, LayerWidth, LayerHeight);
    ilm_surfaceSetSourceRectangle(SURFACE_EXAMPLE_VIDEO_APPLICATION, 107, 0, 426, 360);
    ilm_surfaceSetDestinationRectangle(SURFACE_EXAMPLE_GDTESTENV_APPLICATION_1, LayerWidth - 160, 0, 160, 120);
    ilm_surfaceSetDestinationRectangle(SURFACE_EXAMPLE_EGLX11_APPLICATION, LayerWidth - 320, LayerHeight - 240, 320, 240);
    ilm_commitChanges();
    printf("Do Scene Layout done\n");
}

void RemoveSceneLayout()
{
    printf("Remove Scene Layout\n");
    ilm_layerRemove(LAYER_EXAMPLE_VIDEO_APPLICATIONS);
    ilm_layerRemove(LAYER_EXAMPLE_GLES_APPLICATIONS);
    ilm_layerRemove(LAYER_EXAMPLE_X_APPLICATIONS);
    printf("Remove Scene Layout done\n");
}

void changeLayerOpacity()
{
    printf("change opacity of layers\n");
    ilm_layerSetOpacity(LAYER_EXAMPLE_VIDEO_APPLICATIONS, 0.0);
    ilm_layerSetOpacity(LAYER_EXAMPLE_GLES_APPLICATIONS, 1.0);
    ilm_layerSetOpacity(LAYER_EXAMPLE_X_APPLICATIONS, 1.0);
    ilm_commitChanges();
    printf("change opacity of layers done\n");
}

ilmErrorTypes init()
{
    ilmErrorTypes result = ILM_FAILED;

    result = ilm_init();

    if(ILM_SUCCESS == result)
    {
        printf("ilm_init success\n");
    }
    else
    {
        printf("ilm_init failed\n");
    }

    return result;
}

ilmErrorTypes createLayers(t_ilm_uint LayerWidth, t_ilm_uint LayerHeight)
{
    int index = 0;
    ilmErrorTypes error = ILM_FAILED;
    t_ilm_layer layer[3] = { LAYER_EXAMPLE_VIDEO_APPLICATIONS,
                             LAYER_EXAMPLE_GLES_APPLICATIONS,
                             LAYER_EXAMPLE_X_APPLICATIONS};

    for(index = 0; index < 3; ++index)
    {
        printf("Creating Layer... ");
        error = ilm_layerCreateWithDimension(&layer[index],LayerWidth,LayerHeight);
        if(ILM_SUCCESS == error)
        {
            printf("success\n");
        }
        else
        {
            printf("fail\n");
        }

        printf("Setting Layer destination rectangle(0, 0, %d, %d)... \n", LayerWidth, LayerHeight);
        error = ilm_layerSetDestinationRectangle(layer[index], 0, 0, LayerWidth, LayerHeight);
        printf("Setting Layer source rectangle(0, 0, %d, %d)... \n", LayerWidth, LayerHeight);
        error = ilm_layerSetSourceRectangle(layer[index], 0, 0, LayerWidth, LayerHeight);
        printf("Setting Layer visibility(%d)... \n", 1);
        error = ilm_layerSetVisibility(layer[index],1);
        if(ILM_SUCCESS == error)
        {
            printf("success\n");
        }
        else
        {
            printf("fail\n");
        }
    }

    ilm_displaySetRenderOrder(0, &layer[0], 3);
    ilm_commitChanges();

    return error;
}

void takeScreenShot(t_ilm_string filename)
{
    printf("taking screenshot...");
    ilm_takeScreenshot(0, filename);
}

void shutdown()
{
    ilmErrorTypes error = ILM_FAILED;
    printf("Calling ilm_destroy... ");
    error = ilm_destroy();

    if(ILM_SUCCESS == error)
    {
        printf("success\n");
    }
    else
    {
        printf("failed\n");
    }
}

int main(int argc, char **argv)
{
    t_ilm_uint LayerWidth = LAYERWIDTH;
    t_ilm_uint LayerHeight = LAYERHEIGHT;

    if (ILM_SUCCESS != init())
    {
        return -1;
    }

    ilm_getScreenResolution(0, &LayerWidth, &LayerHeight);

    if (argc == 2 && atoi(argv[1]) == 1)
    {
        doSceneLayeout(LayerWidth, LayerHeight);

        printf("Waiting 20 seconds...");
        sleep(20);

        takeScreenShot("/var/ilmScreenShot.bmp");

        changeLayerOpacity();
    }
    else if (argc == 2 && atoi(argv[1]) == 2)
    {
        RemoveSceneLayout();
    }
    else
    {
        createLayers(LayerWidth, LayerHeight);
    }

    shutdown();
    return 0;
}
