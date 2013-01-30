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
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 ****************************************************************************/

#ifndef _WAYLANDWINDOWSYSTEM_H_
#define _WAYLANDWINDOWSYSTEM_H_
#include "WindowSystems/BaseWindowSystem.h"
#include "GraphicSystems/BaseGraphicSystem.h"
#include "Surface.h"
#include <wayland-server.h>
#include <wayland-client.h>
#include "PlatformSurfaces/WaylandPlatformSurface.h"
#include "Log.h"
#include "ScreenShotType.h"
#include "config.h"
#include "InputManager.h"
#include "WindowSystems/WaylandInputEvent.h"
#include "Rectangle.h"

extern "C" {
// TODO:to abstract
typedef enum waylandWindowSystemStates
{
    REDRAW_STATE = 0,
    WAKEUP_STATE = 1,
    IDLE_STATE = 2,
    UNKOWN_STATE
} WaylandWindowSystemStates;

typedef struct _wlevent {
    int        x, y;
    int32_t    button;
    enum wl_pointer_button_state buttonState;
    long       keyCode;
    enum wl_keyboard_key_state   keyState;
    uint32_t   serial;
    uint32_t   axis;
    wl_fixed_t axisValue;
    int        touchId;
    int        touchType;
} WLEvent;

} // extern "C"

struct native_surface;

class WaylandBaseWindowSystem: public BaseWindowSystem
{
public:
    WaylandBaseWindowSystem(const char* displayname, int width, int height, Scene* pScene, InputManager* pInputManager);
    virtual ~WaylandBaseWindowSystem();
    bool init(BaseGraphicSystem<void*, void*>* sys);
    bool start();
    void stop();
    void signalRedrawEvent();
    void wakeUpRendererThread();
    void setSystemState(WaylandWindowSystemStates state);   // TODO:don't check state
    WaylandWindowSystemStates getSystemState();
    struct wl_display* getNativeDisplayHandle();
    virtual void allocatePlatformSurface(Surface *surface);
    virtual void deallocatePlatformSurface(Surface *surface);
    void doScreenShot(std::string fileName);
    void doScreenShotOfLayer(std::string fileName, const uint id);
    void doScreenShotOfSurface(std::string fileName, const uint id, const uint layer_id);
    int getWindowWidth() const;
    int getWindowHeight() const;

protected:
    struct wl_display* m_wlDisplay;
    struct wl_display* m_wlDisplayClient;
    struct wl_registry* m_wlRegistryClient;
    struct wl_compositor* m_wlCompositorClient;
    struct wl_surface* m_wlSurfaceClient;
    pthread_t renderThread;
    pthread_mutex_t run_lock;
    BaseGraphicSystem<void*, void*>* graphicSystem;
    virtual void RedrawAllLayers(bool clear, bool swap);
    virtual void renderHWLayer(Layer* layer);
    virtual bool initCompositor();
    struct wl_shm* m_wlShm;
    struct wl_global* m_serverInfoGlobal;
    void* m_serverInfo;
    struct wl_global* m_wlCompositorGlobal;
    virtual bool createNativeContext() = 0;
    virtual bool initGraphicSystem() = 0;

    bool m_initialized;
    ScreenShotType m_takeScreenshot;
    const char* m_displayname;
    bool m_success;
    WaylandWindowSystemStates m_systemState;
    uint m_manageConnectionId;
    std::string m_screenShotFile;
    uint m_screenShotSurfaceID;
    uint m_screenShotLayerID;
    bool m_debugMode;
    bool m_error;
    int m_width;
    int m_height;

    struct wl_list m_listFrameCallback;
    struct wl_list m_nativeSurfaceList;

    void createServerinfo(WaylandBaseWindowSystem* windowSystem);
    struct native_surface* createNativeSurface();
    void postReleaseBuffer(struct wl_buffer *buffer);
    virtual void attachBufferToNativeSurface(struct wl_buffer* buffer, struct wl_surface* surface);
    void repaint(int msecs);
    void cleanup();
    void Screenshot();
    void Redraw();
    void shutdownCompositor();
    Surface* getSurfaceFromNativeSurface(struct native_surface* nativeSurface);
    struct native_surface* getNativeSurfaceFromSurface(Surface* surface);
    virtual void checkForNewSurfaceNativeContent();
    void calculateFps();
    void calculateSurfaceFps(Surface *currentSurface, float time) ;
    void printDebug();
    void* eventLoop();

