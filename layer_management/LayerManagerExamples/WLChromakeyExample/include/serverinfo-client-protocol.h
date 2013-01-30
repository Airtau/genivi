/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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
#ifndef SERVERINFO_CLIENT_PROTOCOL_H
#define SERVERINFO_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-util.h"

struct wl_client;
struct wl_resource;

struct serverinfo;

extern const struct wl_interface serverinfo_interface;

struct serverinfo_listener {
	/*
	 * connection_id - (none)
	 * @connection_id: (none)
	 */
	void (*connection_id)(void *data,
			      struct serverinfo *serverinfo,
			      uint32_t connection_id);
};

static inline int
serverinfo_add_listener(struct serverinfo *serverinfo,
			const struct serverinfo_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) serverinfo,
				     (void (**)(void)) listener, data);
}

#define SERVERINFO_GET_CONNECTION_ID	0

static inline void
serverinfo_set_user_data(struct serverinfo *serverinfo, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) serverinfo, user_data);
}

static inline void *
serverinfo_get_user_data(struct serverinfo *serverinfo)
{
	return wl_proxy_get_user_data((struct wl_proxy *) serverinfo);
}

static inline void
serverinfo_destroy(struct serverinfo *serverinfo)
{
	wl_proxy_destroy((struct wl_proxy *) serverinfo);
}

static inline void
serverinfo_get_connection_id(struct serverinfo *serverinfo)
{
	wl_proxy_marshal((struct wl_proxy *) serverinfo,
			 SERVERINFO_GET_CONNECTION_ID);
}

#ifdef  __cplusplus
}
#endif

#endif /* SERVERINFO_CLIENT_PROTOCOL_H */
