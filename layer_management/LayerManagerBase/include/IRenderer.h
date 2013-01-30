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

#ifndef _IRENDERER_H_
#define _IRENDERER_H_

#include <string>
#include "LayerType.h"
#include "Shader.h"
#include "OptimizationType.h"


class InputManager;


/**
 * Abstract Base of all CompositingControllers, ie Renderers.
 * \defgroup RendererAPI Layer Management Renderer API
 */
class IRenderer
{
public:
    /**
     * \brief      default destructor
     */
    virtual ~IRenderer()
    {
    }

    /**
     * \brief      Start the actual rendering process (render loop)
     * \ingroup    RendererAPI
     * \param[in]  width width of display handled by this renderer
     * \param[in]  height height of display handled by this renderer
     * \param[in]  displayName name of display handled by this renderer
     * \return     TRUE: renderer was started successfully
     * \return     FALSE: renderer start failed
     */
    virtual bool start(int width, int height, const char* displayName) = 0;

    /**
     * \brief      Stop rendering process (stop render loop)
     * \ingroup    RendererAPI
     */
    virtual void stop() = 0;

    /**
     * \brief      Switch debug mode of this component on or off
     * \ingroup    RendererAPI
     * \param[in]  onoff TRUE: Turn on debug mode, FALSE: Turn off debug mode
     */
    virtual void setdebug(bool onoff) = 0;

    /**
     * \brief      Store graphical content of screen to bitmap
     * \ingroup    RendererAPI
     * \param[in]  fileToSave path to bitmap file to store the graphical content
     */
    virtual void doScreenShot(std::string fileToSave) = 0;

    /**
     * \brief      Store graphical content of layer to bitmap
     * \ingroup    RendererAPI
     * \param[in]  fileToSave path to bitmap file to store the graphical content
     * \param[in]  id id of layer
     */
    virtual void doScreenShotOfLayer(std::string fileToSave, const unsigned int id) = 0;

    /**
     * \brief      Store graphical content of surface to bitmap
     * \ingroup    RendererAPI
     * \param[in]  fileToSave path to bitmap file to store the graphical content
     * \param[in]  id id of surface
     * \param[in]  layer_id id of layer
     */
    virtual void doScreenShotOfSurface(std::string fileToSave, const unsigned int id, const unsigned int layer_id) = 0;

    /**
     * \brief      Get the capabilies of a layer type
     * \ingroup    RendererAPI
     * \param[in]  layertype type of layer
     * \return     bitset with flags set for capabilities
     */
    virtual unsigned int getLayerTypeCapabilities(LayerType layertype) = 0;

    /**
     * \brief      Get the number of supported hardware layers of the renderer for a screen
     * \ingroup    RendererAPI
     * \param[in]  screenID id of the screen
     * \return     Number of supported hardware layers for screen
     */
    virtual unsigned int getNumberOfHardwareLayers(unsigned int screenID) = 0;

    /**
     * \brief      Get the resolution of a screen handled by this renderer
     * \ingroup    RendererAPI
     * \param[in]  screenID id of the screen
     * \return     array with width and height of screen
     */
    virtual unsigned int* getScreenResolution(unsigned int screenID) = 0;

    /**
     * \brief      Get the list if available screen ids
     * \ingroup    RendererAPI
     * \param[out] length length of the returned array
     * \return     array containing all available screen ids
     */
    virtual unsigned int* getScreenIDs(unsigned int* length) = 0;

    /**
     * \brief      Create a shader object (that can be applied to surfaces)
     * \ingroup    RendererAPI
     * \param[in]  vertexName filename of vertex shader source code
     * \param[in]  fragmentName filename of fragment shader source code
     * \return     Pointer to created shader object
     */
    virtual Shader* createShader(const string* vertexName, const string* fragmentName) = 0;

    /**
     * \brief      Trigger a redraw for this renderer
     * \ingroup    RendererAPI
     */
    virtual void signalWindowSystemRedraw() = 0;

    /**
     * \brief      Force composition for entire scene
     * \ingroup    RendererAPI
     */
    virtual void forceCompositionWindowSystem() = 0;

    /**
      * \brief      Get the InputManager associated to the Scene
      * \ingroup    RendererAPI
      */
    virtual InputManager* getInputManager() const = 0;

    /**
     * \brief      Set the mode for the specified optimization (e.g. OFF,ON,AUTO)
     * \ingroup    RendererAPI
     * \param[in]  id id of optimization
     * \param[in] mode mode to set for the optimization
     * \return     TRUE: id and mode are valid and mode was set
     * \return     FALSE: id or mode was invalid and/or mode could not be set
     */
    virtual bool setOptimizationMode(OptimizationType id, OptimizationModeType mode) = 0;

    /**
     * \brief      Get the current mode for the specified optimization
     * \ingroup    RendererAPI
     * \param[in]  id id of optimization
     * \param[out] mode retrieved mode value
     * \return     TRUE: id is valid and mode was returned
     * \return     FALSE: id was invalid and/or mode was not returned
     */
    virtual bool getOptimizationMode(OptimizationType id, OptimizationModeType *mode) = 0;
};

#endif /* _IRENDERER_H_ */

