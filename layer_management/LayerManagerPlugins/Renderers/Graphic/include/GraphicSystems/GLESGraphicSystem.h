/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#ifndef _GLESGRAPHICSYSTEM_H_
#define _GLESGRAPHICSYSTEM_H_

#include "GraphicSystems/BaseGraphicSystem.h"
#include "ShaderProgramFactory.h"
#include "EGL/egl.h"
#include "Log.h"
#include "Shader.h"

class IlmMatrix;

struct MultiSurfaceRegion
{
    FloatRectangle m_rect;
    SurfaceList m_surfaces;
};

class GLESGraphicsystem: public BaseGraphicSystem<EGLNativeDisplayType, EGLNativeWindowType>
{
public:

    GLESGraphicsystem(int windowHeight, int windowWidth,
            PfnShaderProgramCreator shaderProgram);

    virtual bool init(EGLNativeDisplayType display, EGLNativeWindowType window);

    virtual void clearBackground();

    virtual void activateGraphicContext();
    virtual void releaseGraphicContext();

    virtual void swapBuffers();

    virtual void beginLayer(Layer* layer);
    virtual void endLayer();

    virtual bool needsRedraw(Layer *layer);
    virtual bool needsRedraw(LayerList layers);
    virtual void renderSWLayer(Layer* layer, bool clear);
    virtual void renderSWLayers(LayerList layers, bool clear);

    virtual bool initOpenGLES(EGLint displayWidth, EGLint displayHeight);
    virtual void resize(EGLint displayWidth, EGLint displayHeight);

    virtual void saveScreenShotOfFramebuffer(std::string fileToSave);
    virtual bool setOptimizationMode(OptimizationType id, OptimizationModeType mode);
    virtual bool getOptimizationMode(OptimizationType id, OptimizationModeType *mode);

    virtual EGLDisplay getEGLDisplay()
    {
        return m_eglDisplay;
    }

    virtual void renderSurface(Surface* surface);
    virtual Shader *pickOptimizedShader(SurfaceList surfaces, bool needsBlend);
    virtual void applyLayerMatrix(IlmMatrix& matrix);

    virtual bool needsBlending(SurfaceList surfaces);
    virtual unsigned shaderKey(int numSurfaces,
                               int needsBlend,
                               int hasTransparency1, int hasAlphaChannel1, int hasChromakey1,
                               int hasTransparency2, int hasAlphaChannel2, int hasChromakey2,
                               int hasTransparency3, int hasAlphaChannel3, int hasChromakey3,
                               int hasTransparency4, int hasAlphaChannel4, int hasChromakey4);
    virtual void debugShaderKey(unsigned key);
protected:

    virtual bool setupTextureForChromaKey();
    virtual void createPbufferSurface();
    virtual void createTempTexture();
    virtual void renderTempTexture();
    virtual void destroyTempTexture();

protected:
    virtual std::list<MultiSurfaceRegion*> computeRegions(LayerList layers, bool clear);
    virtual void renderRegion(MultiSurfaceRegion* region, bool blend);
    virtual bool renderSurfaces(SurfaceList surfaces, FloatRectangle targetDestination, bool blend);

    virtual bool canMultitexture(LayerList layers);
    virtual bool useMultitexture();
    virtual bool canSkipClear();
    virtual bool useSkipClear(LayerList layers);

    int m_windowWidth;
    int m_windowHeight;
    EGLNativeDisplayType m_nativeDisplay;
    EGLNativeWindowType m_nativeWindow;
    PfnShaderProgramCreator m_shaderCreatorFunc;
    EGLConfig m_eglConfig;
    EGLContext m_eglContext;
    EGLSurface m_eglSurface;
    EGLSurface m_eglPbufferSurface;
    EGLDisplay m_eglDisplay;
    uint m_vbo;
    EGLint m_displayWidth;
    EGLint m_displayHeight;
    EGLBoolean m_blendingStatus;
    Shader* m_defaultShaderClear;
    Shader* m_defaultShader;
    Shader* m_defaultShaderNoBlend;
    Shader* m_defaultShaderNoUniformAlpha;

    Shader* m_defaultShaderAddUniformChromaKey;

    Shader* m_defaultShaderNoUniformAlphaNoBlend;
    Shader* m_defaultShader2surf;
    Shader* m_defaultShader2surfNoBlend;
    Shader* m_defaultShader2surfNoUniformAlpha;
    Shader* m_defaultShader2surfNoUniformAlphaNoBlend;
    Shader* m_defaultShader2surfNoUniformAlpha0;
    Shader* m_defaultShader2surfNoUniformAlpha0NoBlend;
    Shader* m_defaultShader2surfNoUniformAlpha1;
    Shader* m_defaultShader2surfNoUniformAlpha1NoBlend;
    std::map<int, Shader*> m_shaders;

    Layer* m_currentLayer;
    uint m_texId;

    OptimizationModeType m_optimizations[OPT_COUNT];
private:
    void saveScreenShot();
};

#endif /* _GLESGRAPHICSYSTEM_H_ */
