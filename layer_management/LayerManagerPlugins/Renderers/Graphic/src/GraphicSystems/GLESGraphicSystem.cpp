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

#include "GraphicSystems/GLESGraphicSystem.h"
#include "IlmMatrix.h"
#include "string.h"
#include "EGL/egl.h"
#include "GLES2/gl2.h"
#include "Bitmap.h"
#include "ViewportTransform.h"
#include "config.h"
#include <string>
#include <set>
#include <algorithm>

static const float vertices[8 * 12] =
{ 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0,

0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0,

0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0,

1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,

0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0,

1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0,

0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0,

0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0

};

GLESGraphicsystem::GLESGraphicsystem(int windowWidth, int windowHeight, PfnShaderProgramCreator shaderProgram)
: m_windowWidth(windowWidth)
, m_windowHeight(windowHeight)
, m_nativeDisplay(0)
, m_nativeWindow(0)
, m_shaderCreatorFunc(shaderProgram)
, m_eglConfig(0)
, m_eglContext(0)
, m_eglSurface(0)
, m_eglPbufferSurface(0)
, m_eglDisplay(0)
, m_vbo(0)
, m_displayWidth(0)
, m_displayHeight(0)
, m_blendingStatus(false)
, m_defaultShaderClear(0)
, m_defaultShader(0)
, m_defaultShaderNoBlend(0)
, m_defaultShaderNoUniformAlpha(0)
, m_defaultShaderAddUniformChromaKey(0)
, m_defaultShaderNoUniformAlphaNoBlend(0)
, m_defaultShader2surf(0)
, m_defaultShader2surfNoBlend(0)
, m_defaultShader2surfNoUniformAlpha(0)
, m_defaultShader2surfNoUniformAlphaNoBlend(0)
, m_defaultShader2surfNoUniformAlpha0(0)
, m_defaultShader2surfNoUniformAlpha0NoBlend(0)
, m_defaultShader2surfNoUniformAlpha1(0)
, m_defaultShader2surfNoUniformAlpha1NoBlend(0)
, m_currentLayer(0)
, m_texId(0)
{
    LOG_DEBUG("GLESGraphicsystem", "creating GLESGraphicsystem");
    for (int i=0; i < OPT_COUNT; i++)
    {
        m_optimizations[i] = OPT_MODE_HEURISTIC;
    }
}

void GLESGraphicsystem::activateGraphicContext()
{
    eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
}

void GLESGraphicsystem::releaseGraphicContext() 
{
    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

bool GLESGraphicsystem::init(EGLNativeDisplayType display, EGLNativeWindowType NativeWindow)
{
    m_nativeDisplay = display;
    m_nativeWindow = NativeWindow;
    LOG_INFO("GLESGraphicsystem", "Initialisation");
    EGLint iMajorVersion, iMinorVersion;
    LOG_DEBUG("GLESGraphicsystem", "Getting EGL Display with native display " << m_nativeDisplay);
    m_eglDisplay = eglGetDisplay(m_nativeDisplay);

    if (m_eglDisplay == EGL_NO_DISPLAY){
        LOG_ERROR("GLESGraphicsystem", "failed to Get EGL Display");
        return false;
    }

    LOG_DEBUG("GLESGraphicsystem", "Initialising EGL");
    if (!eglInitialize(m_eglDisplay, &iMajorVersion, &iMinorVersion))
    {
        LOG_ERROR("GLESGraphicsystem", "Initialising EGL failed");
        return false;
    }

    LOG_DEBUG("GLESGraphicsystem", "Binding GLES API");
    eglBindAPI(EGL_OPENGL_ES_API);

    EGLint pi32ConfigAttribs[] = {
            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_RED_SIZE,        8,
            EGL_ALPHA_SIZE,      8,
            EGL_NONE
    };

    LOG_DEBUG("GLESGraphicsystem", "EGLChooseConfig");
    int iConfigs;
    if (!eglChooseConfig(m_eglDisplay, pi32ConfigAttribs, &m_eglConfig, 1, &iConfigs) || (iConfigs != 1))
    {
        LOG_DEBUG("GLESGraphicsystem", "Error: eglChooseConfig() failed.");
        return false;
    }
    EGLint id = 0;
    eglGetConfigAttrib(m_eglDisplay, m_eglConfig, EGL_CONFIG_ID, &id);

    EGLint windowsAttr[] = { EGL_RENDER_BUFFER, EGL_BACK_BUFFER, EGL_NONE };

    LOG_DEBUG("GLESGraphicsystem", "Config chosen:" << id);
    LOG_DEBUG("GLESGraphicsystem", "Create Window surface");

    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, m_eglConfig, m_nativeWindow, windowsAttr);
    if (!m_eglSurface)
    {
        EGLenum status = eglGetError();
        LOG_ERROR("GLESGraphicsystem", "Window Surface creation failed with EGL Error Code: "<< status);
        return false;
    }
    LOG_DEBUG("GLESGraphicsystem", "Window Surface creation successfull");

    EGLint contextAttrs[] = {
            EGL_CONTEXT_CLIENT_VERSION,
            2,
            EGL_NONE
    };

    m_eglContext = eglCreateContext(m_eglDisplay, m_eglConfig, NULL, contextAttrs);
    if (!m_eglContext)
    {
        LOG_ERROR("GLESGraphicsystem","EGL couldn't create context\n");
        return false;
    }
    LOG_DEBUG("GLESGraphicsystem", "EGL make current ...");
    // Make the context and surface current for rendering
    eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
    LOG_DEBUG("GLESGraphicsystem", "made current");
    eglSwapInterval(m_eglDisplay, 1);

    if (!initOpenGLES(m_windowWidth, m_windowHeight))
    {
        LOG_ERROR("GLESGraphicsystem", "Initialisation failed");
        return false;
    }
    LOG_INFO("GLESGraphicsystem", "Initialisation successfull");
    return true;
}

// Bits 31:30 = numSurfaces
// Bit  29    = blend on/off
// Bit  28    = unused
// Bits 27:21 = Surface 4 Attributes
// Bits 20:14 = Surface 3 Attributes
// Bits 13:7  = Surface 2 Attributes
// Bits 6:0   = Surface 1 Attributes
#define SHADERKEY_MAX_SURFACES 4
#define SHADERKEY_NUM_SURF_SHIFT 30
#define SHADERKEY_NUM_SURF_MASK  (0x3 << SHADERKEY_NUM_SURF_SHIFT)

#define SHADERKEY_BLEND_SHIFT        29
#define SHADERKEY_BLEND_MASK         (0x1 << SHADERKEY_BLEND_SHIFT)

#define SHADERKEY_BITS_PER_SURF    7
#define SHADERKEY_CHROMAKEY_SHIFT(x)     (2 + (SHADERKEY_BITS_PER_SURF * x))
#define SHADERKEY_ALPHA_CHANNEL_SHIFT(x) (1 + (SHADERKEY_BITS_PER_SURF * x))
#define SHADERKEY_TRANSPARENCY_SHIFT(x)  (0 + (SHADERKEY_BITS_PER_SURF * x))
#define SHADERKEY_CHROMAKEY_MASK(x)     (0x1 << (SHADERKEY_CHROMAKEY_SHIFT(x)))
#define SHADERKEY_ALPHA_CHANNEL_MASK(x) (0x1 << (SHADERKEY_ALPHA_CHANNEL_SHIFT(x)))
#define SHADERKEY_TRANSPARENCY_MASK(x)  (0x1 << (SHADERKEY_TRANSPARENCY_SHIFT(x)))

