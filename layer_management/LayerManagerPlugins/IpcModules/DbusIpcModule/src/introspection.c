/***************************************************************************
 *
 * Copyright 2010-2012 BMW Car IT GmbH
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
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
#include "introspection.h"
#include "DBUSConfiguration.h"
#include <string.h>  // memset
#include <stdio.h>  // sprintf

//=============================================================================
// internal data types
//=============================================================================
struct IntrospectionTable
{
    const char *name;
    const char *signature;
    const char *reply;
};

//=============================================================================
// internal prototypes
//=============================================================================
void generateString();
unsigned int addHeader(char* msgBuffer);
unsigned int openNode(char* msgBuffer, const char* nodename);
unsigned int openInterface(char* msgBuffer, const char* interfacename);
unsigned int openMethod(char* msgBuffer, const char* methodname);
unsigned int addArgument(char* msgBuffer, const char* argname, const char* direction, const char* type);
unsigned int addArrayArgument(char* msgBuffer, const char* argname, const char* direction, const char* type);
unsigned int closeMethod(char* msgBuffer);
unsigned int closeInterface(char* msgBuffer);
unsigned int closeNode(char* msgBuffer);


//=============================================================================
// implementation
//=============================================================================
void generateIntrospectionString(char* msgBuffer)
{
    //LOG_DEBUG("DBUSCommunicator", "Generating introspection data");

    /* Introspection is a special feature of the DBUS IpcModule, which other
     * IpcModules do (most likely) not have.
     * Although the DBUSIpcModule should not know about existing messages
     * at all, this local data table is used to generate a response to
     * the DBUS Introspection call. This table defines the content of the
     * Introspection response.
     */
    struct IntrospectionTable introspectionInterface[] =
    {
        { "ServiceConnect",                   "u",     ""               },
        { "ServiceDisconnect",                "u",     ""               },
        { "Debug",                            "b",     ""               },
        { "ScreenShot",                       "us",    ""               },
        { "ScreenShotOfLayer",                "su",    ""               },
        { "ScreenShotOfSurface",              "su",    ""               },
        { "GetScreenResolution",              "u",     "uu"             },
        { "GetNumberOfHardwareLayers",        "u",     "u"              },
        { "GetScreenIDs",                     "",      "au"             },
        { "ListAllLayerIDS",                  "",      "au"             },
        { "ListAllLayerIDsOnScreen",          "u",     "au"             },
        { "ListAllSurfaceIDS",                "",      "au"             },
        { "ListSurfaceofLayer",               "u",     "au"             },
        { "GetPropertiesOfSurface",           "u",     "duuuuuuuuuuybu" },
        { "GetPropertiesOfLayer",             "u",     "duuuuuuuuuuyb"  },
        { "CreateSurface",                    "uuuu",  "u"              },
        { "CreateSurfaceFromId",              "uuuuu", "u"              },
        { "InitializeSurface",                "",      "u"              },
        { "InitializeSurfaceFromId",          "u",     "u"              },
        { "SetSurfaceNativeContent",          "uuuuu", ""               },
        { "RemoveSurfaceNativeContent",       "u",     ""               },
        { "RemoveSurface",                    "u",     ""               },
        { "CreateLayer",                      "",      "u"              },
        { "CreateLayerFromId",                "u",     "u"              },
        { "CreateLayerWithDimension",         "uu",    "u"              },
        { "CreateLayerFromIdWithDimension",   "uuu",   "u"              },
        { "RemoveLayer",                      "u",     ""               },
        { "AddSurfaceToLayer",                "uu",    ""               },
        { "RemoveSurfaceFromLayer",           "uu",    ""               },
        { "SetSurfaceSourceRegion",           "uuuuu", ""               },
        { "SetLayerSourceRegion",             "uuuuu", ""               },
        { "SetSurfaceDestinationRegion",      "uuuuu", ""               },
        { "SetSurfacePosition",               "uuu",   ""               },
        { "GetSurfacePosition",               "u",     "uu"             },
        { "SetSurfaceDimension",              "uuu",   ""               },
        { "SetLayerDestinationRegion",        "uuuuu", ""               },
        { "SetLayerPosition",                 "uuu",   ""               },
        { "GetLayerPosition",                 "u",     "uu"             },
        { "SetLayerDimension",                "uuu",   ""               },
        { "GetLayerDimension",                "u",     "uu"             },
        { "GetSurfaceDimension",              "u",     "uu"             },
        { "SetSurfaceOpacity",                "ud",    ""               },
        { "SetLayerOpacity",                  "ud",    ""               },
        { "GetSurfaceOpacity",                "u",     "d"              },
        { "GetLayerOpacity",                  "u",     "d"              },
        { "SetSurfaceOrientation",            "uu",    ""               },
        { "GetSurfaceOrientation",            "uu",    ""               },
        { "SetLayerOrientation",              "uu",    ""               },
        { "GetLayerOrientation",              "uu",    ""               },
        { "GetSurfacePixelformat",            "u",     "u"              },
        { "SetSurfaceVisibility",             "ub",    ""               },
        { "SetLayerVisibility",               "ub",    ""               },
        { "GetSurfaceVisibility",             "u",     "b"              },
        { "GetLayerVisibility",               "u",     "b"              },
        { "SetRenderOrderOfLayers",           "auu",   ""               },
        { "SetSurfaceRenderOrderWithinLayer", "uau",   ""               },
        { "GetLayerType",                     "u",     "u"              },
        { "GetLayertypeCapabilities",         "u",     "u"              },
        { "GetLayerCapabilities",             "u",     "u"              },
        { "Exit",                             "",      ""               },
        { "CommitChanges",                    "",      ""               },
        { "CreateShader",                     "ss",    "u"              },
        { "DestroyShader",                    "u",     ""               },
        { "SetShader",                        "uu",    ""               },
        { "SetUniforms",                      "uas",   ""               },
        { "SetKeyboardFocusOn",               "u",     ""               },
        { "GetKeyboardFocusSurfaceId",        "",      "u"              },
        { "UpdateInputEventAcceptanceOn",     "uub",   ""               },
        { "LayerAddNotification",             "u",     ""               },
        { "LayerRemoveNotification",          "u",     ""               },
        { "SurfaceAddNotification",           "u",     ""               },
        { "SurfaceRemoveNotification",        "u",     ""               },
        { "SetOptimizationMode",              "uu",    ""               },
        { "GetOptimizationMode",              "u",     "u"              }
    };

    int introspectionInterfaceCount = sizeof(introspectionInterface) / sizeof(struct IntrospectionTable);

    msgBuffer += addHeader(msgBuffer);
    msgBuffer += openNode(msgBuffer, DBUS_SERVICE_PREFIX);
    msgBuffer += openInterface(msgBuffer, "org.freedesktop.DBus.Introspectable");
    msgBuffer += openMethod(msgBuffer, "Introspect");
    msgBuffer += addArgument(msgBuffer, "data", "out", "s");
    msgBuffer += closeMethod(msgBuffer);
    msgBuffer += closeInterface(msgBuffer);
    msgBuffer += openInterface(msgBuffer, DBUS_SERVICE_PREFIX);

    int index = 0;

    for (index = 0; index < introspectionInterfaceCount; ++index)
    {
        struct IntrospectionTable* method = &introspectionInterface[index];
        const char* methodName = method->name;
        const char* parameter = method->signature;
        const char* returnValue = method->reply;

        int parameterLength = strlen(parameter);
        int returnValueLength = strlen(returnValue);

        msgBuffer += openMethod(msgBuffer, methodName);

        int i = 0;
        for(i = 0; i < parameterLength; ++i)
        {
            switch (parameter[i])
            {
                case 'a':
                    i++;
                    msgBuffer += addArrayArgument(msgBuffer, "", "in", &parameter[i]);
                    break;
                default:
                    msgBuffer += addArgument(msgBuffer, "", "in", &parameter[i]);
                    break;
            }
        }

        for(i = 0; i < returnValueLength; ++i)
        {
            switch (returnValue[i])
            {
                case 'a':
                    i++;
                    msgBuffer += addArrayArgument(msgBuffer, "", "out", &returnValue[i]);
                    break;
                default:
                    msgBuffer += addArgument(msgBuffer, "", "out", &returnValue[i]);
                    break;
            }
        }

        msgBuffer += closeMethod(msgBuffer);
    }

    msgBuffer += closeInterface(msgBuffer);

    msgBuffer += closeNode(msgBuffer);
}

