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
#ifndef _GLES2APPLICATION_H_
#define _GLES2APPLICATION_H_

#include "ilm_client.h"
#include <GLES2/gl2.h>

t_ilm_bool initGlApplication();
t_ilm_bool initShader();
t_ilm_bool destroyShader();
t_ilm_bool initVertexBuffer();

void attachVertexBuffer();
void detachVertexBuffer();
void destroyVertexBuffer();

void draw(t_ilm_uint animTime);

void destroyGlApplication();

#endif /* _GLES2APPLICATION_H_ */
