/***************************************************************************
 *
 * Copyright 2010-2012 BMW Car IT GmbH
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

#ifndef _DBUSCONFIGURATION_H_
#define _DBUSCONFIGURATION_H_

//=============================================================================
// DBUS configuration
//=============================================================================
#define DBUS_SERVICE_PREFIX             "org.genivi.layermanagementservice\0"
//#define DBUS_SERVICE_OBJECT_PATH        "/org/genivi/layermanagementservice\0"
#define DBUS_SERVICE_ERROR              "org.genivi.layermanagementservice.error"
#define DBUS_RECEIVE_TIMEOUT_IN_MS      500

#define ILM_SERVICE_NAME                "org.genivi.layermanagementservice"
//#define ILM_PATH_COMPOSITE_SERVICE      "/org/genivi/layermanagementservice"
#define ILM_PATH_COMPOSITE_SERVICE      "/"
#define ILM_INTERFACE_COMPOSITE_SERVICE "org.genivi.layermanagementservice"
#define ILM_INTERFACE_COMPOSITE_CLIENT  "org.genivi.layermanagementclient"


#endif // _DBUSCONFIGURATION_H_
