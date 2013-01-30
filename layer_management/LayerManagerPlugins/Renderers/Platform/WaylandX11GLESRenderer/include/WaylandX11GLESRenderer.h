/***************************************************************************
 * 
 * Copyright 2010-2012 BMW Car IT GmbH
 * Copyright (C) 2011 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#ifndef _WAYLANDX11GLESRENDERER_H_
#define _WAYLANDX11GLESRENDERER_H_

#include "WaylandGLESRenderer.h"

class WaylandX11GLESRenderer : public WaylandGLESRenderer
{
public:
    WaylandX11GLESRenderer(ICommandExecutor& executor, Configuration& config);

    // from PluginBase
    virtual t_ilm_const_string pluginGetName() const;

    // from WaylandGLESRenderer
    virtual WaylandBaseWindowSystem* getWindowSystem(const char* displayname);
    virtual GLESGraphicsystem* getGraphicSystem(PfnShaderProgramCreator shaderProgram);
};

#endif /* _WAYLANDX11GLESRENDERER_H_*/
