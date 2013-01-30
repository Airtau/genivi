/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
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

#ifndef _DRMGLESGRAPHICSYSTEM_H_
#define _DRMGLESGRAPHICSYSTEM_H_

#include "GraphicSystems/BaseGraphicSystem.h"
#include "GraphicSystems/GLESGraphicSystem.h"
#include "ShaderProgramFactory.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "Log.h"
#include "Shader.h"

#include "wayland-util.h"

#include <xf86drm.h>
#include <xf86drmMode.h>

struct gbm_device;

struct DrmMode {
	uint32_t		flags;
	int32_t			width;
	int32_t			height;
	uint32_t		refresh;
	drmModeModeInfo	modeInfo;
	wl_list			link;
};

struct DrmOutput {
	struct wl_list  link;
	struct DrmMode*	currentMode;
	struct wl_list	modeList;
	uint32_t        crtcID;
	uint32_t        connectorID;
	drmModeCrtcPtr  orgCrtc;
	uint32_t        fbID[2];
	uint32_t        current;
	int             fdDev;
	struct gbm_surface*	surface[2];
	EGLSurface	eglSurface[2];
	uint32_t	screenID;
};

class DrmGLESGraphicSystem: public GLESGraphicsystem
{
// functions
public:
    DrmGLESGraphicSystem(int windowWidth, int windowHeight,
                 PfnShaderProgramCreator shaderProgram);
    virtual ~DrmGLESGraphicSystem();

    virtual bool init(EGLNativeDisplayType display, EGLNativeWindowType window);
    virtual void activateGraphicContext();
    virtual void swapBuffers();
    virtual void updateScreenList(LmScreenList& screenList);
    virtual void switchScreen(uint screenid);

// proterties
private:
	struct wl_list m_outputList;

	gbm_device*    m_gbm;
	struct gbm_surface*   m_dummySurface;
	EGLSurface     m_dummyEglSurface;
	int            m_fdDev;
	uint32_t*      m_crtcs;
	int            m_crtcsNum;
	uint32_t       m_crtcAllocator;
	uint32_t       m_connectorAllocator;
	DrmOutput*     m_currentOutput;

	PFNEGLBINDWAYLANDDISPLAYWL						m_pfEglBindWaylandDisplayWL;
	PFNEGLUNBINDWAYLANDDISPLAYWL					m_pfEglUnbindWaylandDisplayWL;

// private functions
private:
    bool initializeSystem();
	bool createOutputs();
	int  createOutputForConnector(drmModeRes* resources, drmModeConnector* connector, int x, int y);
	int  drmOutputAddMode(struct DrmOutput* output, drmModeModeInfo* info);
	int  drmOutputPrepareRender(struct DrmOutput* output);
};

#endif /* _DRMGLESGRAPHICSYSTEM_H_ */
