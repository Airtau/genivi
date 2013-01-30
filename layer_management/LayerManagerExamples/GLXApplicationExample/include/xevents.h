/***************************************************************************
 *
 * Copyright 2011 Valeo
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

#ifndef _XEVENTS_H_
#define _XEVENTS_H_

#include "config.h" 
#include <X11/Xlib.h>
#include "ilm_client.h"

void xeventInitialiaze(Display* dpy, t_ilm_surface surfaceid, t_ilm_int x, t_ilm_int y, t_ilm_int width, t_ilm_int height);
void parseXEventsNonBlocking();


#endif // _XEVENTS_H_