unsigned int GLESGraphicsystem::shaderKey(int numSurfaces,
                                 int needsBlend,
                                 int hasTransparency0, int hasAlphaChannel0, int hasChromakey0,
                                 int hasTransparency1, int hasAlphaChannel1, int hasChromakey1,
                                 int hasTransparency2, int hasAlphaChannel2, int hasChromakey2,
                                 int hasTransparency3, int hasAlphaChannel3, int hasChromakey3)
{
    return (((numSurfaces << SHADERKEY_NUM_SURF_SHIFT) & SHADERKEY_NUM_SURF_MASK) |
            ((needsBlend << SHADERKEY_BLEND_SHIFT) & SHADERKEY_BLEND_MASK) |

            ((hasChromakey3 << SHADERKEY_CHROMAKEY_SHIFT(3)) & SHADERKEY_CHROMAKEY_MASK(3)) |
            ((hasChromakey2 << SHADERKEY_CHROMAKEY_SHIFT(2)) & SHADERKEY_CHROMAKEY_MASK(2)) |
            ((hasChromakey1 << SHADERKEY_CHROMAKEY_SHIFT(1)) & SHADERKEY_CHROMAKEY_MASK(1)) |
            ((hasChromakey0 << SHADERKEY_CHROMAKEY_SHIFT(0)) & SHADERKEY_CHROMAKEY_MASK(0)) |

            ((hasAlphaChannel3 << SHADERKEY_ALPHA_CHANNEL_SHIFT(3)) & SHADERKEY_ALPHA_CHANNEL_MASK(3)) |
            ((hasAlphaChannel2 << SHADERKEY_ALPHA_CHANNEL_SHIFT(2)) & SHADERKEY_ALPHA_CHANNEL_MASK(2)) |
            ((hasAlphaChannel1 << SHADERKEY_ALPHA_CHANNEL_SHIFT(1)) & SHADERKEY_ALPHA_CHANNEL_MASK(1)) |
            ((hasAlphaChannel0 << SHADERKEY_ALPHA_CHANNEL_SHIFT(0)) & SHADERKEY_ALPHA_CHANNEL_MASK(0)) |

            ((hasTransparency3 << SHADERKEY_TRANSPARENCY_SHIFT(3)) & SHADERKEY_TRANSPARENCY_MASK(3)) |
            ((hasTransparency2 << SHADERKEY_TRANSPARENCY_SHIFT(2)) & SHADERKEY_TRANSPARENCY_MASK(2)) |
            ((hasTransparency1 << SHADERKEY_TRANSPARENCY_SHIFT(1)) & SHADERKEY_TRANSPARENCY_MASK(1)) |
            ((hasTransparency0 << SHADERKEY_TRANSPARENCY_SHIFT(0)) & SHADERKEY_TRANSPARENCY_MASK(0)));
}

void GLESGraphicsystem::debugShaderKey(unsigned int key)
{
    int numSurfaces = ((key & SHADERKEY_NUM_SURF_MASK) >> SHADERKEY_NUM_SURF_SHIFT);
    int needsBlend =  ((key & SHADERKEY_BLEND_MASK) >> SHADERKEY_BLEND_SHIFT);

    LOG_DEBUG("GLESGraphicsystem", "Key: " << std::hex << key
                               << " numSurfaces:" << numSurfaces
                               << " needsBlend:" << needsBlend);

    for (int i = 0; i < SHADERKEY_MAX_SURFACES; i++)
    {
        int hasChromakey =    ((key & SHADERKEY_CHROMAKEY_MASK(i)) >> SHADERKEY_CHROMAKEY_SHIFT(i));
        int hasAlphaChannel = ((key & SHADERKEY_ALPHA_CHANNEL_MASK(i)) >> SHADERKEY_ALPHA_CHANNEL_SHIFT(i));
        int hasTransparency = ((key & SHADERKEY_TRANSPARENCY_MASK(i)) >> SHADERKEY_TRANSPARENCY_SHIFT(i));
        LOG_DEBUG("GLESGraphicsystem", "    Surface" << i << ":"
                                   << " hasTransparency:" << hasTransparency
                                   << " hasAlphaChannel:" << hasAlphaChannel
                                   << " hasChromakey:" << hasChromakey);
    }
}

void GLESGraphicsystem::clearBackground()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLESGraphicsystem::swapBuffers()
{
    eglSwapBuffers(m_eglDisplay, m_eglSurface);
}

void GLESGraphicsystem::beginLayer(Layer* currentLayer)
{
    //LOG_DEBUG("GLESGraphicsystem", "Beginning to draw layer: " << currentLayer->getID());
    m_currentLayer = currentLayer;
    // TODO layer destination / source
}

// Reports whether a single layer is damaged/dirty
// Can not account for possible occlusion by other layers
bool GLESGraphicsystem::needsRedraw(Layer *layer)
{
    if (layer->renderPropertyChanged)
    {
        return true;
    }

    if (layer->visibility && layer->opacity > 0.0)
    {
        SurfaceList surfaces = layer->getAllSurfaces();
        for(SurfaceListConstIterator currentS = surfaces.begin(); currentS != surfaces.end(); currentS++)
        {
            if ((*currentS)->renderPropertyChanged)
            {
                return true;
            }

            if ((*currentS)->hasNativeContent() && (*currentS)->damaged && (*currentS)->visibility && (*currentS)->opacity>0.0f)
            {
                return true;
            }
        }
    }

    return false;
}

// Reports whether the passed in layers have visible damage or are otherwise
// dirty because render properties changed.
// Assumes that layers in the list belong to same composition. ie. damage to
// one layer affects the others.  A warning is logged if the assumption is wrong.
bool GLESGraphicsystem::needsRedraw(LayerList layers)
{
    // TODO: Ignore damage from completely obscured surfaces

    for (LayerListConstIterator layer = layers.begin(); layer != layers.end(); layer++)
    {
        if ((*layer)->getLayerType() == Hardware && layers.size() > 1)
        {
            // Damage in a hardware layer should not imply a redraw in other layers
            LOG_WARNING("GLESGraphicsystem", "needsRedraw() called with layers not in the same composition");
        }

        if (needsRedraw(*layer))
        {
            return true;
        }
    }

    return false;
}

void GLESGraphicsystem::renderSWLayer(Layer *layer, bool clear)
{
    beginLayer(layer);

    if ((layer->getLayerType() != Software_2D) &&
        (layer->getLayerType() != Software_2_5D))
    {
        LOG_WARNING("GLESGraphicsystem", "renderSWLayer() called with a non-SW layer");
    }

    if (clear)
    {
        clearBackground();
    }

    if ( layer->visibility && layer->opacity > 0.0 )
    {
        bool bChromaKeyEnabled = m_currentLayer->getChromaKeyEnabled();
        if (bChromaKeyEnabled && !setupTextureForChromaKey())
        {
            LOG_WARNING("GLESGraphicsystem", "Failed to create Pbuffer. Layer chroma key to be disabled.");
            bChromaKeyEnabled = false;
        }

        SurfaceList surfaces = m_currentLayer->getAllSurfaces();
        for(std::list<Surface*>::const_iterator currentS = surfaces.begin(); currentS != surfaces.end(); currentS++)
        {
            if ((*currentS)->hasNativeContent() && (*currentS)->visibility && (*currentS)->opacity>0.0f)
            {
                renderSurface(*currentS);
            }
        }

        if (bChromaKeyEnabled)
        {
            activateGraphicContext();
            renderTempTexture();
            destroyTempTexture();
        }
    }

    endLayer();
}

bool GLESGraphicsystem::setOptimizationMode(OptimizationType id, OptimizationModeType mode)
{
    if ((int)id >= OPT_COUNT)
    {
       return false;
    }

    if ((int)mode >= OPT_MODE_COUNT)
    {
        return false;
    }

    m_optimizations[id] = mode;

    return true;
}

bool GLESGraphicsystem::getOptimizationMode(OptimizationType id, OptimizationModeType *mode)
{
    if ((int)id >= OPT_COUNT)
    {
       return false;
    }

    *mode = m_optimizations[id];

    return true;
}

