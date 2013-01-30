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

#ifndef _BASERENDERER_H_
#define _BASERENDERER_H_

#include "PluginBase.h"
#include "LayerType.h"
#include "Scene.h"
#include "IRenderer.h"
#include "WindowSystems/BaseWindowSystem.h"

class BaseRenderer: public IRenderer, public PluginBase
{
public:
    BaseRenderer(ICommandExecutor& executor, Configuration& config);
    virtual ~BaseRenderer();

    bool start(int, int, const char*) = 0;
    void stop() = 0;
    void setdebug(bool onoff);

    virtual void doScreenShot(std::string fileToSave) = 0;
    virtual uint getLayerTypeCapabilities(LayerType layerType);
    virtual InputManager* getInputManager() const {return m_pInputManager;}
    virtual Shader* createShader(const string* vertexName, const string* fragmentName); 
    virtual bool setOptimizationMode(unsigned int id, unsigned int mode)
    {
        (void)id;
        (void)mode;
        return false;
    }
    virtual bool getOptimizationMode(unsigned int id, unsigned int *mode)
    {
        (void)id;
        (void)mode;
        return false;
    }
protected:
    Scene* m_pScene;
    InputManager* m_pInputManager;

private:
    static bool debugMode;
};

inline Shader* BaseRenderer::createShader(const string* vertexName, const string* fragmentName) 
{
	(void)vertexName; // TODO: removed, prevents warning
	(void)fragmentName; // TODO: removed, prevents warning
    return NULL;
}
inline void BaseRenderer::setdebug(bool onoff)
{
    debugMode = onoff;
}

#endif /* _BASERENDERER_H_*/
