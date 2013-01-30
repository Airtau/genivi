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
 * Unless required by applicable law or agreed in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either inputess or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/
#include "ilm_client.h"
#include "LMControl.h"
#include "Expression.h"
#include "ExpressionInterpreter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <signal.h> // signal
#include <unistd.h> // alarm

using namespace std;


#define COMMAND(text) COMMAND2(__COUNTER__,text)

#define COMMAND2(x,y) COMMAND3(x,y)

#define COMMAND3(funcNumber, text) \
    void func_ ## funcNumber(Expression* input); \
    static const bool reg_ ## funcNumber = \
        ExpressionInterpreter::addExpression(func_ ## funcNumber, text); \
    void func_ ## funcNumber(Expression* input)



//=============================================================================
COMMAND("help")
//=============================================================================
{
    (void)input;
    cout << "help: supported commands:\n\n";
    ExpressionInterpreter::printExpressionList();
    cout << "\n";
}

//=============================================================================
COMMAND("tree")
//=============================================================================
{
    (void)input;
    cout << "help: supported commands:\n\n";
    ExpressionInterpreter::printExpressionTree();
    cout << "\n";
}

//=============================================================================
COMMAND("get screens")
//=============================================================================
{
    (void)input;
    unsigned int count = 0;
    unsigned int* array = NULL;
    ilm_getScreenIDs(&count, &array);
    printArray("Screen", array, count);
}

//=============================================================================
COMMAND("get layers")
//=============================================================================
{
    (void)input;
    int count = 0;
    unsigned int* array = NULL;
    ilm_getLayerIDs(&count, &array);
    printArray("Layer", array, count);
}

//=============================================================================
COMMAND("get surfaces")
//=============================================================================
{
    (void)input;
    int count = 0;
    unsigned int* array = NULL;
    ilm_getSurfaceIDs(&count, &array);
    printArray("Surface", array, count);
}

//=============================================================================
COMMAND("get screen <screenid>")
//=============================================================================
{
    printScreenProperties(input->getUint("screenid"));
}

//=============================================================================
COMMAND("get layer <layerid>")
//=============================================================================
{
    printLayerProperties(input->getUint("layerid"));
}

//=============================================================================
COMMAND("get surface <surfaceid>")
//=============================================================================
{
    printSurfaceProperties(input->getUint("surfaceid"));
}

//=============================================================================
COMMAND("dump screen <screenid> to <file>")
//=============================================================================
{
    ilm_takeScreenshot(input->getUint("screenid"),
                       input->getString("file").c_str());
}

//=============================================================================
COMMAND("dump layer <layerid> to <file>")
//=============================================================================
{
    ilm_takeLayerScreenshot(input->getString("file").c_str(),
                            input->getUint("layerid"));
}

//=============================================================================
COMMAND("dump surface <surfaceid> to <file>")
//=============================================================================
{
    ilm_takeSurfaceScreenshot(input->getString("file").c_str(),
                              input->getUint("surfaceid"));
}

//=============================================================================
COMMAND("set layer <layerid> source region <x> <y> <w> <h>")
//=============================================================================
{
    ilm_layerSetSourceRectangle(input->getUint("layerid"),
                                input->getUint("x"),
                                input->getUint("y"),
                                input->getUint("w"),
                                input->getUint("h"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set surface <surfaceid> source region <x> <y> <w> <h>")
//=============================================================================
{
    ilm_surfaceSetSourceRectangle(input->getUint("surfaceid"),
                                  input->getUint("x"),
                                  input->getUint("y"),
                                  input->getUint("w"),
                                  input->getUint("h"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> destination region <x> <y> <w> <h>")
//=============================================================================
{
    ilm_layerSetDestinationRectangle(input->getUint("layerid"),
                                     input->getUint("x"),
                                     input->getUint("y"),
                                     input->getUint("w"),
                                     input->getUint("h"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set surface <surfaceid> destination region <x> <y> <w> <h>")
//=============================================================================
{
    ilm_surfaceSetDestinationRectangle(input->getUint("surfaceid"),
                                       input->getUint("x"),
                                       input->getUint("y"),
                                       input->getUint("w"),
                                       input->getUint("h"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> opacity <opacity>")
//=============================================================================
{
    ilm_layerSetOpacity(input->getUint("layerid"),
                        input->getDouble("opacity"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set surface <surfaceid> opacity <opacity>")
//=============================================================================
{
    ilm_surfaceSetOpacity(input->getUint("surfaceid"),
                          input->getDouble("opacity"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> visibility <visibility>")
//=============================================================================
{
    ilm_layerSetVisibility(input->getUint("layerid"),
                           input->getBool("visibility"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set surface <surfaceid> visibility <visibility>")
//=============================================================================
{
    ilm_surfaceSetVisibility(input->getUint("surfaceid"),
                             input->getBool("visibility"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> orientation <orientation>")
//=============================================================================
{
    ilm_layerSetOrientation(input->getUint("layerid"),
            (ilmOrientation)input->getInt("orientation"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set surface <surfaceid> orientation <orientation>")
//=============================================================================
{
    ilm_surfaceSetOrientation(input->getUint("surfaceid"),
              (ilmOrientation)input->getInt("orientation"));
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set screen <screenid> render order <layeridarray>")
//=============================================================================
{
    unsigned int count = 0;
    unsigned int* array = NULL;
    unsigned int screenid = input->getUint("screenid");
    input->getUintArray("layeridarray", &array, &count);
    ilm_displaySetRenderOrder(screenid, array, count);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set screen <screenid> render order")
//=============================================================================
{
    unsigned int screenid = input->getUint("screenid");
    ilm_displaySetRenderOrder(screenid, NULL, 0);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> render order <surfaceidarray>")
//=============================================================================
{
    unsigned int count = 0;
    unsigned int* array = NULL;
    unsigned int layerid = input->getUint("layerid");
    input->getUintArray("surfaceidarray", &array, &count);
    ilm_layerSetRenderOrder(layerid, array, count);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> render order")
//=============================================================================
{
    unsigned int layerid = input->getUint("layerid");
    ilm_layerSetRenderOrder(layerid, NULL, 0);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> width <width>")
//=============================================================================
{
    unsigned int dimension[2];
    unsigned int layerid = input->getUint("layerid");
    ilm_layerGetDimension(layerid, dimension);
    dimension[0] = input->getUint("width");
    ilm_layerSetDimension(layerid, dimension);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set surface <surfaceid> width <width>")
//=============================================================================
{
    unsigned int dimension[2];
    unsigned int surfaceid = input->getUint("surfaceid");
    ilm_surfaceGetDimension(surfaceid, dimension);
    dimension[0] = input->getUint("width");
    ilm_surfaceSetDimension(surfaceid, dimension);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> height <height>")
//=============================================================================
{
    unsigned int dimension[2];
    unsigned int layerid = input->getUint("layerid");
    ilm_layerGetDimension(layerid, dimension);
    dimension[1] = input->getUint("height");
    ilm_layerSetDimension(layerid, dimension);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set surface <surfaceid> height <height>")
//=============================================================================
{
    unsigned int dimension[2];
    unsigned int surfaceid = input->getUint("surfaceid");
    ilm_surfaceGetDimension(surfaceid, dimension);
    dimension[1] = input->getUint("height");
    ilm_surfaceSetDimension(surfaceid, dimension);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> position <x> <y>")
//=============================================================================
{
    unsigned int dimension[2];
    unsigned int layerid = input->getUint("layerid");
    dimension[0] = input->getUint("x");
    dimension[1] = input->getUint("y");
    ilm_layerSetPosition(layerid, dimension);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set surface <surfaceid> position <x> <y>")
//=============================================================================
{
    unsigned int dimension[2];
    unsigned int surfaceid = input->getUint("surfaceid");
    dimension[0] = input->getUint("x");
    dimension[1] = input->getUint("y");
    ilm_surfaceSetPosition(surfaceid, dimension);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("create layer <layerid>")
//=============================================================================
{
    unsigned int layerid = input->getUint("layerid");
    ilm_layerCreate(&layerid);
}

//=============================================================================
COMMAND("create layer <layerid> <width> <height>")
//=============================================================================
{
    unsigned int layerid = input->getUint("layerid");
    unsigned int width = input->getUint("width");
    unsigned int height = input->getUint("height");
    ilm_layerCreateWithDimension(&layerid, width, height);
}

//=============================================================================
COMMAND("create surface <surfaceid> <nativehandle> <width> <height> <pixelformat>")
//=============================================================================
{
    unsigned int surfaceid = input->getUint("surfaceid");
    unsigned int nativeHandle = input->getUint("nativehandle");
    unsigned int width = input->getUint("width");
    unsigned int height = input->getUint("height");
    e_ilmPixelFormat pixelformat = (e_ilmPixelFormat)input->getUint("pixelformat");
    ilm_surfaceCreate(nativeHandle, width, height, pixelformat, &surfaceid);
}

//=============================================================================
COMMAND("destroy layer <layerid>")
//=============================================================================
{
    unsigned int layerid = input->getUint("layerid");
    ilm_layerRemove(layerid);
}

//=============================================================================
COMMAND("destroy surface <surfaceid>")
//=============================================================================
{
    unsigned int surfaceid = input->getUint("surfaceid");
    ilm_surfaceRemove(surfaceid);
}

//=============================================================================
COMMAND("get scene")
//=============================================================================
{
    printScene();
}

//=============================================================================
COMMAND("get communicator performance")
//=============================================================================
{
   getCommunicatorPerformance();
}

//=============================================================================
COMMAND("set surface <surfaceid> keyboard focus")
//=============================================================================
{
    t_ilm_surface surface = input->getUint("surfaceid");

    setSurfaceKeyboardFocus(surface);
}

//=============================================================================
COMMAND("get keyboard focus")
//=============================================================================
{
    getKeyboardFocus();
}

//=============================================================================
COMMAND("set surface <surfaceid> accept <acceptance> input events from devices <kbd:pointer:touch>")
//=============================================================================
{
    t_ilm_surface surfaceId = input->getUint("surfaceid");
    t_ilm_bool acceptance = input->getBool("acceptance");
    string kbdPointerTouch = input->getString("kbd:pointer:touch");

    setSurfaceAcceptsInput(surfaceId, kbdPointerTouch, acceptance);
}

//=============================================================================
COMMAND("set surface <surfaceid> chromakey <red> <green> <blue>")
//=============================================================================
{
    t_ilm_surface surface = input->getUint("surfaceid");
    t_ilm_int color[3] =
    {
        input->getInt("red"),
        input->getInt("green"),
        input->getInt("blue")
    };
    ilm_surfaceSetChromaKey(surface, color);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set surface <surfaceid> chromakey disabled")
//=============================================================================
{
    t_ilm_surface surface = input->getUint("surfaceid");
    ilm_surfaceSetChromaKey(surface, NULL);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("set layer <layerid> chromakey <red> <green> <blue>")
//=============================================================================
{
    t_ilm_surface surface = input->getUint("layerid");
    t_ilm_int color[3] =
    {
        input->getInt("red"),
        input->getInt("green"),
        input->getInt("blue")
    };

    ilm_layerSetChromaKey(surface, color);
    ilm_commitChanges();
}

//=============================================================================
COMMAND("test notification layer <layerid>")
//=============================================================================
{
    unsigned int layerid = input->getUint("layerid");

    testNotificationLayer(layerid);
}

//=============================================================================
COMMAND("watch layer <layeridarray>")
//=============================================================================
{
    unsigned int* layerids = NULL;
    unsigned int layeridCount;
    input->getUintArray("layeridarray", &layerids, &layeridCount);

    watchLayer(layerids, layeridCount);
}

//=============================================================================
COMMAND("watch surface <surfaceidarray>")
//=============================================================================
{
    unsigned int* surfaceids = NULL;
    unsigned int surfaceidCount;
    input->getUintArray("surfaceidarray", &surfaceids, &surfaceidCount);

    watchSurface(surfaceids, surfaceidCount);
}

//=============================================================================
COMMAND("set optimization <id> mode <mode>")
//=============================================================================
{
    t_ilm_uint id = input->getUint("id");
    t_ilm_uint mode = input->getUint("mode");
    setOptimization(id, mode);
}

//=============================================================================
COMMAND("get optimization <id>")
//=============================================================================
{
    t_ilm_uint id = input->getUint("id");
    getOptimization(id);
}

//=============================================================================
COMMAND("analyze surface <surfaceid>")
//=============================================================================
{
    t_ilm_surface targetSurfaceId = (t_ilm_uint) input->getUint("surfaceid");
    analyzeSurface(targetSurfaceId);
}

//=============================================================================
COMMAND("scatter")
//=============================================================================
{
    scatter();
}

//=============================================================================
COMMAND("scatter all")
//=============================================================================
{
    scatterAll();
}

//=============================================================================
COMMAND("demo <animation_mode>")
//=============================================================================
{
    int mode = (int) input->getInt("animation_mode");
    demo(mode);
}

//=============================================================================
COMMAND("export scene to <filename>")
//=============================================================================
{
    string filename = (string) input->getString("filename");
    exportSceneToFile(filename);
}

//=============================================================================
COMMAND("export xtext to <filename> <grammar> <url>")
//=============================================================================
{
    string filename = (string) input->getString("filename");
    string grammar = (string) input->getString("grammar");
    string url = (string) input->getString("url");
    exportXtext(filename, grammar, url);
}

//=============================================================================
COMMAND("import scene from <filename>")
//=============================================================================
{
    string filename = (string) input->getString("filename");
    importSceneFromFile(filename);
}
