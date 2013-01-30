/***************************************************************************
*
* Copyright 2010 BMW Car IT GmbH
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

#include "X11GLESRenderer.h"
#include "Configuration.h"
#include "Shader.h"
#include "ShaderProgramGLES.h"
#include "X11/Xlib.h"
#include "TextureBinders/X11CopyGLES.h"
#include "TextureBinders/X11EglImage.h"
#include <pthread.h>
#include <signal.h>

X11GLESRenderer::X11GLESRenderer(ICommandExecutor& executor, Configuration& config)
: BaseRenderer(executor, config)
, m_pWindowSystem(0)
, m_pGraphicSystem(0)
, m_width(0)
, m_height(0)
, m_binder(NULL)
{
    LOG_DEBUG("X11GLESRenderer", "Creating Renderer");
}

bool X11GLESRenderer::start(int width, int height, const char* displayname)
{
    Display* nativeDisplayHandle = NULL;
    EGLDisplay eglDisplayhandle = NULL;
    m_binder = NULL;
    m_width = width;
    m_height = height;

    // add default screen
    LmScreenList& screenList = m_pScene->getScreenList();
    LmScreen* lmScreen = new LmScreen();
    screenList.push_back(lmScreen);

    // create X11 windows, register as composite manager etc
    m_pWindowSystem = new X11WindowSystem(displayname, width, height, m_pScene, m_pInputManager);
    m_pGraphicSystem = new GLESGraphicsystem(width,height, ShaderProgramGLES::createProgram);

    if (!m_pWindowSystem->init(m_pGraphicSystem))
    {
        goto fail; // TODO bad style
    }

    m_pGraphicSystem->setBaseWindowSystem(m_pWindowSystem);

    // create graphic context from window, init egl etc
    nativeDisplayHandle = m_pWindowSystem->getNativeDisplayHandle();

    LOG_DEBUG("X11GLESRenderer", "Got nativedisplay handle: " << nativeDisplayHandle << " from windowsystem");

    eglDisplayhandle = m_pGraphicSystem->getEGLDisplay();

#ifdef WITH_FORCE_COPY
    m_binder = new X11CopyGLES(eglDisplayhandle, nativeDisplayHandle);
#else // WITH_FORCE_COPY
#ifdef EGL_NATIVE_PIXMAP_KHR
    m_binder = new X11EglImage(eglDisplayhandle, nativeDisplayHandle);
#else // EGL_NATIVE_PIXMAP_KHR
    m_binder = new X11CopyGLES(eglDisplayhandle, nativeDisplayHandle);
#endif // EGL_NATIVE_PIXMAP_KHR
#endif // WITH_FORCE_COPY
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

    LOG_ERROR("X11GLESRenderer", "Initialization failed !");
    return false;
}

void X11GLESRenderer::stop()
{
    m_pWindowSystem->stop();
    if (m_binder)
    {
        delete m_binder;
    }
}

void X11GLESRenderer::doScreenShot(std::string fileToSave)
{
    m_pWindowSystem->doScreenShot(fileToSave);
}

void X11GLESRenderer::doScreenShotOfLayer(std::string fileToSave, uint id)
{
    m_pWindowSystem->doScreenShotOfLayer(fileToSave,id);
}

void X11GLESRenderer::doScreenShotOfSurface(std::string fileToSave, uint id, uint layer_id)
{
    m_pWindowSystem->doScreenShotOfSurface(fileToSave,id,layer_id);
}

uint X11GLESRenderer::getNumberOfHardwareLayers(uint screenID)
{
    (void)screenID;

    return 0; // TODO provide real value here
}

uint* X11GLESRenderer::getScreenResolution(uint screenID)
{
    (void)screenID;

    // TODO provide value of real screen here
    uint * resolution = new uint[2];
    resolution[0] = m_width;
    resolution[1] = m_height;
    return resolution;
}

uint* X11GLESRenderer::getScreenIDs(uint* length)
{
    Display* x11Display = m_pWindowSystem->getNativeDisplayHandle();
    if (!x11Display)
    {
        return NULL;
    }
    // Screens in X11 can be addresses/accessed by just the number - we must only know how many there are
    uint numberOfScreens = ScreenCount(x11Display);
    uint* screenIDS = new uint[numberOfScreens];
    for (uint i = 0; i < numberOfScreens; ++i)
    {
        screenIDS[i] = i;
    }
    *length = numberOfScreens;
    return screenIDS;
}

void X11GLESRenderer::signalWindowSystemRedraw()
{
    m_pWindowSystem->signalRedrawEvent();
}

void X11GLESRenderer::forceCompositionWindowSystem()
{
    m_pWindowSystem->m_forceComposition = true;
}

bool X11GLESRenderer::setOptimizationMode(OptimizationType id, OptimizationModeType mode)
{
    return m_pGraphicSystem->setOptimizationMode(id, mode);
}

bool X11GLESRenderer::getOptimizationMode(OptimizationType id, OptimizationModeType *mode)
{
    return m_pGraphicSystem->getOptimizationMode(id, mode);
}

t_ilm_const_string X11GLESRenderer::pluginGetName() const
{
    return "X11GLESRenderer";
}

Shader* X11GLESRenderer::createShader(const string* vertexName, const string* fragmentName)
{
    Shader *result = NULL;
    m_pWindowSystem->setSystemState(WAKEUP_STATE);
    m_pWindowSystem->wakeUpRendererThread();
    m_pScene->unlockScene();
    while (m_pWindowSystem->getSystemState() != IDLE_STATE);
    m_pGraphicSystem->activateGraphicContext();
    result = Shader::createShader(*vertexName,*fragmentName);
    m_pGraphicSystem->releaseGraphicContext();
    m_pWindowSystem->setSystemState(WAKEUP_STATE);
    m_pScene->lockScene();
    return result;
}

HealthCondition X11GLESRenderer::pluginGetHealth()
{
    HealthCondition health = PluginBase::pluginGetHealth();
    if (0 != pthread_kill(m_pWindowSystem->mThreadId, 0))
    {
        health = HealthDead;
    }
    return health;
}

DECLARE_LAYERMANAGEMENT_PLUGIN(X11GLESRenderer)