    static void bindCompositor(struct wl_client* client, void* data, uint32_t version, uint32_t id);
    static int signalEventOnTerm(int signal_number, void *data);
    static void destroyListenerSurfaceBuffer(struct wl_listener* listener,void *data);
    static void destroyListenerSurfacePendingBuffer(struct wl_listener* listener,void *data);
    static void idleEventRepaint(void *data);
    bool createWaylandClient();
    void releaseWaylandClient();

    WaylandInputEvent* m_inputEvent;
    virtual bool createInputEvent();

public:
    static void serverinfoIFCreateConnection(struct wl_client *client, struct wl_resource *resource);
    static void bindServerinfo(struct wl_client *client, void *data, uint32_t version, uint32_t id);
    static uint32_t getTime(void);
    static void destroySurfaceCallback(struct wl_resource* resource);
    static void destroyFrameCallback(struct wl_resource *resource);
    static void* eventLoopCallback(void* ptr);
    static void registryHandleGlobalClient(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
    static void surfaceListenerFrame(void* data, struct wl_callback* callback, uint32_t time);

    // wl_surface interface
    static void surfaceIFDestroy(struct wl_client *client, struct wl_resource *resource);
    static void surfaceIFAttach(struct wl_client *client, struct wl_resource *resource, struct wl_resource *buffer_resource, int32_t x, int32_t y);
    static void surfaceIFDamage(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
    static void surfaceIFFrame(struct wl_client *client, struct wl_resource *resource, uint32_t callback);
    static void surfaceIFCommit(struct wl_client *client, struct wl_resource *resource);
    
    // wl_compositor interface
    static void compositorIFCreateSurface(struct wl_client *client, struct wl_resource* resource, uint32_t id);

    struct wl_list m_connectionList;

    // Input event
    void manageWLInputEvent(const InputDevice type, const InputEventState state, const WLEvent *wlEvent);
};

inline void WaylandBaseWindowSystem::setSystemState (WaylandWindowSystemStates state)
{
    m_systemState = state;
}

inline WaylandWindowSystemStates WaylandBaseWindowSystem::getSystemState ()
{
    return m_systemState;
}

inline struct wl_display* WaylandBaseWindowSystem::getNativeDisplayHandle()
{
    return m_wlDisplay;
}

inline bool WaylandBaseWindowSystem::createInputEvent()
{
    return false;
}

inline int WaylandBaseWindowSystem::getWindowWidth() const { return m_width; }
inline int WaylandBaseWindowSystem::getWindowHeight() const { return m_height; }

extern "C" {
    struct native_surface {
        struct wl_surface surface;
        WaylandBaseWindowSystem* windowSystem;
        struct wl_buffer* buffer;
        int connectionId;

        uint32_t visual; // TODO confirm:it's necessary or not
        struct wl_listener buffer_destroy_listener;
        struct wl_list link;

        /* All the pending state, that wl_surface.commit will apply. */
        struct {
            /* wl_surface.attach */
            int remove_contents;
            struct wl_buffer *buffer;
            struct wl_listener buffer_destroy_listener;
            int32_t sx;
            int32_t sy;

            /* wl_surface.damage */
            Rectangle damage;
            bool damaged;

            /* wl_surface.frame */
            struct wl_list frame_callback_list;
        } pending;
    };

    struct native_frame_callback {
        struct wl_resource resource;
        struct wl_list link;
    };

    struct native_process;
    typedef void (*cleanupFuncForNativeProcess)(struct native_process* process, int status);

    struct native_process {
        pid_t pid;
        cleanupFuncForNativeProcess cleanup;
        struct wl_list link;
    };
}

#endif /* _WAYLANDWINDOWSYSTEM_H_ */
