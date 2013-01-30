/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#ifndef _ISCENE_H_
#define _ISCENE_H_

#include <semaphore.h>
#include "Layer.h"
#include "Surface.h"
#include "LayerList.h"
#include "SurfaceMap.h"
#include "LmScreen.h"
#include "LmScreenList.h"

/**
 * \defgroup SceneAPI Layer Management Scene API
 */

/**
 * \brief Represents the scene, which is a tree of scene objects
 *
 * Represents a scene with screens which have Layers which contain the Surfaces.
 * Sorting is based upon z-order of the contained layers.
 */
class IScene
{
public:
    /**
     * \brief default destructor
     * \ingroup SceneAPI
     */
    virtual ~IScene() {}

    /**
     * \brief Creates a new layer within the scene.
     * \ingroup SceneAPI
     * \param[in] id id of layer
     * \param[in] creatorPid client process id that requested the creation of this layer
     * \return pointer to layer
     */
    virtual Layer* createLayer(const uint id, int creatorPid) = 0;

    /**
     * \brief Creates a new surface within the scene.
     * \ingroup SceneAPI
     * \param[in] id id of surface
     * \param[in] creatorPid client process id that requested the creation of this surface
     * \return pointer to surface
     */
    virtual Surface* createSurface(const uint id, int creatorPid) = 0;

    /**
     * \brief Remove a layer from the scene.
     * \ingroup SceneAPI
     * \param[in] layer pointer to layer
     */
    virtual bool removeLayer(Layer* layer) = 0;

    /**
     * \brief Remove surface from scene.
     * \ingroup SceneAPI
     * \param[in] surface pointer to surface
     */
    virtual bool removeSurface(Surface* surface) = 0;

    /**
     * \brief Get a screen of the scene by id.
     * \ingroup SceneAPI
     * \param[in] id id of the screen
     * \return pointer to the screen with id
     */
    virtual LmScreen* getScreen(const uint id) const = 0;

    /**
     * \brief Get a layer of the scene by id.
     * \ingroup SceneAPI
     * \param[in] id id of the layer
     * \return pointer to the layer with id
     */
    virtual Layer* getLayer(const uint id) = 0;

    /**
     * \brief Get a surface of the scene by id.
     * \ingroup SceneAPI
     * \param[in] id id of the surface
     * \return pointer to the surface with id
     */
    virtual Surface* getSurface(const uint id) = 0;

    /**
     * \brief Get list of ids of all layers currently existing.
     * \ingroup SceneAPI
     * \param[out] length length of array returned in array
     * \param[out] array array containing the ids of all layers
     * \return list of ids of all currently know layers
     */
    virtual void getLayerIDs(uint* length, uint** array) const = 0;

    /**
     * \brief Get list of ids of all layers currently existing.
     * \ingroup SceneAPI
     * \param[in] screenID id of screen
     * \param[out] length length of array returned in array
     * \param[out] array array containing the ids of all layers on screen
     * \return list of ids of all currently know layers
     */
    virtual bool getLayerIDsOfScreen(const uint screenID, uint* length, uint** array) const = 0;

    /**
     * \brief Get list of ids of all surfaces currently existing.
     * \ingroup SceneAPI
     * \param[out] length length of array returned in array
     * \param[out] array array containing the ids of all surfaces
     * \return list of ids of all currently know surfaces
     */
    virtual void getSurfaceIDs(uint* length, uint** array) const = 0;

    /**
     * \brief Lock the list for read and write access
     * \ingroup SceneAPI
     */
    virtual void lockScene() = 0;

    /**
     * \brief Unlock the list for read and write access
     * \ingroup SceneAPI
     */
    virtual void unlockScene() = 0;

    /**
     * \brief Get the current render order of the scene.
     * \ingroup SceneAPI
     * \param[in] id screen id
     * \return reference to render order
     */
    virtual LayerList& getCurrentRenderOrder(const uint id) = 0;

    /**
     * \brief Get the screen list of the scene.
     * \ingroup SceneAPI
     * \return reference to screen list
     */
    virtual LmScreenList& getScreenList() = 0;

    /**
     * \brief Get a map of all surface from the scene.
     * \ingroup SceneAPI
     * \return Map holding all surfaces.
     * \todo return SurfaceMap& instead?
     */
    virtual const SurfaceMap getAllSurfaces() const = 0;

    /**
     * \brief Check, if layer is in render order.
     * \ingroup SceneAPI
     * \param[in] id layer id
     * \return TRUE: layer is in render order
     * \return FALSE: layer is not in render order
     */
    virtual bool isLayerInCurrentRenderOrder(const uint id) = 0;

    bool debugMode;

};

#endif /* _ISCENE_H_ */
