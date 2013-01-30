/***************************************************************************
*
* Copyright 2010, 2011 BMW Car IT GmbH 
* Copyright (C) 2011 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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
#ifndef _EGL_HELPER_H_
#define _EGL_HELPER_H_

#include "wayland-client.h"
#include "wayland-egl.h"
#include "ilm_client.h"
#include <EGL/egl.h>

t_ilm_uint GetTickCount();
t_ilm_bool createWLContext(t_ilm_int width, t_ilm_int height);
t_ilm_bool createEGLContext(t_ilm_int width, t_ilm_int height);
void destroyEglContext();
void destroyWLContext();
void swapBuffers();

#endif /* _EGL_HELPER_H_ */