unsigned int addHeader(char* msgBuffer)
{
    return sprintf(msgBuffer, "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">");
}

unsigned int openNode(char* msgBuffer, const char* nodename)
{
    return sprintf(msgBuffer, "<node name=\"%s\">", nodename);
}

unsigned int openInterface(char* msgBuffer, const char* interfacename)
{
    return sprintf(msgBuffer, "<interface name=\"%s\">", interfacename);
}

unsigned int openMethod(char* msgBuffer, const char* methodname)
{
    return sprintf(msgBuffer, "<method name=\"%s\">", methodname);
}

unsigned int addArgument(char* msgBuffer, const char* argname, const char* direction, const char* type)
{
    return sprintf(msgBuffer, "<arg name=\"%s\" direction=\"%s\" type=\"%c\"/>", argname, direction, type[0]);
}

unsigned int addArrayArgument(char* msgBuffer, const char* argname, const char* direction, const char* type)
{
    return sprintf(msgBuffer, "<arg name=\"%s\" direction=\"%s\" type=\"a%c\"/>", argname, direction, type[0]);
}

unsigned int closeMethod(char* msgBuffer)
{
    return sprintf(msgBuffer, "</method>");
}

unsigned int closeInterface(char* msgBuffer)
{
    return sprintf(msgBuffer, "</interface>");
}

unsigned int closeNode(char* msgBuffer)
{
    return sprintf(msgBuffer, "</node>");
}