// Decide if multitexture rendering is a supported possibility, but not necessarily if
// it should be used.  That is determined in useMultitexture().
bool GLESGraphicsystem::canMultitexture(LayerList layers)
{
    // TODO, cache this result until there is a scene change
    for (LayerListConstIterator layer = layers.begin(); layer != layers.end(); layer++)
    {
        // No layer rotation support currently for multitexture rendering
        // No multitexture rendering if one layer has chromaKeyEnabled
        if ((*layer)->getOrientation() != 0 || (*layer)->getChromaKeyEnabled() )
        {
            return false;
        }

        SurfaceList surfaces = (*layer)->getAllSurfaces();
        for (SurfaceListConstIterator surface = surfaces.begin(); surface != surfaces.end(); surface++)
        {
            // No custom shaders allowed.  We wouldn't know what we were
            // overriding by using multi-surface shaders.
            Shader *shader = (*surface)->getShader();
            if (!shader)
            {
                shader = (*layer)->getShader();
            }

            // TODO, other custom shaders okay too.
            if ( (shader && shader != m_defaultShader) || (*surface)->getChromaKeyEnabled())
            {
                return false;
            }
        }
    }
    return true;
}

// Decide if the multitexturing rendering method should be used.  This does not
// necessarily mean that multitexturing is legal to use.  That is determined in
// canMultitexture().
bool GLESGraphicsystem::useMultitexture()
{
    static int count = 0;
    count++;

    switch(m_optimizations[OPT_MULTITEXTURE])
    {
    case OPT_MODE_FORCE_OFF:
        return false;
    case OPT_MODE_FORCE_ON:
        return true;
    case OPT_MODE_HEURISTIC:
        // TODO: Decide intelligently whether or not to use multitexture
        return true;
    case OPT_MODE_TOGGLE:
        // Toggles optimization on and off to reveal bugs.  If flickering
        // appears, something is broken.  Optimizations should have no impact
        // on image appearance.  For debugging only.
        return (count % 10) > 5;
    default:
        LOG_WARNING("GLESGraphicsystem", "Bad Multitexture Optimization Mode");
        return false;
    }
}

// Decide if the "skip clear" optimization is legal to use, but not necessarily that
// it should be used.  That is determined in useSkipClear().
bool GLESGraphicsystem::canSkipClear()
{
    // No current cases where this approach cannot be used.
    return true;
}

// Decide if the "skip clear" optimization should be used.  This does not necessarily
// mean that the optimization is legal to use.  In this case, it is always legal, but
// not always wise.
bool GLESGraphicsystem::useSkipClear(LayerList layers)
{
    float surfaceArea, displayArea, threshold;
    SurfaceList surfaces;
    static int count = 0;
    count++;

    switch(m_optimizations[OPT_SKIP_CLEAR])
    {
    case OPT_MODE_FORCE_OFF:
        return false;
    case OPT_MODE_FORCE_ON:
        return true;
    case OPT_MODE_HEURISTIC:
        // Sample heuristic.  When large amounts of background are visible, it
        // may be better to disable this optimization and simply use a regular
        // glClear() call.  If very little, or no background is visible,
        // we can skip glClear() to save time.
        displayArea = m_displayWidth * m_displayHeight;
        surfaceArea = 0;
        threshold = .75; // 75% (Needs to be refined by experimentation)
        for(LayerListConstIterator layer = layers.begin(); layer != layers.end(); layer++)
        {
            surfaces = (*layer)->getAllSurfaces();

            if (surfaces.size() == 0) continue;
            if (!(*layer)->visibility || (*layer)->getOpacity() <= 0.0f) continue;

            for(SurfaceListConstIterator currentS = surfaces.begin(); currentS != surfaces.end(); currentS++)
            {
                if ((*currentS)->visibility == true && (*currentS)->getOpacity() > 0.0f)
                {
                    FloatRectangle surfDest = (*currentS)->getTargetDestinationRegion();
                    surfaceArea += surfDest.width * surfDest.height;
                }
            }
            break;
        }
        return (surfaceArea > threshold * displayArea);
    case OPT_MODE_TOGGLE:
        // Toggles optimization on and off to reveal bugs.  If flickering
        // appears, something is broken.  Optimizations should have no impact
        // on image appearance.  For debugging only.
        return (count % 20) > 10;
    default:
        LOG_WARNING("GLESGraphicsystem", "Bad SkipClear Optimization Mode");
        return false;
    }
}

static void incrementDrawCounters(LayerList layers)
{
    for(LayerListConstIterator layer = layers.begin(); layer != layers.end(); layer++)
    {
        SurfaceList surfaces = (*layer)->getAllSurfaces();

        if (surfaces.size() == 0) continue;
        if (!(*layer)->visibility || (*layer)->getOpacity() <= 0.0f) continue;

        for(SurfaceListConstIterator surface = surfaces.begin(); surface != surfaces.end(); surface++)
        {
            if ((*surface)->visibility == true && (*surface)->getOpacity() > 0.0f)
            {
                (*surface)->frameCounter++;
                (*surface)->drawCounter++;
            }
        }
    }
}

// Composite the surfaces from all layers in the provided list.
// A traditional single-texture approach or a potentially faster multi-texture
// method may be used.
// 'clear' means that the framebuffer is dirty and needs to be initialized
// to known values.  Normally achieved through a glClear, it can also be satisfied
// by ensuring that each pixel receives at least one unblended draw.
void GLESGraphicsystem::renderSWLayers(LayerList layers, bool clear)
{
    bool multitexture = useMultitexture() && canMultitexture(layers);
    bool optimizeClear = useSkipClear(layers) && canSkipClear();
    bool countersIncremented = false;

    if (layers.size() == 0)
    {
        if (clear)
        {
            clearBackground();
        }
        return;
    }

    if (clear && !optimizeClear)
    {
        clearBackground();
        clear = false;
    }

    if (multitexture)
    {
        // TODO, compute regions only when scene changes happen, not inside render loop
        std::list<MultiSurfaceRegion*> regions = computeRegions(layers, clear);

        bool blend = !clear;

        for (std::list<MultiSurfaceRegion*>::const_iterator region = regions.begin(); region != regions.end(); region++)
        {
            renderRegion(*region, blend);
        }
    }
    else
    {
        for (LayerListConstIterator layer = layers.begin(); layer != layers.end(); layer++)
        {
            if ((*layer)->getAllSurfaces().size() == 0) continue;
            if (!(*layer)->visibility || (*layer)->getOpacity() <= 0.0f) continue;

            if (clear && optimizeClear)
            {
                // Re-use the computeRegions code to fill out background areas not covered by
                // the first layer of surfaces (in case its not a single fullscreen surface).
                // Then perform an unblended draw for the whole first layer to "skip clear".
                LayerList singleLayer;
                singleLayer.push_back(*layer);
                std::list<MultiSurfaceRegion*> regions = computeRegions(singleLayer, true); //do clear
                for (std::list<MultiSurfaceRegion*>::const_iterator region = regions.begin(); region != regions.end(); region++)
                {
                    renderRegion(*region, false); //don't blend
                }
            } else {
                renderSWLayer(*layer, clear);
                countersIncremented = true;
            }
            clear = false;
        }
    }

    // Increment counters now if the multitexture option was used.
    if (!countersIncremented)
    {
        incrementDrawCounters(layers);
    }
}

void GLESGraphicsystem::endLayer()
{
    //LOG_DEBUG("GLESGraphicsystem", "Done with rendering layer: " << m_currentLayer->getID());
    m_currentLayer = NULL;
}

