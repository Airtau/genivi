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

#include "GraphicSystems/GLXGraphicsystem.h"
#include <string.h>
#include "TextureBinders/X11CopyGLX.h"
#include "TextureBinders/X11TextureFromPixmap.h"
#include "ViewportTransform.h"

#include "Bitmap.h"

GLXGraphicsystem::GLXGraphicsystem(int WindowWidth, int WindowHeight)
: m_windowWidth(WindowWidth)
, m_windowHeight(WindowHeight)
, m_x11display(0)
, m_window(0)
, m_context(0)
, m_currentLayer(0)
, m_zerocopy(false)

{
    LOG_DEBUG("GLXGraphicsystem", "creating GLXGraphicsystem");
}

GLXGraphicsystem::~GLXGraphicsystem()
{
    if (m_binder)
    {
        delete m_binder;
    }
}

XVisualInfo* GLXGraphicsystem::GetMatchingVisual(Display *dpy)
{
    int screen = DefaultScreen(dpy);
    XVisualInfo *visinfo;
    int attribs[] = {
            GLX_RGBA,
            GLX_ALPHA_SIZE,8,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            GLX_DEPTH_SIZE,8,
            GLX_BUFFER_SIZE,32,
            GLX_DOUBLEBUFFER,
            None
    };

    visinfo = glXChooseVisual(dpy, screen, attribs);
    if (!visinfo)
    {
        LOG_ERROR("GLXGraphicsystem", "Unable to find RGB, double-buffered visual");
    }
    return visinfo;
}
bool GLXGraphicsystem::CheckConfigMask(Display *curDisplay,GLXFBConfig currentConfig, int attribute, int expectedValue)
{
    bool result = true;
    int returnedValue = 0;

    glXGetFBConfigAttrib(curDisplay,currentConfig,attribute,&returnedValue);
    if (!(returnedValue & expectedValue))
    {
        result = false;
    }
    return result;
}

bool GLXGraphicsystem::CheckConfigValue(Display *curDisplay,GLXFBConfig currentConfig, int attribute, int expectedValue)
{
    bool result = true;
    int returnedValue = 0;

    glXGetFBConfigAttrib(curDisplay,currentConfig,attribute,&returnedValue);
    if ((returnedValue != expectedValue))
    {
        result = false;
    }
    return result;
}

void GLXGraphicsystem::activateGraphicContext()
{
    glXMakeCurrent(m_x11display, m_window, m_context);   
}

void GLXGraphicsystem::releaseGraphicContext() 
{
    glXMakeCurrent(m_x11display, None, NULL);
}

GLXFBConfig* GLXGraphicsystem::GetMatchingPixmapConfig(Display *curDisplay)
{
    int neededMaskAttribute[] =
    {
        GLX_DRAWABLE_TYPE,GLX_PIXMAP_BIT,
        GLX_DRAWABLE_TYPE,GLX_WINDOW_BIT,
        GLX_BIND_TO_TEXTURE_TARGETS_EXT,GLX_TEXTURE_2D_BIT_EXT,
        None
    };
    int neededValueAttribute[] =
    {
        GLX_BUFFER_SIZE,32,
        GLX_ALPHA_SIZE,8,
        GLX_BIND_TO_TEXTURE_RGBA_EXT,True,
        None
    };
    LOG_DEBUG("GLXGraphicsystem", "Choose pixmap GL configuration");
    int screen = DefaultScreen(curDisplay);
    GLXFBConfig *currentFBconfigs;
    int i = 0;
    int j = 0;
    int nConfigs = 0;

    currentFBconfigs = glXGetFBConfigs(curDisplay, screen, &nConfigs);
    for (i = 0; i < nConfigs; i++)
    {
        GLXFBConfig config = currentFBconfigs[i];
        bool result = true;
        /* check first all mask values */
        j = 0;
        while ( neededMaskAttribute[j] != None && result == true )
        {
           result = CheckConfigMask(curDisplay,config, neededMaskAttribute[j], neededMaskAttribute[j+1]);
           j += 2;
        }
        /* no matching found in needed mask attribute, skip config take next */
        if (result == false )
        {
            continue;
        }
        /* check all fixed values */

        /* reset attribute counter */
        j = 0;
        /* check all fixed values */
        while ( neededValueAttribute[j] != None && result == true )
        {
           result = CheckConfigValue(curDisplay,config, neededValueAttribute[j], neededValueAttribute[j+1]);
           j += 2;
        }
        /* no matching found in needed fixed value attribute, skip config take next */

        if (result == false )
        {
            continue;
        }
        break;
    }

    if (i == nConfigs)
    {
        LOG_ERROR("GLXGraphicsystem", "Unable to find FBconfig for texturing");
        return NULL;
    }

    LOG_DEBUG("GLXGraphicsystem", "Done choosing GL Pixmap configuration");
    return &currentFBconfigs[i];
}

