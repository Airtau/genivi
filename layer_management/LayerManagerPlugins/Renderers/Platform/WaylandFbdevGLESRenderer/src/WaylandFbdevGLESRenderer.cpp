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

#include "WaylandFbdevGLESRenderer.h"
#include "WindowSystems/WaylandFbdevWindowSystem.h"
#include "Configuration.h"


WaylandFbdevGLESRenderer::WaylandFbdevGLESRenderer(ICommandExecutor& executor, Configuration& config)
: WaylandGLESRenderer(executor, config)
{
}

t_ilm_const_string WaylandFbdevGLESRenderer::pluginGetName() const
{
    return "WaylandFbdevGLESRenderer";
}

WaylandBaseWindowSystem* WaylandFbdevGLESRenderer::getWindowSystem(const char* displayname)
{
    return new WaylandFbdevWindowSystem(displayname, m_width, m_height, m_pScene, m_pInputManager);
}

GLESGraphicsystem* WaylandFbdevGLESRenderer::getGraphicSystem(PfnShaderProgramCreator shaderProgram)
{
    return new GLESGraphicsystem(m_width, m_height, shaderProgram);
}

DECLARE_LAYERMANAGEMENT_PLUGIN(WaylandFbdevGLESRenderer)