Shader *GLESGraphicsystem::pickOptimizedShader(SurfaceList surfaces, bool needsBlend)
{
    int numSurfaces = surfaces.size();

    if (numSurfaces > MAX_MULTI_SURFACE || numSurfaces > SHADERKEY_MAX_SURFACES)
    {
        return NULL;
    }

    int hastransparency[SHADERKEY_MAX_SURFACES] = {0};
    int hasalphachannel[SHADERKEY_MAX_SURFACES] = {0};
    int haschromakey[SHADERKEY_MAX_SURFACES] = {0};

    int i = 0;
    for (SurfaceListConstIterator surface = surfaces.begin(); surface != surfaces.end(); surface++, i++)
    {
        int layerId = (*surface)->getContainingLayerId();
        Layer* layer = m_baseWindowSystem->m_pScene->getLayer(layerId);

        hastransparency[i] = ((*surface)->getOpacity() * layer->getOpacity()) < 1.0f;
        hasalphachannel[i] = PixelFormatHasAlpha((*surface)->getPixelFormat());
        haschromakey[i] = (*surface)->getChromaKeyEnabled() ? 1:0 ;
    }

    unsigned int key = shaderKey(numSurfaces,
                        needsBlend ? 1 : 0,
                        hastransparency[0], hasalphachannel[0], haschromakey[0],
                        hastransparency[1], hasalphachannel[1], haschromakey[1],
                        hastransparency[2], hasalphachannel[2], haschromakey[2],
                        hastransparency[3], hasalphachannel[3], haschromakey[3]);

    std::map<int, Shader*>::const_iterator iter = m_shaders.find(key);

//    LOG_DEBUG("GLESGraphicsystem", "Looking for shader with key=" << std::hex << key);
//    debugShaderKey(key);

    if (iter == m_shaders.end())
    {
        LOG_WARNING("GLESGraphicsystem", "No optimal shader found for key=" << std::hex << key);
        LOG_WARNING("GLESGraphicsystem", "Falling back to default shader");

        switch (numSurfaces) {
        case 0:
            return m_defaultShaderClear;
        case 1:
            if (haschromakey[0])
            {
                return m_defaultShaderAddUniformChromaKey;
            } else { 
                return needsBlend ? m_defaultShader : m_defaultShaderNoBlend;
            }
        case 2:
            return needsBlend ? m_defaultShader2surf : m_defaultShader2surfNoBlend;
        default:
            return NULL;
        }
    }

    return (*iter).second;
}

bool GLESGraphicsystem::needsBlending(SurfaceList surfaces)
{
    // Completely opaque surfaces don't need to be blended with framebuffer.
    // Look for any possible translucency.
    for (SurfaceListConstIterator surface = surfaces.begin(); surface != surfaces.end(); surface++)
    {
        if (PixelFormatHasAlpha((*surface)->getPixelFormat()))
        {
            return true;
        }

        int layerId = (*surface)->getContainingLayerId();
        Layer* layer = m_baseWindowSystem->m_pScene->getLayer(layerId);

        if (((*surface)->getOpacity() * layer->getOpacity()) < 1.0f)
        {
            return true;
        }
    }
    return false;
}

void GLESGraphicsystem::applyLayerMatrix(IlmMatrix& matrix)
{
    IlmMatrixRotateZ(matrix, m_currentLayer->getOrientation() * 90.0f);
}

void GLESGraphicsystem::renderSurface(Surface* surface)
{
//  LOG_DEBUG("GLESGraphicsystem", "renderSurface " << surface->getID());
    GLenum glErrorCode = GL_NO_ERROR;
    // check if surface is cropped completely, if so then skip rendering
    if (surface->isCropped())
        return; // skip rendering of this surface, because it is cropped by layer source region

    GLint index = 0;
    IlmMatrix layerMatrix;
    IlmMatrixIdentity(layerMatrix);

    ShaderProgram::CommonUniforms uniforms;
    Shader* layerShader = m_currentLayer->getShader();
    if (!layerShader)
    {
        // use default shader if no custom shader is assigned to this layer
        layerShader = m_defaultShader;
    }
    Shader* shader = (surface)->getShader();
    if (!shader)
    {
        // use layer shader if no custom shader is assigned to this surface
        shader = layerShader;
    }


    FloatRectangle targetSurfaceSource = surface->getTargetSourceRegion();
    FloatRectangle targetSurfaceDestination = surface->getTargetDestinationRegion();

    float textureCoordinates[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSource, surface->OriginalSourceWidth, surface->OriginalSourceHeight, textureCoordinates);

    applyLayerMatrix(layerMatrix);
    // update all common uniforms, scale values to display size
    // offsets are generated w.r.t lower left corner (following GL conventions)
    uniforms.x = targetSurfaceDestination.x / m_displayWidth;
    uniforms.y = 1.0f - (targetSurfaceDestination.y + targetSurfaceDestination.height) / m_displayHeight;;
    uniforms.width = targetSurfaceDestination.width / m_displayWidth;
    uniforms.height = targetSurfaceDestination.height / m_displayHeight;
    uniforms.opacity[0] = (surface)->getOpacity() * m_currentLayer->getOpacity();
    uniforms.texRange[0][0] = (textureCoordinates[2]-textureCoordinates[0]);
    uniforms.texRange[0][1] = (textureCoordinates[3]-textureCoordinates[1]);
    uniforms.texOffset[0][0] = textureCoordinates[0];
    uniforms.texOffset[0][1] = textureCoordinates[1];
    uniforms.texUnit[0] = 0;
    uniforms.matrix = &layerMatrix.f[0];
    uniforms.chromaKeyEnabled = (surface)->getChromaKeyEnabled();
    if (true == uniforms.chromaKeyEnabled)
    {
        unsigned char red = 0;
        unsigned char green = 0;
        unsigned char blue = 0;
        (surface)->getChromaKey(red, green, blue);
        uniforms.chromaKey[0] = (float)red / 255.0f;
        uniforms.chromaKey[1] = (float)green / 255.0f;
        uniforms.chromaKey[2] = (float)blue / 255.0f;
    }

    //We only know about specific Shaders, only do this if we start with the defaultShader
    if (shader == m_defaultShader && uniforms.opacity[0] == 1.0f)
    {
        if(!PixelFormatHasAlpha((surface)->getPixelFormat()))
        {
            //disable alpha blend completely
            glDisable (GL_BLEND);
        }
        else
        {
            //make sure alpha blend is enabled
            glEnable (GL_BLEND);
        }
    }
    else
    {
        //make sure alpha blend is enabled
        glEnable (GL_BLEND);
    }

    {
        SurfaceList sl;
        sl.push_back(surface);
        bool needblend = shader != m_defaultShader ||
                         PixelFormatHasAlpha((surface)->getPixelFormat()) ||
                         uniforms.opacity[0] < 1.0f;
        shader = pickOptimizedShader(sl, needblend);
    }

    shader->use();

    /* load common uniforms */
    shader->loadCommonUniforms(uniforms, 1);

    /* update all custom defined uniforms */
    shader->loadUniforms();
    /* Bind texture and set section */
    glActiveTexture(GL_TEXTURE0);
    if (false == m_binder->bindSurfaceTexture(surface))
    {
        LOG_WARNING("GLESGraphicsystem", "Surface not successfully bind " << surface->getID());
        return;
    }

    /* rotated positions are saved sequentially in vbo
     offset in multiples of 12 decide rotation */
    /* Draw two triangles */
    int orientation = (surface)->getOrientation();
    orientation %= 4;
    index = orientation * 12;
    surface->frameCounter++;
    surface->drawCounter++;
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) (sizeof(float) * 12));
    glEnableVertexAttribArray(1);

    /* Enable only the needed vertex attributes */
    for (int i = 2; i <= MAX_MULTI_SURFACE; i++)
    {
        glDisableVertexAttribArray(i);
    }

    glDrawArrays(GL_TRIANGLES, index, 6);

    m_binder->unbindSurfaceTexture(surface);
    glErrorCode = glGetError();
    if ( GL_NO_ERROR != glErrorCode )
    {
        LOG_ERROR("GLESGraphicsystem", "GL Error occured :" << glErrorCode );
    };
}