bool GLXGraphicsystem::init(Display* x11Display, Window x11Window)
{
    LOG_DEBUG("GLXGraphicsystem", "init");
    m_x11display = x11Display;
    m_window = x11Window;

    if (!m_x11display)
    {
        LOG_ERROR("GLXGraphicsystem", "given display is null");
        return false;
    }

    if (!m_window)
    {
        LOG_ERROR("GLXGraphicsystem", "given windowid is 0");
        return false;
    }

    XVisualInfo* windowVis = GetMatchingVisual(m_x11display);

    LOG_DEBUG("GLXGraphicsystem", "Initialising opengl");
    m_context = glXCreateContext(m_x11display, windowVis, 0, GL_TRUE);
    if (!m_context)
    {
        LOG_ERROR("GLXGraphicsystem", "Couldn't create GLX context!");
        return false;
    }
    LOG_DEBUG("GLXGraphicsystem", "Make GLX Context current");
    glXMakeCurrent(m_x11display, m_window, m_context);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_MODELVIEW);
    const char *ext;
    ext = glXQueryExtensionsString(m_x11display, 0);
    if (!strstr(ext, "GLX_EXT_texture_from_pixmap") )
    {
        m_zerocopy = false;
    }
    else
    {
        m_zerocopy = true;
    }

    LOG_DEBUG("GLXGraphicsystem", "Initialised");
    return true;
}


void GLXGraphicsystem::clearBackground()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLXGraphicsystem::swapBuffers()
{
    glXSwapBuffers(m_x11display, m_window);
}

void GLXGraphicsystem::beginLayer(Layer* currentLayer)
{
    m_currentLayer = currentLayer;

    /* Load Identity Matrix for each Layer */
    glLoadIdentity();

    /* set layer Transformations */
/*    const Rectangle& layerDestination = m_currentLayer->getDestinationRegion(); */
    // TODO: unused? const Rectangle& layerSource = m_currentLayer->getSourceRegion();

/*    glTranslatef(layerDestination.x, layerDestination.y, 0.0); */
}

// Reports whether a single layer is damaged/dirty
// Can not account for possible occlusion by other layers
bool GLXGraphicsystem::needsRedraw(Layer *layer)
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
bool GLXGraphicsystem::needsRedraw(LayerList layers)
{
    // TODO: Ignore damage from completely obscured surfaces

    for (LayerListConstIterator layer = layers.begin(); layer != layers.end(); layer++)
    {
        if ((*layer)->getLayerType() == Hardware && layers.size() > 1)
        {
            // Damage in a hardware layer should not imply a redraw in other layers
            LOG_WARNING("GLXGraphicsystem", "needsRedraw() called with layers not in the same composition");
        }

        if (needsRedraw(*layer))
        {
            return true;
        }
    }
    return false;
}

void GLXGraphicsystem::renderSWLayer(Layer *layer, bool clear)
{
    if (clear)
    {
        clearBackground();
    }

    if ( layer->visibility && layer->opacity > 0.0 )
    {
        SurfaceList surfaces = layer->getAllSurfaces();
        beginLayer(layer);
        for(SurfaceListConstIterator currentS = surfaces.begin(); currentS != surfaces.end(); currentS++)
        {
            if ((*currentS)->hasNativeContent() && (*currentS)->visibility && (*currentS)->opacity>0.0f)
            {
                renderSurface(*currentS);
            }
        }
        endLayer();
    }
}

void GLXGraphicsystem::renderSWLayers(LayerList layers, bool clear)
{
    // This is a stub.
    //
    // TODO: render in a more optimal way
    //   1. Turn off blending for first surface rendered
    //   2. Don't clear when it's legal to avoid it
    //         eg. a fullscreen opaque surface exists
    //   3. Render multiple surfaces at time via multi-texturing
    //   4. Remove fully obscured layers/surfaces
    if (clear)
    {
        clearBackground();
    }

    for (LayerListConstIterator layer = layers.begin(); layer != layers.end(); layer++)
    {
        renderSWLayer(*layer, false); // Don't clear
    }
}

