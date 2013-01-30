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

#include <cstring>

#include <iostream>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;


#include <iomanip>
using std::dec;
using std::hex;


#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>


bool gBenchmark_running;

void benchmarkSigHandler(int sig)
{
    gBenchmark_running = false;
}

void getCommunicatorPerformance()
{
    int runs = 0;
    int runtimeInSec = 5;
    unsigned int hwLayerCnt = 0;
    cout << "running performance test for " << runtimeInSec << " seconds... ";
    flush(cout);

    signal(SIGALRM, benchmarkSigHandler);

    gBenchmark_running = true;

    alarm(runtimeInSec);

    while (gBenchmark_running)
    {
        ilm_getNumberOfHardwareLayers(0, &hwLayerCnt);
        ++runs;
    }

    signal(SIGALRM, SIG_DFL);

    cout << (runs/runtimeInSec) << " transactions/second\n";
}

void setSurfaceKeyboardFocus(t_ilm_surface surface)
{
    if (ilm_SetKeyboardFocusOn(surface) != ILM_SUCCESS)
    {
        cerr << "Error during communication" << endl;
    }
}

void getKeyboardFocus()
{
    t_ilm_surface surfaceId;
    if (ilm_GetKeyboardFocusSurfaceId(&surfaceId) == ILM_SUCCESS)
    {
        cout << "keyboardFocusSurfaceId == " << surfaceId << endl;
    }
    else
    {
        cerr << "Error during communication" << endl;
    }
}

void setSurfaceAcceptsInput(t_ilm_surface surfaceId, string kbdPointerTouch, t_ilm_bool acceptance)
{
    char* str;
    char* tok;

    ilmInputDevice devices = (ilmInputDevice)0;

    str = new char [kbdPointerTouch.size()+1];
    strcpy (str, kbdPointerTouch.c_str());
    tok = strtok(str, ":");
    while (tok != NULL)
    {
        if (!strcmp(tok, "kbd"))
        {
            devices |= ILM_INPUT_DEVICE_KEYBOARD;
        }
        else if (!strcmp(tok, "pointer"))
        {
            devices |= ILM_INPUT_DEVICE_POINTER;
        }
        else if (!strcmp(tok, "touch"))
        {
            devices |= ILM_INPUT_DEVICE_TOUCH;
        }
        else
        {
            cerr << "Unknown devices specified." << endl;
        }

        tok = strtok(NULL, ":");
    }

    ilm_UpdateInputEventAcceptanceOn(surfaceId, devices, acceptance);
    ilm_commitChanges();

    delete[] str;
}

void layerNotificationCallback(t_ilm_layer layer, struct ilmLayerProperties* properties, t_ilm_notification_mask mask)
{
    cout << "\nNotification: layer " << layer << " updated properties:\n";

    if (ILM_NOTIFICATION_VISIBILITY & mask)
    {
        cout << "\tvisibility = " << properties->visibility << "\n";
    }

    if (ILM_NOTIFICATION_OPACITY & mask)
    {
        cout << "\topacity = " << properties->opacity << "\n";
    }

    if (ILM_NOTIFICATION_ORIENTATION & mask)
    {
        cout << "\torientation = " << properties->orientation << "\n";
    }

    if (ILM_NOTIFICATION_SOURCE_RECT & mask)
    {
        cout << "\tsource rect = x:" << properties->sourceX
             << ", y:" << properties->sourceY
             << ", width:" << properties->sourceWidth
             << ", height:" << properties->sourceHeight
             << "\n";
    }

    if (ILM_NOTIFICATION_DEST_RECT & mask)
    {
        cout << "\tdest rect = x:" << properties->destX
             << ", y:" << properties->destY
             << ", width:" << properties->destWidth
             << ", height:" << properties->destHeight
             << "\n";
    }
}

void testNotificationLayer(t_ilm_layer layerid)
{

    ilmErrorTypes ret;
    cout << "Setup notification for layer " << layerid << " \n";
    ret = ilm_layerAddNotification(layerid, layerNotificationCallback);

    if (ret != ILM_SUCCESS)
    {
        cerr << "ilm_layerAddNotification returned error " << ret << "\n";
    }

    for  (int i = 0; i < 2; ++i)
    {
        usleep(100 * 1000);
        cout << "Set layer 1000 visbility to FALSE\n";
        ilm_layerSetVisibility(layerid, ILM_FALSE);
        ilm_commitChanges();

        usleep(100 * 1000);
        cout << "Set layer 1000 visbility to TRUE\n";
        ilm_layerSetVisibility(layerid, ILM_TRUE);

        cout << "Set layer 1000 opacity to 0.3\n";
        ilm_layerSetOpacity(layerid, 0.3);
        ilm_commitChanges();

        usleep(100 * 1000);
        cout << "Set layer 1000 opacity to 1.0\n";
        ilm_layerSetOpacity(layerid, 1.0);
        ilm_commitChanges();
    }

    ilm_commitChanges(); // make sure, app lives long enough to receive last notification
}