// For a given area of the screen, render the appropriate part of all the
// surfaces listed in the provided SurfaceList.
//     returns true if rendering succeeds
//     returns false if rendering was aborted.  e.g. no shader was available
bool GLESGraphicsystem::renderSurfaces(SurfaceList surfaces, FloatRectangle targetDestination, bool blend)
{
    GLenum glErrorCode = GL_NO_ERROR;

    IlmMatrix identityMatrix;
    IlmMatrixIdentity(identityMatrix);

    ShaderProgram::CommonUniforms uniforms;

    if (surfaces.size() > MAX_MULTI_SURFACE)
    {
        return false;
    }

    // update all common uniforms, scale values to display size
    // offsets are generated w.r.t lower left corner (following GL conventions)
    uniforms.x = targetDestination.x / m_displayWidth;
    uniforms.y = 1.0f - (targetDestination.y + targetDestination.height) / m_displayHeight;;
    uniforms.width = targetDestination.width / m_displayWidth;
    uniforms.height = targetDestination.height / m_displayHeight;
    uniforms.matrix = &identityMatrix.f[0];

    // update per-texture uniforms
    int surfacenum = 0;
    for (SurfaceListConstIterator surface = surfaces.begin(); surface != surfaces.end(); surface++, surfacenum++)
    {
        int layerId = (*surface)->getContainingLayerId();
        Layer* layer = m_baseWindowSystem->m_pScene->getLayer(layerId);

        FloatRectangle targetSurfaceSource = (*surface)->getTargetSourceRegion();
        FloatRectangle targetSurfaceDestination = (*surface)->getTargetDestinationRegion();

        FloatRectangle targetRegionSurfaceSource;  // Further restricted to only targetDestination
        float fracx = (targetDestination.x - targetSurfaceDestination.x) / targetSurfaceDestination.width;
        float fracy = (targetDestination.y - targetSurfaceDestination.y) / targetSurfaceDestination.height;
        float widthRatio = targetDestination.width / targetSurfaceDestination.width;
        float heightRatio = targetDestination.height / targetSurfaceDestination.height;
        targetRegionSurfaceSource.x = targetSurfaceSource.x + (targetSurfaceSource.width * fracx);
        targetRegionSurfaceSource.y = targetSurfaceSource.y + (targetSurfaceSource.height * fracy);
        targetRegionSurfaceSource.width = targetSurfaceSource.width * widthRatio;
        targetRegionSurfaceSource.height = targetSurfaceSource.height * heightRatio;

        // Convert cropped surface source rectangle to texture coordinates
        float textureCoordinates[4];
        ViewportTransform::transformRectangleToTextureCoordinates(
            targetRegionSurfaceSource,
            (*surface)->OriginalSourceWidth,
            (*surface)->OriginalSourceHeight,
            textureCoordinates);

        uniforms.opacity[surfacenum] = (*surface)->getOpacity() * layer->getOpacity();

        int effectiveOrientation = ((*surface)->getOrientation() + layer->getOrientation()) % 4;

        switch (effectiveOrientation)
        {
        case 0:
            uniforms.texRange[surfacenum][0] = (textureCoordinates[2]-textureCoordinates[0]);
            uniforms.texRange[surfacenum][1] = (textureCoordinates[3]-textureCoordinates[1]);
            uniforms.texOffset[surfacenum][0] = textureCoordinates[0];
            uniforms.texOffset[surfacenum][1] = textureCoordinates[1];
            break;
        case 1:
            uniforms.texRange[surfacenum][0] = (textureCoordinates[3]-textureCoordinates[1]);
            uniforms.texRange[surfacenum][1] = (textureCoordinates[2]-textureCoordinates[0]);
            uniforms.texOffset[surfacenum][0] = textureCoordinates[1];
            uniforms.texOffset[surfacenum][1] = 1 - textureCoordinates[2];
            break;
        case 2:
            uniforms.texRange[surfacenum][0] = (textureCoordinates[2]-textureCoordinates[0]);
            uniforms.texRange[surfacenum][1] = (textureCoordinates[3]-textureCoordinates[1]);
            uniforms.texOffset[surfacenum][0] = 1 - textureCoordinates[2];
            uniforms.texOffset[surfacenum][1] = 1 - textureCoordinates[3];
            break;
        case 3:
            uniforms.texRange[surfacenum][0] = (textureCoordinates[3]-textureCoordinates[1]);
            uniforms.texRange[surfacenum][1] = (textureCoordinates[2]-textureCoordinates[0]);
            uniforms.texOffset[surfacenum][0] = 1 - textureCoordinates[3];
            uniforms.texOffset[surfacenum][1] = textureCoordinates[0];
            break;
        }

        uniforms.texUnit[surfacenum] = surfacenum;
    }

    if (blend && !needsBlending(surfaces))
    {
        // We were asked to blend, but it looks like we don't need to...
        blend = false;
    }

    Shader *shader = pickOptimizedShader(surfaces, blend);

    if (!shader)
    {
        LOG_WARNING("GLESGraphicsystem", "Could not find a shader to render surfaces with");
        return false;
    }

    // Set GL blend state (ignored for binary shaders)
    if (blend)
    {
        glEnable (GL_BLEND);
    }
    else
    {
        glDisable (GL_BLEND);
    }

    shader->use();

    /* load common uniforms */
    shader->loadCommonUniforms(uniforms, surfaces.size());

    /* update all custom defined uniforms */
    shader->loadUniforms();

    /* Bind all textures */
    surfacenum = 0;
    for (SurfaceListConstIterator surface = surfaces.begin(); surface != surfaces.end(); surface++, surfacenum++)
    {
        glActiveTexture(GL_TEXTURE0 + uniforms.texUnit[surfacenum]);
        if (false == m_binder->bindSurfaceTexture(*surface))
        {
            LOG_WARNING("GLESGraphicsystem", "Surface not successfully bound " << (*surface)->getID());
            return false;
        }

        /* Rotate texture vertex attribs, per-surface */
        int layerId = (*surface)->getContainingLayerId();
        Layer* layer = m_baseWindowSystem->m_pScene->getLayer(layerId);
        int effectiveOrientation = ((*surface)->getOrientation() + layer->getOrientation()) % 4;

        glVertexAttribPointer(surfacenum + 1, 2, GL_FLOAT, GL_FALSE, 0, (void*) ((1+(2*effectiveOrientation)) * sizeof(float) * 12));
    }

    /* Enable only the needed vertex attributes */
    for (unsigned int i = 1; i <= MAX_MULTI_SURFACE; i++)
    {
        if (i <= surfaces.size())
        {
            glEnableVertexAttribArray(i);
        }
        else
        {
            glDisableVertexAttribArray(i);
        }
    }

    /* Draw two triangles */
    glDrawArrays(GL_TRIANGLES, 0, 6);

    /* UnBind all textures */
    for (SurfaceListConstIterator surface = surfaces.begin(); surface != surfaces.end(); surface++)
    {
        if (false == m_binder->unbindSurfaceTexture(*surface))
        {
            LOG_WARNING("GLESGraphicsystem", "Surface not successfully un-bound " << (*surface)->getID());
            return false;
        }
    }

    glErrorCode = glGetError();
    if ( GL_NO_ERROR != glErrorCode )
    {
        LOG_ERROR("GLESGraphicsystem", "GL Error occured :" << glErrorCode );
        return false;
    };

    return true;
}

