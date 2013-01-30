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

#ifndef _X11WINDOWSYSTEM_H_
#define _X11WINDOWSYSTEM_H_
#include "WindowSystems/BaseWindowSystem.h"
#include "GraphicSystems/BaseGraphicSystem.h"
#include "X11/Xlib.h"
#include "Surface.h"
#include "PlatformSurfaces/XPlatformSurface.h"
#include <X11/Xutil.h>
#include "Log.h"
#include "ScreenShotType.h"


typedef XVisualInfo* (*GetVisualInfoFunction)(Display *dpy);

typedef enum x11WindowSystemStates
{
    REDRAW_STATE = 0,
    WAKEUP_STATE = 1,
    IDLE_STATE = 2,
    UNKOWN_STATE
} X11WindowSystemStates;

class X11WindowSystem: public BaseWindowSystem
{
public:
    X11WindowSystem(const char* displayname, int width, int height, Scene* pScene, InputManager* pInputManager, GetVisualInfoFunction func = X11WindowSystem::getDefaultVisual);
    virtual ~X11WindowSystem();
    bool init(BaseGraphicSystem<Display*, Window>* sys);
    bool start();
    void stop();
    static XVisualInfo * getDefaultVisual(Display *dpy);
    void signalRedrawEvent();
    void wakeUpRendererThread();
    void setSystemState (X11WindowSystemStates state);
    X11WindowSystemStates getSystemState ();
    Display* getNativeDisplayHandle();
    Window getCompositorNativeWindowHandle();
    virtual void allocatePlatformSurface(Surface *surface);
    virtual void deallocatePlatformSurface(Surface *surface);
    void doScreenShot(std::string fileName);
    void doScreenShotOfLayer(std::string fileName, const uint id);
    void doScreenShotOfSurface(std::string fileName, const uint id, const uint layer_id);
private:
    ScreenShotType takeScreenshot;
    std::string screenShotFile;
    uint screenShotSurfaceID;
    uint screenShotLayerID;
    const char* displayname;
    GetVisualInfoFunction getVisualFunc;
    bool debugMode;
    int resolutionWidth;
    int resolutionHeight;
    static int composite_opcode;
    int composite_event, composite_error;
    int composite_major, composite_minor;
    static int damage_opcode;
    int damage_event, damage_error;
    int damage_major, damage_minor;
    static const char CompositorWindowTitle[];
    bool m_running; 
/*    bool m_initialized; */
    bool m_success;
    X11WindowSystemStates m_systemState;
    const char* m_displayEnvironment;

protected:
    Display* x11Display;

    pthread_t renderThread;
    //void setDisplayMode();
    int windowWidth;
    int windowHeight;
    //    Window background;
    Window CompositorWindow;
    XVisualInfo* windowVis;
    float m_fpsinterval;    
    pthread_mutex_t run_lock;
    pthread_mutex_t init_lock;
    pthread_cond_t init_condition;
    pthread_cond_t run_condition;
    BaseGraphicSystem<Display*, Window>* graphicSystem;
    virtual bool initXServer();
    virtual void RedrawAllLayers(bool clear, bool swap);
    virtual void renderHWLayer(Layer* layer);
    virtual void calculateFps();
    virtual void calculateSurfaceFps(Surface *currentSurface, float time) ;
private:
    void cleanup();
    void Screenshot();
    void Redraw();
    bool OpenDisplayConnection();
    bool checkForCompositeExtension();
    bool checkForDamageExtension();
    void createSurfaceForWindow(Window w);
    void configureSurfaceWindow(Window w);
    Surface* getSurfaceForWindow(Window w);
    void checkForNewSurfaceNativeContent();
    void destroy_surface(Window w);
    void updateSurface(Surface* s, Window w, XPlatformSurface* x11surf);
    void MapWindow(Window w);
    void UnMapWindow(Window w);
    void NewWindow(Surface* s, Window w);
    void DestroyWindow(Window w);
    bool isWindowValid(Window w);
    bool CreatePixmapsForAllWindows();
    bool CreateCompositorWindow();
    void UnredirectSpecialWIndows(Window w);

    void printDebug();
    void* EventLoop();
    static int error(Display *dpy, XErrorEvent *ev);
    void ManageXInputEvent(InputDevice type, InputEventState state, XEvent *pevent);
    static bool m_xerror;


    friend void * X11eventLoopCallback(void *);
};

inline void X11WindowSystem::setSystemState (X11WindowSystemStates state)
{
    m_systemState = state;
}

inline X11WindowSystemStates X11WindowSystem::getSystemState () 
{
    return m_systemState;
}

inline Display* X11WindowSystem::getNativeDisplayHandle()
{
    return x11Display;
}

inline Window X11WindowSystem::getCompositorNativeWindowHandle()
{
    return CompositorWindow;
}

#endif /* _X11WINDOWSYSTEM_H_ */
