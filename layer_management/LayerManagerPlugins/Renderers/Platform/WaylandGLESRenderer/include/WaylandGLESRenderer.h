/***************************************************************************
*
* Copyright 2010, 2011 BMW Car IT GmbH 
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

#ifndef _WAYLANDGLESRENDERER_H_
#define _WAYLANDGLESRENDERER_H_

#include "BaseRenderer.h"
#include "LayerList.h"
#include "GraphicSystems/GLESGraphicSystem.h"
#include "WindowSystems/WaylandBaseWindowSystem.h"

class ITextureBinder;

class WaylandGLESRenderer : public BaseRenderer
{
public:
    WaylandGLESRenderer(ICommandExecutor& executor, Configuration& config);
    bool start(int, int, const char*);
    void stop();
    void doScreenShot(std::string fileToSave);
    void doScreenShotOfLayer(std::string fileToSave, uint id);
    void doScreenShotOfSurface(std::string fileToSave, uint id, uint layer_id);
    uint getNumberOfHardwareLayers(uint screenID);
    uint* getScreenResolution(uint screenID);
    uint* getScreenIDs(uint* length);
    void signalWindowSystemRedraw();
    void forceCompositionWindowSystem();
    Shader* createShader(const string* vertexName, const string* fragmentName);
    virtual bool setOptimizationMode(OptimizationType id, OptimizationModeType mode);
    virtual bool getOptimizationMode(OptimizationType id, OptimizationModeType *mode);

    // from PluginBase
    virtual HealthCondition pluginGetHealth();

    // implement in inheriting classes
    virtual WaylandBaseWindowSystem* getWindowSystem(const char* displayname) = 0;
    virtual GLESGraphicsystem* getGraphicSystem(PfnShaderProgramCreator shaderProgram) = 0;

protected:
    WaylandBaseWindowSystem* m_pWindowSystem;
    GLESGraphicsystem* m_pGraphicSystem;
    uint m_width;
    uint m_height;
    ITextureBinder* m_binder;
};

#endif /* _WAYLANDGLESRENDERER_H_*/
