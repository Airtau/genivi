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
#include "IpcModule.h"
#include "common.h"
#include "callbacks.h"
#include "DBUSConfiguration.h"
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset


//=============================================================================
// prototypes
//=============================================================================
t_ilm_bool initService();
t_ilm_bool destroyClient();
t_ilm_bool destroyService();


//=============================================================================
// setup
//=============================================================================
t_ilm_bool initServiceMode()
{
    memset(&gDbus, 0, sizeof(gDbus));

    gDbus.initialized = ILM_FALSE;
    gDbus.isClient = ILM_FALSE;

    pthread_mutex_init(&gDbus.mutex, NULL);

    dbus_error_init(&gDbus.error);

    char* useSessionBus = getenv("LM_USE_SESSION_BUS");
    if (useSessionBus && strcmp(useSessionBus, "enable") == 0 )
    {
        gDbus.type = DBUS_BUS_SESSION;
        printf("DbusIpcmodule: using session bus\n");
    }
    else
    {
        gDbus.type = DBUS_BUS_SYSTEM;
        printf("DbusIpcmodule: using system bus\n");
    }

    gDbus.connection = dbus_bus_get(gDbus.type, &gDbus.error);

    if (dbus_error_is_set(&gDbus.error) || !gDbus.connection)
    {
        printf("DbusIpcmodule: Connection error\n");
        dbus_error_free(&gDbus.error);
        exit(1);
    }

    gDbus.initialized = ILM_TRUE;

    if (!gDbus.isClient)
    {
        printf("DbusIpcmodule: registering dbus address %s\n", DBUS_SERVICE_PREFIX);
        int ret = dbus_bus_request_name(gDbus.connection, DBUS_SERVICE_PREFIX, DBUS_NAME_FLAG_REPLACE_EXISTING, &gDbus.error);

        if (dbus_error_is_set(&gDbus.error))
        {
            printf("DbusIpcmodule: Name Error %s\n", gDbus.error.message);
            dbus_error_free(&gDbus.error);
            gDbus.initialized = ILM_FALSE;
        }

        if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
        {
            printf("DbusIpcmodule: Not Primary Owner of %s (error %d)\n", DBUS_SERVICE_PREFIX, ret);
            gDbus.initialized = ILM_FALSE;
        }

        const char* rule = "type='signal',"
                           "sender='"DBUS_INTERFACE_DBUS"',"
                           "interface='"DBUS_INTERFACE_DBUS"',"
                           "member='NameOwnerChanged'";

        dbus_bus_add_match(gDbus.connection, rule, &gDbus.error);
        if (dbus_error_is_set(&gDbus.error))
        {
            printf("DbusIpcModule: Could not add client watch, error: %s\n", gDbus.error.message);
            dbus_error_free(&gDbus.error);
        }
    }

/*
    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterLogging,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }
*/
    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterNameOwnerChanged,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }

    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterIntrospection,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }

    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterNameAcquired,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }

    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterLayerManagerCommands,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }

    if(!dbus_connection_set_watch_functions(gDbus.connection,
                                            addWatch,
                                            removeWatch,
                                            toggleWatch,
                                            0,
                                            NULL))
    {
        printf("Couldn't set up watch functions\n");
        exit(1);
    }

    return gDbus.initialized;
}

t_ilm_bool initClientMode()
{
    memset(&gDbus, 0, sizeof(gDbus));

    gDbus.initialized = ILM_FALSE;
    gDbus.isClient = ILM_TRUE;

    pthread_mutex_init(&gDbus.mutex, NULL);

    dbus_error_init(&gDbus.error);

    char* useSessionBus = getenv("LM_USE_SESSION_BUS");
    if (useSessionBus && strcmp(useSessionBus, "enable") == 0 )
    {
        gDbus.type = DBUS_BUS_SESSION;
        printf("DbusIpcmodule: using session bus\n");
    }
    else
    {
        gDbus.type = DBUS_BUS_SYSTEM;
        printf("DbusIpcmodule: using system bus\n");
    }

    dbus_threads_init_default();

    gDbus.connection = dbus_bus_get_private(gDbus.type, &gDbus.error);

    if (dbus_error_is_set(&gDbus.error) || !gDbus.connection)
    {
        printf("DbusIpcmodule: Connection error\n");
        dbus_error_free(&gDbus.error);
        exit(1);
    }

    gDbus.initialized = ILM_TRUE;
/*
    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterLogging,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }
*/
    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterNameAcquired,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }

    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterLayerManagerCommands,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }

    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterLayerManagerErrors,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }

    if (!dbus_connection_add_filter(gDbus.connection,
                                    filterLayerManagerNotifications,
                                    NULL,
                                    NULL))
    {
        printf("Couldn't set up filter functions\n");
        exit(1);
    }

    if(!dbus_connection_set_watch_functions(gDbus.connection,
                                            addWatch,
                                            removeWatch,
                                            toggleWatch,
                                            0,
                                            NULL))
    {
        printf("Couldn't set up watch functions\n");
        exit(1);
    }

    return gDbus.initialized;
}

t_ilm_bool destroy()
{
    if (gDbus.initialized)
    {
        gDbus.initialized = ILM_FALSE;

        dbus_connection_set_watch_functions(gDbus.connection, NULL, NULL, NULL, 0, NULL);
        dbus_connection_remove_filter(gDbus.connection, filterNameAcquired, NULL);
        dbus_connection_remove_filter(gDbus.connection, filterLayerManagerCommands, NULL);

        if (gDbus.isClient)
        {
            dbus_connection_remove_filter(gDbus.connection, filterLayerManagerErrors, NULL);
            dbus_connection_remove_filter(gDbus.connection, filterLayerManagerNotifications, NULL);
            destroyClient();
        }
        else
        {
            dbus_connection_remove_filter(gDbus.connection, filterNameOwnerChanged, NULL);
            dbus_connection_remove_filter(gDbus.connection, filterIntrospection, NULL);
            destroyService();
        }

        pthread_mutex_destroy(&gDbus.mutex);
    }

    return gDbus.initialized;
}


//=============================================================================
// service specific
//=============================================================================
t_ilm_bool destroyService()
{
    DBusError err;
    dbus_error_init(&err);

    t_ilm_bool errorset = dbus_error_is_set(&err);
    if (errorset)
    {
        printf("DbusIpcmodule: there was an dbus error\n");
    }

    dbus_bus_name_has_owner(gDbus.connection, DBUS_SERVICE_PREFIX, &err);
    errorset = dbus_error_is_set(&err);
    
    if (errorset)
    {
        printf("DbusIpcmodule: there was an dbus error\n");
    }

    dbus_error_init(&err);
    
    const char* rule = "type='signal',"
                       "sender='"DBUS_INTERFACE_DBUS"',"
                       "interface='"DBUS_INTERFACE_DBUS"',"
                       "member='NameOwnerChanged'";

    dbus_bus_remove_match(gDbus.connection, rule, &err);

    errorset = dbus_error_is_set(&err);
    
    if (errorset)
    {
        printf("DbusIpcmodule: there was an dbus error\n");
    }
    
    dbus_error_init(&err);
    dbus_bus_release_name(gDbus.connection, DBUS_SERVICE_PREFIX, &err);

    return ILM_TRUE;
}


//=============================================================================
// client specific
//=============================================================================

t_ilm_bool destroyClient()
{
    // private dbus connection must be closed
    if (dbus_connection_get_is_connected(gDbus.connection))
    {
        dbus_connection_close(gDbus.connection);
    }
    dbus_connection_unref(gDbus.connection);
    dbus_error_free(&gDbus.error);
    return ILM_TRUE;
}
