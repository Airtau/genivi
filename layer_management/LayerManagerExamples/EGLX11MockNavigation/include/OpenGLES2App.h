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
#ifndef _OpenGLES2App_h
#define _OpenGLES2App_h

#include <GLES2/gl2.h>

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include <EGL/egl.h>

struct SurfaceConfiguration
{
    unsigned int layerId;
    unsigned int surfaceId;
    unsigned int surfaceWidth;
    unsigned int surfaceHeight;
    unsigned int surfacePosX;
    unsigned int surfacePosY;
    float opacity;
    bool nosky;
};

class OpenGLES2App
{
public:
    OpenGLES2App(float fps, float animationSpeed, SurfaceConfiguration* config);
    virtual ~OpenGLES2App();

    void mainloop();

protected:
    virtual void update(int currentTimeInMs, int elapsedTimeInMs) = 0;
    virtual void render() = 0;
	void swapBuffers();

private:
	bool createX11Context(SurfaceConfiguration* config);
	void destroyX11Context();

	bool createEGLContext();
	void destroyEglContext();

	bool setupLayerMangement(SurfaceConfiguration* config);

	unsigned int GetTickCount();

protected:
    float m_framesPerSecond;
    float m_animationSpeed;
    unsigned int m_timerIntervalInMs;

    struct EglContextStruct
    {
        EGLDisplay eglDisplay;
        EGLConfig eglConfig;
        EGLSurface eglSurface;
        EGLContext eglContext;
    };

    EglContextStruct m_eglContextStruct;

    struct X11ContextStruct
    {
        Window x11Window;
        Display* x11Display;
        long x11Screen;
        XVisualInfo* x11Visual;
        Colormap x11Colormap;
    };

    X11ContextStruct m_x11ContextStruct;

};

#endif
