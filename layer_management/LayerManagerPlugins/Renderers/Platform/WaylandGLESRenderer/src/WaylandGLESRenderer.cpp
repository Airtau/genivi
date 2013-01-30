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

#include "WaylandGLESRenderer.h"
#include "Configuration.h"
#include "Shader.h"
#include "ShaderProgramGLES.h"
#include "TextureBinders/WaylandGLESTexture.h"

WaylandGLESRenderer::WaylandGLESRenderer(ICommandExecutor& executor, Configuration& config)
: BaseRenderer(executor, config)
, m_pWindowSystem(0)
, m_pGraphicSystem(0)
, m_width(0)
, m_height(0)
, m_binder(0)
{
    LOG_DEBUG("WaylandGLESRenderer", "Creating Renderer");
}

bool WaylandGLESRenderer::start(int width, int height, const char* displayname)
{
    struct wl_display* nativeDisplayHandle = NULL;
    EGLDisplay eglDisplayhandle = NULL;
    m_binder = NULL;
    m_width = width;
    m_height = height;

    // add default screen
    LmScreenList& screenList = m_pScene->getScreenList();
    LmScreen* lmScreen = new LmScreen();
    screenList.push_back(lmScreen);

    // create Wayland windows, register as composite manager etc
    m_pWindowSystem = getWindowSystem(displayname);
    if( m_pWindowSystem == NULL )
    {
    LOG_ERROR("WaylandGLESRenderer", "Window system is not specified. Consider to specify WITH_WAYLAND_X11 or WITH_WAYLAND_FBDEV");
    goto fail; // TODO bad style
    }

    m_pGraphicSystem = getGraphicSystem(ShaderProgramGLES::createProgram);

    if (!m_pWindowSystem->init((BaseGraphicSystem<void*, void*>*) m_pGraphicSystem))
    {
        goto fail; // TODO bad style
    }

    m_pGraphicSystem->setBaseWindowSystem(m_pWindowSystem);

    // create graphic context from window, init egl etc
    nativeDisplayHandle = m_pWindowSystem->getNativeDisplayHandle();

    LOG_DEBUG("WaylandGLESRenderer", "Got nativedisplay handle: " << nativeDisplayHandle << " from windowsystem");

    eglDisplayhandle = m_pGraphicSystem->getEGLDisplay();

    m_binder = new WaylandGLESTexture(eglDisplayhandle, nativeDisplayHandle);
    if (m_binder && nativeDisplayHandle && eglDisplayhandle)
    {
        m_pGraphicSystem->setTextureBinder(m_binder);

        if (!m_pWindowSystem->start())
        {
            goto fail; // TODO bad style
        }
    }
    else
    {
        goto fail; // TODO bad style
    }
    return true;

    fail: // TODO bad style

    LOG_ERROR("WaylandGLESRenderer", "Initialization failed !");
    return false;
}

void WaylandGLESRenderer::stop()
{
    m_pWindowSystem->stop();
    if(m_binder)
    {
        delete m_binder;
    }
}

void WaylandGLESRenderer::doScreenShot(std::string fileToSave)
{
    m_pWindowSystem->doScreenShot(fileToSave);
}

void WaylandGLESRenderer::doScreenShotOfLayer(std::string fileToSave, uint id)
{
    m_pWindowSystem->doScreenShotOfLayer(fileToSave,id);
}

void WaylandGLESRenderer::doScreenShotOfSurface(std::string fileToSave, uint id, uint layer_id)
{
    m_pWindowSystem->doScreenShotOfSurface(fileToSave,id,layer_id);
}

uint WaylandGLESRenderer::getNumberOfHardwareLayers(uint screenID)
{
    (void)screenID;

    return 0; // TODO provide real value here
}

uint* WaylandGLESRenderer::getScreenResolution(uint screenID)
{
    (void)screenID;
    // TODO provide value of real screen here
    uint * resolution = new uint[2];
    resolution[0] = m_width;
    resolution[1] = m_height;
    return resolution;
}

uint* WaylandGLESRenderer::getScreenIDs(uint* length)
{
    // TODO necessary to implement
    uint* screenIDS = new uint[1];
    screenIDS[0] = 0;
    *length = 1;
    return screenIDS;
}

void WaylandGLESRenderer::signalWindowSystemRedraw()
{
    m_pWindowSystem->signalRedrawEvent();
}

void WaylandGLESRenderer::forceCompositionWindowSystem()
{
    m_pWindowSystem->m_forceComposition = true;
}

bool WaylandGLESRenderer::setOptimizationMode(OptimizationType id, OptimizationModeType mode)
{
    return m_pGraphicSystem->setOptimizationMode(id, mode);
}

bool WaylandGLESRenderer::getOptimizationMode(OptimizationType id, OptimizationModeType *mode)
{
    return m_pGraphicSystem->getOptimizationMode(id, mode);
}

HealthCondition WaylandGLESRenderer::pluginGetHealth()
{
    return BaseRenderer::pluginGetHealth();
}

Shader* WaylandGLESRenderer::createShader(const string* vertexName, const string* fragmentName)  
{
    Shader *result = NULL;
    m_pWindowSystem->setSystemState(WAKEUP_STATE);
    m_pWindowSystem->wakeUpRendererThread();
    m_pGraphicSystem->activateGraphicContext();
    result = Shader::createShader(*vertexName,*fragmentName);
    m_pGraphicSystem->releaseGraphicContext();
    m_pWindowSystem->setSystemState(IDLE_STATE);
    return result;
}