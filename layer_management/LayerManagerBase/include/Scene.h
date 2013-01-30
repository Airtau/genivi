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
#ifndef _SCENE_H_
#define _SCENE_H_

#include "IScene.h"
#include "Layermanager.h"
#include "CommandList.h"
#include "ShaderMap.h"
#include "SurfaceMap.h"
#include "LayerMap.h"
#include "ShaderMap.h"
#include "LayerList.h"
#include "LmScreenList.h"
#include <pthread.h>

class Layer;
class Surface;

/*
 * Represents a list of Layers which contain the Surfaces.
 */
class Scene: public IScene
{
    // TODO: these should use public interface instead
    friend class Layermanager;
    friend class CommitCommand;
    friend class ShaderCreateCommand;
    friend class ShaderDestroyCommand;
    friend class SurfaceSetShaderCommand;
    friend class ShaderSetUniformsCommand;

public:
    Scene();
    virtual ~Scene();

    virtual Layer* createLayer(const uint id, int creatorPid);
    virtual Surface *createSurface(const uint id, int creatorPid);

    virtual bool removeLayer(Layer* layer);
    virtual bool removeSurface(Surface* surface);
    virtual void removeSurfaceNativeContent(Surface* surface);
    virtual LmScreenList& getScreenList();
    virtual LmScreen* getScreen(const uint id) const;
    virtual Layer* getLayer(const uint id);
    virtual Surface* getSurface(const uint id);

    virtual void getLayerIDs(uint* length, uint** array) const;
    virtual bool getLayerIDsOfScreen(const uint screenID, uint* length, uint** array) const;
    virtual void getSurfaceIDs(uint* length, uint** array) const;

    virtual void lockScene();
    virtual void unlockScene();

    virtual LayerList& getCurrentRenderOrder(const uint id);
    virtual const SurfaceMap getAllSurfaces() const;
    virtual bool isLayerInCurrentRenderOrder(const uint id);

    const LayerMap getAllLayers() const;

private:
    ShaderMap m_shaderMap;
    pthread_mutex_t m_layerListMutex;
    SurfaceMap m_surfaceMap;
    LayerMap m_layerMap;
    LayerList m_nullRenderOrder;
    LmScreenList m_screenList;
};

inline const LayerMap Scene::getAllLayers() const
{
    return m_layerMap;
}

inline LayerList& Scene::getCurrentRenderOrder(const uint id) // TODO: const
{
    LmScreen* screen = Scene::getScreen(id);
    if (NULL != screen)
    {
        return screen->getCurrentRenderOrder();
    }
    else
    {
        return m_nullRenderOrder;
    }
}

inline LmScreenList& Scene::getScreenList()
{
    return m_screenList;
}

inline const SurfaceMap Scene::getAllSurfaces() const
{
    return m_surfaceMap;
}

inline void Scene::lockScene()
{
    pthread_mutex_lock(&m_layerListMutex);
}

inline void Scene::unlockScene()
{
    pthread_mutex_unlock(&m_layerListMutex);
}

#endif /* _SCENE_H_ */