void GLESGraphicsystem::renderRegion(MultiSurfaceRegion* region, bool blend)
{
    // Render all surfaces in this region, performing as many rendering
    // passes as necessary, each with up to MAX_MULTI_SURFACE surfaces.
    SurfaceList surfaceBatch;
    for(SurfaceListConstIterator currentS = region->m_surfaces.begin(); currentS != region->m_surfaces.end(); currentS++)
    {
        surfaceBatch.push_back(*currentS);

        // Render when batch size reaches maximum, then start a new batch
        if (surfaceBatch.size() == MAX_MULTI_SURFACE)
        {
            if (!renderSurfaces(surfaceBatch, region->m_rect, blend))
            {
                LOG_DEBUG("GLESGraphicsystem", "Failed to render region (multi-pass/multi-texture)");
                return;
            }
            surfaceBatch.clear();
            blend = true; // Do blend for subsequent batches
        }
    }

    // Finish remaining work, take care of clear-color regions now also
    if (!blend || surfaceBatch.size() > 0)
    {
        if (!renderSurfaces(surfaceBatch, region->m_rect, blend))
        {
            LOG_DEBUG("GLESGraphicsystem", "Failed to render region (multi-pass/multi-texture)");
            return;
        }
    }
}

struct OrderedSurface {
    Surface *surface;
    int depth;
    OrderedSurface(Surface *s, int d):surface(s),depth(d) {}
    bool operator<(OrderedSurface rhs) const { return depth < rhs.depth; }
    bool operator==(OrderedSurface rhs) const { return surface == rhs.surface; }
};

struct RegionLimit {
    float value; // Location on either X or Y axis
    bool entry;  // 1=in, 0=out
    OrderedSurface orderedSurface;
    RegionLimit(float v, bool e, OrderedSurface s):value(v),entry(e),orderedSurface(s) {}
    bool operator<(RegionLimit rhs) const { return value < rhs.value; }
};

// Creates a list of non-overlapping screen-space regions, each with a list of
// surfaces that reside in that area of the screen.  If 'clear' is true, the
// list of screen regions are guaranteed to collectively cover the entire
// screen space (empty surface lists are used to indicate regions with only
// background color visible).  When 'clear' is false, the set of regions will
// only cover screen space occupied by these layers' surfaces.
std::list<MultiSurfaceRegion*> GLESGraphicsystem::computeRegions(std::list<Layer*> layers, bool clear)
{
    /*
     * Example: on a 10x6 screen, with two surfaces A,B
     *
     * Surface A = (1,0)-(9,3)
     * Surface B = (3,2)-(9,5)
     *
     * Sorted edge lists (note X=9 is listed twice):
     *     xlimits = 1 A(in), 3 B(in), 9 A(out), 9 B(out)
     *     ylimits = 0 A(in), 2 B(in), 3 A(out), 5 B(out)
     *
     *  X     1     3                 9
     * Y
     * 0     _ ________________________ _
     *     |  |                       |  |
     *     |  |           A           |  |
     *     |  |                       |  |
     * 2   | _|_ _ _ _________________|_ |
     *     |  |  A  |       A+B       |  |
     * 3   | _|_____|_________________|_ |
     *     |        |                 |  |
     *     |        |        B        |  |
     *     |        |                 |  |
     * 5   | _ _ _ _|_________________|_ |
     *     |                             |
     *     | _ _ _ _ _ _ _ _ _ _ _ _ _ _ |
     *
     * Without clear, the process will create 4 regions. 3 single surface regions
     * and one dual-surface region.
     *
     * With clear, the edges of the screen are also added into xlimits and ylimits,
     * so 11 regions are created. 3 single-surface, 1-dual-surface, and 7 no-surface.
     */
    std::list<MultiSurfaceRegion*> regions;
    std::vector<RegionLimit> xlimits;
    std::vector<RegionLimit> ylimits;
 
    // If clear is needed enable a dummy surface so that additional zero-surface
    // regions will be created around the edges to fill screen (when necessary)
    if (clear)
    {
        xlimits.push_back(RegionLimit(0,               true,  OrderedSurface(NULL, 0)));
        xlimits.push_back(RegionLimit(m_displayWidth,  false, OrderedSurface(NULL, 0)));
        ylimits.push_back(RegionLimit(0,               true,  OrderedSurface(NULL, 0)));
        ylimits.push_back(RegionLimit(m_displayHeight, false, OrderedSurface(NULL, 0)));
    }

    // Add the edges of the visible surfaces into xlimits and ylimits.  Each edge is flagged
    // as either an "IN" (left or top edge), or an "OUT" (right or bottom edge).  Layer depth is
    // also stored in the edge information.
    LayerListConstIterator layer;
    int depth = 0;
    for(layer = layers.begin(); layer != layers.end(); layer++)
    {
        if ((*layer)->visibility == false || (*layer)->getOpacity() <= 0.0f)
        {
            continue;
        }

        SurfaceList surfaces = (*layer)->getAllSurfaces();
        SurfaceListConstIterator surface;
        for(surface = surfaces.begin(); surface != surfaces.end(); surface++)
        {
            if (!(*surface)->hasNativeContent())
            {
                continue;
            }

            if ((*surface)->visibility == false || (*surface)->getOpacity() <= 0.0f)
            {
                continue;
            }

            depth++;

            FloatRectangle rect = (*surface)->getTargetDestinationRegion();

            xlimits.push_back(RegionLimit(rect.x,               true,  OrderedSurface((*surface), depth)));
            xlimits.push_back(RegionLimit(rect.x + rect.width,  false, OrderedSurface((*surface), depth)));
            ylimits.push_back(RegionLimit(rect.y,               true,  OrderedSurface((*surface), depth)));
            ylimits.push_back(RegionLimit(rect.y + rect.height, false, OrderedSurface((*surface), depth)));
        }
    }

    // Early exit if there were no visible surfaces
    if (xlimits.size() < 2 || ylimits.size() < 2)
    {
        return regions; //empty
    }

    // Sort the surface edges prior to traversal
    std::sort(xlimits.begin(), xlimits.end());
    std::sort(ylimits.begin(), ylimits.end());

    /* Create the regions by stepping through rows and columns, keeping
     * track of which surfaces are present using IN/OUT edge information.
     *
     * Foreach ylimit {
     *     If the ylimit defines a new row {
     *         Foreach xlimit {
     *             If the xlimit defines a new column {
     *                 If xlimit is for a surface not in "rowsurfaces", ignore it
     *                 Create new region containing "regionsurfaces"
     *                 Add the surface to the current "regionsurfaces" list if "IN"
     *                 Remove from the current "regionsurfaces" list if "OUT"
     *             }
     *         }
     *     }
     *
     *     Add the surface to the current "rowsurfaces" list if "IN"
     *     Remove from the current "rowsurfaces" list if "OUT"
     * }
     *
     * Care must be taken with duplicate xlimit or ylimit edges. Like in the
     * example, both A and B are OUT at X=9, so there are two xlimit values
     * with X=9.
     */

    // The set of surfaces which are present somewhere in the current row
    std::set<Surface*> rowsurfaces;

    int x1 = -1; // Left of current region
    int x2 = -1; // Right of current region
    int y1 = -1; // Top of current row/region
    int y2 = -1; // Bottom of current row/region

    // The first row on the screen starts at the very top
    y1 = ylimits[0].value;
    for (unsigned y = 0; y < ylimits.size(); y++)
    {
        // The current row is between y1 and y2
        y2 = ylimits[y].value;

        // When y1 and y2 are the same, this isn't a new row yet.  Do update
        // "rowsurfaces", but don't yet step across the row.
        if (y2 != y1)
        {
            // Process the new row.  The list of regionsurfaces starts empty
            std::list<OrderedSurface> regionsurfaces;

            // 1st region in the row starts at the far left
            x1 = xlimits[0].value;
            for (unsigned x = 0; x < xlimits.size(); x++)
            {
                // Skip x edges that don't pertain to surfaces within the
                // current row.  This coallesces rows into as few regions as
                // possible. Columns are not coallesced though.
                if (rowsurfaces.find(xlimits[x].orderedSurface.surface) == rowsurfaces.end())
                {
                    continue;
                }
                // If this x edge defines the right side of a new region,
                // create that region.
                x2 = xlimits[x].value;
                if (x2 != x1)
                {
                    // A new region is ready to be created.  It exists between
                    // (x1,y1)-(x2,y2) and has the surfaces which currently are
                    // in "regionsurfaces".
                    MultiSurfaceRegion *region = new MultiSurfaceRegion();
                    region->m_rect = FloatRectangle(x1, y1, x2-x1, y2-y1);
                    std::list<OrderedSurface>::iterator si;
                    for (si = regionsurfaces.begin(); si != regionsurfaces.end(); si++)
                    {
                        if ((*si).surface)
                        {
                            region->m_surfaces.push_back((*si).surface);
                        }
                    }
                    // Add the new region to the list we return.  Only create
                    // empty regions when 'clear' was requested
                    if (clear || regionsurfaces.size() > 0)
                    {
                        regions.push_back(region);
                    }
                    // The right side of this region becomes the left side of
                    // the next region
                    x1 = x2;
                }

                // Update the list of surfaces in the current region
                if (xlimits[x].entry)
                {
                    // This xlimit is the left side of a surface, add it to
                    // the list.  Keep the surfaces in this list sorted by
                    // depth so when the region object is created, it will
                    // be able to render with the surfaces layered properly.
                    bool inserted = false;
                    std::list<OrderedSurface>::iterator si;
                    for (si = regionsurfaces.begin(); si != regionsurfaces.end(); si++)
                    {
                        if ((*si).depth >= xlimits[x].orderedSurface.depth)
                        {
                            regionsurfaces.insert(si, xlimits[x].orderedSurface);
                            inserted = true;
                            break;
                        }
                    }
                    if (!inserted)
                    {
                        regionsurfaces.push_back(xlimits[x].orderedSurface);
                    }
                }
                else
                {
                    // This xlimit is the right side of a surface, remove it
                    // from the list
                    regionsurfaces.remove(xlimits[x].orderedSurface);
                }
            }
            // The bottom of this row becomes the top of the next row
            y1 = y2;
        }

        // Update the list of surfaces which are present in this current row
        if (ylimits[y].entry)
        {
            rowsurfaces.insert(ylimits[y].orderedSurface.surface);
        }
        else
        {
            rowsurfaces.erase(ylimits[y].orderedSurface.surface);
        }
    }

    return regions;
}