void GLXGraphicsystem::endLayer()
{
    m_currentLayer = NULL;
}

void GLXGraphicsystem::renderSurface(Surface* currentSurface)
{
//    LOG_DEBUG("GLXGraphicsystem", "renderSurface " << currentSurface->getID() );
    GLenum glErrorCode = GL_NO_ERROR;
    
    if (currentSurface->isCropped())
        return; // skip rendering of this surface, because it is cropped by layer source region
        
    // check if surface is cropped completely, if so then skip rendering
    FloatRectangle targetSurfaceSource = currentSurface->getTargetSourceRegion();
    FloatRectangle targetSurfaceDestination = currentSurface->getTargetDestinationRegion();

    float textureCoordinates[4];
    ViewportTransform::transformRectangleToTextureCoordinates(targetSurfaceSource, currentSurface->OriginalSourceWidth, currentSurface->OriginalSourceHeight, textureCoordinates);

    glPushMatrix();
    if (false == m_binder->bindSurfaceTexture(currentSurface))
    {
        /* skip render surface if not bind successfully */
        return;
    }
//    glPushMatrix();
    glColor4f(1.0f,1.0f,1.0f,currentSurface->opacity*(m_currentLayer)->opacity);

    glBegin(GL_QUADS);

//    LOG_DEBUG("GLXGraphicsystem","rendersurface: src" << src.x << " " << src.y << " " << src.width << " " << src.height );
//    LOG_DEBUG("GLXGraphicsystem","rendersurface: dest" << dest.x << " " << dest.y << " " << dest.width << " " << dest.height );
//    LOG_DEBUG("GLXGraphicsystem","orig: " << currentSurface->OriginalSourceWidth << " " << currentSurface->OriginalSourceHeight  );
//    LOG_DEBUG("GLXGraphicsystem","window: " << m_windowWidth << " " << m_windowHeight  );

    //bottom left
    glTexCoord2d(textureCoordinates[0],textureCoordinates[3]);
    glVertex2d((float)targetSurfaceDestination.x/m_windowWidth*2-1,  1-(float)(targetSurfaceDestination.y+targetSurfaceDestination.height)/m_windowHeight*2);

    // bottom right
    glTexCoord2f(textureCoordinates[2],textureCoordinates[3]);
    glVertex2d( (float)(targetSurfaceDestination.x+targetSurfaceDestination.width)/m_windowWidth*2-1, 1-(float)(targetSurfaceDestination.y+targetSurfaceDestination.height)/m_windowHeight*2);

    // top right
    glTexCoord2f(textureCoordinates[2], textureCoordinates[1]);
    glVertex2d((float)(targetSurfaceDestination.x+targetSurfaceDestination.width)/m_windowWidth*2-1, 1-(float)targetSurfaceDestination.y/m_windowHeight*2);

    // top left
    glTexCoord2f(textureCoordinates[0], textureCoordinates[1]);
    glVertex2d((float)targetSurfaceDestination.x/m_windowWidth*2-1 ,  1-(float)targetSurfaceDestination.y/m_windowHeight*2);
    glEnd();

    m_binder->unbindSurfaceTexture(currentSurface);
    glPopMatrix();
    glErrorCode = glGetError();
    if ( GL_NO_ERROR != glErrorCode )
    {
        LOG_ERROR("GLXGraphicsystem", "GL Error occured :" << glErrorCode );
    }
    currentSurface->frameCounter++;
    currentSurface->drawCounter++;
}

void GLXGraphicsystem::saveScreenShotOfFramebuffer(std::string fileToSave)
{
    LOG_DEBUG("GLXGraphicsystem","taking screenshot and saving it to:" << fileToSave);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport); // x,y,width,height

    int WINDOW_WIDTH= viewport[2];
    int WINDOW_HEIGHT= viewport[3];
    LOG_DEBUG("GLXGraphicsystem","Screenshot: " << WINDOW_WIDTH << " * " << WINDOW_HEIGHT);
    char *buffer = (char *)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 3 * sizeof(unsigned char));
    glReadPixels(0,0,WINDOW_WIDTH,WINDOW_HEIGHT,GL_BGR,GL_UNSIGNED_BYTE, buffer);

    writeBitmap(fileToSave,buffer,WINDOW_WIDTH,WINDOW_HEIGHT);
    free(buffer);
    LOG_DEBUG("GLXGraphicsystem","done taking screenshot");
}

