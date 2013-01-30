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

#ifndef _GLXGRAPHICSYSTEM_H_
#define _GLXGRAPHICSYSTEM_H_

#include "BaseGraphicSystem.h"
#include "PlatformSurfaces/GLXPlatformSurface.h"
#include "Log.h"

class GLXGraphicsystem : public BaseGraphicSystem<Display*, Window>
{
public:
    GLXGraphicsystem( int windowWidth, int windowHeight);
    virtual ~GLXGraphicsystem();
    static XVisualInfo* GetMatchingVisual(Display *dpy);
    virtual bool init(Display* x11Display, Window x11Window);
    virtual bool isZeroCopyEnabled()
    {
        return m_zerocopy;
    }
    virtual void beginLayer(Layer* layer);
    virtual void endLayer();

    virtual bool needsRedraw(Layer *layer);
    virtual bool needsRedraw(LayerList layers);
    virtual void renderSWLayer(Layer *layer, bool clear);
    virtual void renderSWLayers(LayerList layers, bool clear);

    virtual void clearBackground();
    virtual void swapBuffers();
    virtual void saveScreenShotOfFramebuffer(std::string fileToSave);
    GLXFBConfig* GetMatchingPixmapConfig(Display *curDisplay);
    bool CheckConfigValue(Display *curDisplay,GLXFBConfig currentConfig, int attribute, int expectedValue);
    bool CheckConfigMask(Display *curDisplay,GLXFBConfig currentConfig, int attribute, int expectedValue);
    virtual void renderSurface(Surface* currentSurface);
    virtual void activateGraphicContext();
    virtual void releaseGraphicContext();

private:
    int         m_windowWidth;
    int         m_windowHeight;
    Display*    m_x11display;
    Window      m_window;
    GLXContext  m_context;
    Layer*      m_currentLayer;
    bool        m_zerocopy;
};

#endif /* _GLXGRAPHICSYSTEM_H_ */