bool GLESGraphicsystem::initOpenGLES(EGLint displayWidth, EGLint displayHeight)
{
    LOG_DEBUG("GLESGraphicsystem", "initEGL");
    bool result = true;
    ShaderProgramFactory::setCreatorFunc(m_shaderCreatorFunc);
    m_defaultShaderClear = Shader::createShader("default", "default_clear");
    m_defaultShader = Shader::createShader("default", "default");
    m_defaultShaderNoBlend = Shader::createShader("default", "default_no_blend");
    m_defaultShaderNoUniformAlpha = Shader::createShader("default", "default_no_uniform_alpha");
    m_defaultShaderAddUniformChromaKey= Shader::createShader("default", "default_add_uniform_chromakey");
    m_defaultShaderNoUniformAlphaNoBlend = Shader::createShader("default", "default_no_blend_no_uniform_alpha");
    m_defaultShader2surf = Shader::createShader("default_2surf", "default_2surf");
    m_defaultShader2surfNoBlend = Shader::createShader("default_2surf", "default_2surf_no_blend");
    m_defaultShader2surfNoUniformAlpha = Shader::createShader("default_2surf", "default_2surf_no_uniform_alpha");
    m_defaultShader2surfNoUniformAlphaNoBlend = Shader::createShader("default_2surf", "default_2surf_no_blend_no_uniform_alpha");
    m_defaultShader2surfNoUniformAlpha0 = Shader::createShader("default_2surf", "default_2surf_no_uniform_alpha_0");
    m_defaultShader2surfNoUniformAlpha0NoBlend = Shader::createShader("default_2surf", "default_2surf_no_blend_no_uniform_alpha_0");
    m_defaultShader2surfNoUniformAlpha1 = Shader::createShader("default_2surf", "default_2surf_no_uniform_alpha_1");
    m_defaultShader2surfNoUniformAlpha1NoBlend = Shader::createShader("default_2surf", "default_2surf_no_blend_no_uniform_alpha_1");

    if ((!m_defaultShaderClear) ||
        (!m_defaultShaderAddUniformChromaKey) ||
        (!m_defaultShader || !m_defaultShaderNoBlend) ||
        (!m_defaultShaderNoUniformAlpha || !m_defaultShaderNoUniformAlphaNoBlend) ||
        (!m_defaultShader2surf || !m_defaultShader2surfNoBlend) ||
        (!m_defaultShader2surfNoUniformAlpha || !m_defaultShader2surfNoUniformAlphaNoBlend) ||
        (!m_defaultShader2surfNoUniformAlpha0 || !m_defaultShader2surfNoUniformAlpha0NoBlend) ||
        (!m_defaultShader2surfNoUniformAlpha1 || !m_defaultShader2surfNoUniformAlpha1NoBlend))
    {
        LOG_ERROR("GLESGraphicsystem", "Failed to create and link default shader program");
        delete m_defaultShader;
        result = false;
    }
    else
    {
        LOG_INFO("GLESGraphicsystem", "Default Shader successfully applied");
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) (sizeof(float) * 12));
        glEnableVertexAttribArray(1);  // Enables only single texture initially

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        m_blendingStatus = true;
        glClearColor(0.0, 0.0, 0.0, 0.0);
        resize(displayWidth, displayHeight);
        glActiveTexture(GL_TEXTURE0);
    }

    //   Build Shader Map
    //  --------------------------------------------
    //   #: Number of Textures
    //   B: Blends with framebuffer
    //   T: Texture n has layer/surface transparency (layerAlpha != 1 or surfaceAlpha != 1)
    //   A: Texture n has alpha channel
    //   C: Texture n has chromakey
    //                         0      1      2      3
    //                       -----  -----  -----  -----
    //                  # B  T A C  T A C  T A C  T A C
    m_shaders[shaderKey(0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0)] = m_defaultShaderClear;
    m_shaders[shaderKey(1,1, 1,1,0, 0,0,0, 0,0,0, 0,0,0)] = m_defaultShader;
    m_shaders[shaderKey(1,1, 1,1,1, 0,0,0, 0,0,0, 0,0,0)] = m_defaultShaderAddUniformChromaKey;
    m_shaders[shaderKey(1,0, 1,1,0, 0,0,0, 0,0,0, 0,0,0)] = m_defaultShaderNoBlend;
    m_shaders[shaderKey(1,1, 0,1,0, 0,0,0, 0,0,0, 0,0,0)] = m_defaultShaderNoUniformAlpha;
    m_shaders[shaderKey(1,0, 0,1,0, 0,0,0, 0,0,0, 0,0,0)] = m_defaultShaderNoUniformAlphaNoBlend;

    m_shaders[shaderKey(2,1, 1,1,0, 1,1,0, 0,0,0, 0,0,0)] = m_defaultShader2surf;
    m_shaders[shaderKey(2,0, 1,1,0, 1,1,0, 0,0,0, 0,0,0)] = m_defaultShader2surfNoBlend;
    m_shaders[shaderKey(2,1, 0,1,0, 0,1,0, 0,0,0, 0,0,0)] = m_defaultShader2surfNoUniformAlpha;
    m_shaders[shaderKey(2,0, 0,1,0, 0,1,0, 0,0,0, 0,0,0)] = m_defaultShader2surfNoUniformAlphaNoBlend;
    m_shaders[shaderKey(2,1, 0,1,0, 1,1,0, 0,0,0, 0,0,0)] = m_defaultShader2surfNoUniformAlpha0;
    m_shaders[shaderKey(2,0, 0,1,0, 1,1,0, 0,0,0, 0,0,0)] = m_defaultShader2surfNoUniformAlpha0NoBlend;
    m_shaders[shaderKey(2,1, 1,1,0, 0,1,0, 0,0,0, 0,0,0)] = m_defaultShader2surfNoUniformAlpha1;
    m_shaders[shaderKey(2,0, 1,1,0, 0,1,0, 0,0,0, 0,0,0)] = m_defaultShader2surfNoUniformAlpha1NoBlend;

    return result;
}