void watchLayer(unsigned int* layerids, unsigned int layeridCount)
{
    for (unsigned int i = 0; i < layeridCount; ++i)
    {
        unsigned int layerid = layerids[i];
        cout << "Setup notification for layer " << layerid << "\n";
        ilmErrorTypes ret = ilm_layerAddNotification(layerid, layerNotificationCallback);

        if (ret != ILM_SUCCESS)
        {
            cerr << "ilm_layerAddNotification(" << layerid << ") returned error " << ret << "\n";
            return;
        }
    }

    cout << "Waiting for notifications...\n";
    int block;
    cin >> block;

    for (unsigned int i = 0; i < layeridCount; ++i)
    {
        unsigned int layerid = layerids[i];
        cout << "Removing notification for layer " << layerid << "\n";
        ilm_layerRemoveNotification(layerid);
    }

    if (layerids)
    {
        delete[] layerids;
    }
}

void surfaceNotificationCallback(t_ilm_layer surface, struct ilmSurfaceProperties* properties, t_ilm_notification_mask mask)
{
    cout << "\nNotification: surface " << surface << " updated properties:\n";

    if (ILM_NOTIFICATION_VISIBILITY & mask)
    {
        cout << "\tvisibility = " << properties->visibility << "\n";
    }

    if (ILM_NOTIFICATION_OPACITY & mask)
    {
        cout << "\topacity = " << properties->opacity << "\n";
    }

    if (ILM_NOTIFICATION_ORIENTATION & mask)
    {
        cout << "\torientation = " << properties->orientation << "\n";
    }

    if (ILM_NOTIFICATION_SOURCE_RECT & mask)
    {
        cout << "\tsource rect = x:" << properties->sourceX
             << ", y:" << properties->sourceY
             << ", width:" << properties->sourceWidth
             << ", height:" << properties->sourceHeight
             << "\n";
    }

    if (ILM_NOTIFICATION_DEST_RECT & mask)
    {
        cout << "\tdest rect = x:" << properties->destX
             << ", y:" << properties->destY
             << ", width:" << properties->destWidth
             << ", height:" << properties->destHeight
             << "\n";
    }
}

void watchSurface(unsigned int* surfaceids, unsigned int surfaceidCount)
{
    for (unsigned int i = 0; i < surfaceidCount; ++i)
    {
        unsigned int surfaceid = surfaceids[i];
        cout << "Setup notification for surface " << surfaceid << "\n";
        ilmErrorTypes ret = ilm_surfaceAddNotification(surfaceid, surfaceNotificationCallback);

        if (ret != ILM_SUCCESS)
        {
            cerr << "ilm_surfaceAddNotification(" << surfaceid << ") returned error " << ret << "\n";
            return;
        }
    }

    cout << "Waiting for notifications...\n";
    int block;
    cin >> block;

    for (unsigned int i = 0; i < surfaceidCount; ++i)
    {
        unsigned int surfaceid = surfaceids[i];
        cout << "Removing notification for surface " << surfaceid << "\n";
        ilm_surfaceRemoveNotification(surfaceid);
    }

    if (surfaceids)
    {
        delete[] surfaceids;
    }
}

void setOptimization(t_ilm_uint id, t_ilm_uint mode)
{
    if (ilm_SetOptimizationMode((ilmOptimization)id,
                                    (ilmOptimizationMode)mode) != ILM_SUCCESS)
    {
        cerr << "Error during communication" << endl;
    }

    ilm_commitChanges();
}

void getOptimization(t_ilm_uint id)
{
    ilmOptimization optimizationId = (ilmOptimization)id;
    ilmOptimizationMode optimizationMode;
    if (ilm_GetOptimizationMode(optimizationId, &optimizationMode) == ILM_SUCCESS)
    {
        switch ( optimizationId )
        {
        case ILM_OPT_MULTITEXTURE :
            cout << "Optimization " << (int)optimizationId << " (Multitexture Optimization)" << endl;
            break;

        case ILM_OPT_SKIP_CLEAR :
            cout << "Optimization " << (int)optimizationId << " (Skip Clear)" << endl;
            break;
        default:
            cout << "Optimization " << "unknown" << endl;
            break;
        }

        switch (optimizationMode)
        {
        case ILM_OPT_MODE_FORCE_OFF :
            cout << "Mode " << (int)optimizationMode << " (forced off)" << endl;
            break;

        case ILM_OPT_MODE_FORCE_ON :
            cout << "Mode " << (int)optimizationMode << " (forced on)" << endl;
            break;
        case ILM_OPT_MODE_HEURISTIC :
            cout << "Mode " << (int)optimizationMode << " (Heuristic / Render choose the optimization)" << endl;
            break;
        case ILM_OPT_MODE_TOGGLE :
            cout << "Mode " << (int)optimizationMode << " (Toggle on/and off rapidly for debugging)" << endl;
            break;

        default:
            cout <<"Mode " << "unknown" << endl ;
            break;
        }
    }
    else
    {
        cerr << "Error during communication" << endl;
    }

    ilm_commitChanges();
}