void GLESGraphicsystem::resize(EGLint displayWidth, EGLint displayHeight)
{
    m_displayWidth = displayWidth;
    m_displayHeight = displayHeight;
    glViewport(0, 0, m_displayWidth, m_displayHeight);
}

void GLESGraphicsystem::saveScreenShotOfFramebuffer(std::string fileToSave)
{
    // clear error if any
    int error = glGetError();
    LOG_DEBUG("GLESGraphicSystem","taking screenshot and saving it to:" << fileToSave);

    LOG_DEBUG("GLESGraphicSystem","Screenshot: " << m_displayWidth << " * " << m_displayHeight);
    char *buffer = (char *) malloc( m_displayWidth * m_displayHeight * 4 * sizeof(char));
    glReadPixels(0, 0, m_displayWidth, m_displayHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        LOG_DEBUG("GLESGraphicSystem","error reading pixels for screenshot: " << error);
    }
    // convert to RGB for bitmap
    int pixelcount = m_displayHeight * m_displayWidth;
    char *rgbbuffer = (char *) malloc(pixelcount * 3 * sizeof(char));
    for (int row = 0; row < m_displayHeight; row++)
    {
        for (int col = 0; col < m_displayWidth; col++)
        {
            int offset = row * m_displayWidth + col;
            rgbbuffer[offset * 3] = buffer[offset * 4 + 2];
            rgbbuffer[offset * 3 + 1] = buffer[offset * 4 + 1];
            rgbbuffer[offset * 3 + 2] = buffer[offset * 4];
        }
    }

    writeBitmap(fileToSave, rgbbuffer, m_displayWidth, m_displayHeight);
    free(buffer);
    free(rgbbuffer);
}

bool GLESGraphicsystem::setupTextureForChromaKey()
{
    createPbufferSurface();
    if (m_eglPbufferSurface == EGL_NO_SURFACE)
    {
        return false;
    }

    // Switch the current context to pbuffer surface
    eglMakeCurrent(m_eglDisplay, m_eglPbufferSurface, m_eglPbufferSurface, m_eglContext);
    createTempTexture();
    return true;
}

void GLESGraphicsystem::createPbufferSurface()
{
    if (m_eglPbufferSurface != EGL_NO_SURFACE)
    {
        LOG_DEBUG("GLESGraphicsystem", "m_eglPbufferSurface is alread created");
        return;
    }

    const FloatRectangle layerDestRegion = m_currentLayer->getDestinationRegion();
    EGLint width  = static_cast<EGLint>(layerDestRegion.width);
    EGLint height = static_cast<EGLint>(layerDestRegion.height);

    EGLint pb_attrs[] = {
        EGL_WIDTH,  width,
        EGL_HEIGHT, height,
        EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
        EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
        EGL_NONE
    };

    m_eglPbufferSurface = eglCreatePbufferSurface(m_eglDisplay, m_eglConfig, pb_attrs);
    if (m_eglPbufferSurface == EGL_NO_SURFACE)
    {
        LOG_ERROR("GLESGraphicsystem", "Failed to create EGL pbuffer: " << eglGetError());
        return;
    }
}

void GLESGraphicsystem::createTempTexture()
{
    glGenTextures(1, &m_texId);
    glBindTexture(GL_TEXTURE_2D, m_texId);

    if (!eglBindTexImage(m_eglDisplay, m_eglPbufferSurface, EGL_BACK_BUFFER))
    {
        LOG_ERROR("GLESGraphicsystem", "Failed to bind texture for chroma key layer");
        glDeleteTextures(1, &m_texId);
        return;
    }

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void GLESGraphicsystem::destroyTempTexture()
{
    if (m_texId > 0)
    {
        glDeleteTextures(1, &m_texId);
    }

    if (m_eglPbufferSurface != EGL_NO_SURFACE)
    {
        eglReleaseTexImage(m_eglDisplay, m_eglPbufferSurface, EGL_BACK_BUFFER);
        eglDestroySurface(m_eglDisplay, m_eglPbufferSurface);
    }

    m_texId = 0;
    m_eglPbufferSurface = EGL_NO_SURFACE;
}

void GLESGraphicsystem::renderTempTexture()
{
    // TODO FIX IT , IS NOT ALREADY WORKING ANYMORE WITH MULTITEXTURE OPTIMIZATION, IF WE HAVE CHROMAKEY LAYER    
    const FloatRectangle layerSourceRegion      = m_currentLayer->getSourceRegion();
    const FloatRectangle layerDestinationRegion = m_currentLayer->getDestinationRegion();
    float textureCoordinates[4];

    ViewportTransform::transformRectangleToTextureCoordinates(
        layerSourceRegion,
        m_currentLayer->OriginalSourceWidth,
        m_currentLayer->OriginalSourceHeight,
        textureCoordinates);

    ShaderProgram::CommonUniforms uniforms;
    IlmMatrix layerMatrix;
    IlmMatrixIdentity(layerMatrix);

    uniforms.x = layerDestinationRegion.x / m_displayWidth;
    uniforms.y = 1.0f - (layerDestinationRegion.y + layerDestinationRegion.height) / m_displayHeight;
    uniforms.width   = layerDestinationRegion.width  / m_displayWidth;
    uniforms.height  = layerDestinationRegion.height / m_displayHeight;
    uniforms.opacity[0] = m_currentLayer->getOpacity();
    uniforms.texRange[0][0]  = textureCoordinates[2] - textureCoordinates[0];
    uniforms.texRange[0][1]  = textureCoordinates[3] - textureCoordinates[1];
    uniforms.texOffset[0][0] = textureCoordinates[0];
    uniforms.texOffset[0][1] = textureCoordinates[1];
    uniforms.texUnit[0] = 0;
    uniforms.matrix = layerMatrix.f;
    uniforms.chromaKeyEnabled = m_currentLayer->getChromaKeyEnabled();
    if (uniforms.chromaKeyEnabled == true) {
        unsigned char red   = 0;
        unsigned char green = 0;
        unsigned char blue  = 0;
        m_currentLayer->getChromaKey(red, green, blue);
        uniforms.chromaKey[0] = (float)red   / 255.0f;
        uniforms.chromaKey[1] = (float)green / 255.0f;
        uniforms.chromaKey[2] = (float)blue  / 255.0f;
    }

    glEnable (GL_BLEND);

    Shader* shader = m_currentLayer->getShader();
    if (!shader) {
        shader = m_defaultShader;
    }
    //shader = pickOptimizedShader(shader, uniforms);
    shader = m_defaultShaderAddUniformChromaKey;
    shader->use();
    shader->loadCommonUniforms(uniforms,0);
    shader->loadUniforms();

    glBindTexture(GL_TEXTURE_2D, m_texId);

    int orientation = m_currentLayer->getOrientation() % 4;
    GLint index     = orientation * 12;
    glDrawArrays(GL_TRIANGLES, index, 6);

    GLenum glErrorCode = glGetError();
    if ( GL_NO_ERROR != glErrorCode ) {
        LOG_ERROR("GLESGraphicsystem", "GL Error occured in renderTempTexture:" << glErrorCode );
    }
}
