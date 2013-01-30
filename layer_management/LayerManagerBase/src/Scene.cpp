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

#include "LayerList.h"
#include "Layermanager.h"
#include "Log.h"

#include "Layer.h"
#include "Surface.h"
#include "Shader.h"
#include "Scene.h"

Scene::Scene()
{
    pthread_mutex_init(&m_layerListMutex, NULL);
}

Scene::~Scene()
{
    pthread_mutex_destroy(&m_layerListMutex);
}

Layer* Scene::createLayer(const uint layerId, int creatorPid)
{
    Layer* newLayer = NULL;
    if (layerId == GraphicalObject::INVALID_ID)
    {
        LOG_DEBUG("Scene", "createLayer: creating new layer");
        newLayer = new Layer(creatorPid);
        uint newLayerId = newLayer->getID();
        m_layerMap[newLayerId] = newLayer;
    }
    else
    {
        if (0 == m_layerMap.count(layerId))
        {
            LOG_DEBUG("Scene", "createLayer: unknown layer id provided; creating new layer");
            newLayer = new Layer(layerId, creatorPid);
            uint newLayerId = newLayer->getID();
            m_layerMap[newLayerId] = newLayer;
        }
        else
        {
            LOG_DEBUG("Scene","createLayer: Layer with id " << layerId << " already exists ");
            newLayer = m_layerMap[layerId];
        }
    }
    return newLayer;
}

Surface* Scene::createSurface(const uint surfaceId, int creatorPid)
{
    Surface* newSurface = NULL;
    if (surfaceId == GraphicalObject::INVALID_ID)
    {
        newSurface = new Surface(creatorPid);
        uint newSurfaceId = newSurface->getID();
        m_surfaceMap[newSurfaceId] = newSurface;
    }
    else
    {
        if (0 == m_surfaceMap.count(surfaceId))
        {
            newSurface = new Surface(surfaceId, creatorPid);
            uint newSurfaceId = newSurface->getID();
            m_surfaceMap[newSurfaceId] = newSurface;
        }
        else
        {
            newSurface = m_surfaceMap[surfaceId];
            LOG_DEBUG("Scene","Surface with id [ " << surfaceId << " ] " << " already exists " );
        }
    }
    return newSurface;
}

LmScreen* Scene::getScreen(const uint screenId) const
{
    LmScreenListConstIterator iter = m_screenList.begin();
    LmScreenListConstIterator iterEnd = m_screenList.end();
    for (; iter != iterEnd; ++iter)
    {
        if (screenId == (*iter)->getID())
        {
            return (*iter);
        }
    }
    LOG_WARNING("Scene","screen not found : id [ " << screenId << " ]");

    return NULL;
}

Layer* Scene::getLayer(const uint layerId)
{
    Layer* layer = NULL;
    if (m_layerMap.count(layerId) > 0)
    {
        layer = m_layerMap[layerId];
    }
    else
    {
        LOG_WARNING("Scene","layer not found : id [ " << layerId << " ]");
    }
    return layer;
}

Surface* Scene::getSurface(const uint surfaceId)
{
    Surface* surface = NULL;
    if (m_surfaceMap.count(surfaceId) > 0)
    {
        surface = m_surfaceMap[surfaceId];
    }
    else
    {
        LOG_WARNING("Scene","surface not found : id [ " << surfaceId << " ]");
    }
    return surface;
}

/// \brief take layer out of list of layers
bool Scene::removeLayer(Layer* layer)
{
    bool result = false;
    LmScreenListIterator iter = m_screenList.begin();
    LmScreenListIterator iterEnd = m_screenList.end();

    if (layer != NULL)
    {
        result = isLayerInCurrentRenderOrder(layer->getID());
        layer->removeAllSurfaces();
        for (; iter != iterEnd; ++iter)
        {
            if (NULL == *iter)
            {
                LOG_WARNING("Scene", "screen invalid");
                continue;
            }
            (*iter)->getCurrentRenderOrder().remove(layer);
        }
        m_layerMap.erase(layer->getID());
        delete layer;
    }

    return result;
}

/// \brief take surface out of list of surfaces
bool Scene::removeSurface(Surface* surface)
{
    bool result = false;

    if (surface != NULL)
    {
        uint surfaceId = surface->getID();
        uint layerId = surface->getContainingLayerId();

        if (layerId != GraphicalObject::INVALID_ID)
        {
            Layer* layer = getLayer(layerId);
            if (layer != NULL)
            {
                result = layer->removeSurface(surface);
            }
        }

        m_surfaceMap.erase(surfaceId);
        delete surface;
    }

    return result;
}

/// \brief take removing applied native content
void Scene::removeSurfaceNativeContent(Surface* surface)
{
    if (surface != NULL)
    {
        surface->removeNativeContent();
    }
}

void Scene::getLayerIDs(uint* length, uint** array) const
{
    uint numOfLayers = m_layerMap.size();
    uint arrayPos = 0;

    *length = numOfLayers;
    *array = new uint[numOfLayers]; // TODO: safe, if size = 0?

    LayerMapConstIterator iter = m_layerMap.begin();
    LayerMapConstIterator iterEnd = m_layerMap.end();

    for (; iter != iterEnd; ++iter)
    {
        (*array)[arrayPos] = (*iter).first;
        arrayPos++;
    }
}

bool Scene::getLayerIDsOfScreen(const uint screenID, uint* length,
        uint** array) const
{
    // check if screen is valid, currently all layers are only on one screen
    LmScreen* screen = NULL;
    uint numOfLayers = 0;
    uint arrayPos = 0;
    LayerList currentRenderOrder;

    screen = getScreen(screenID);
    if (NULL == screen)
    {
        return false;
    }

    currentRenderOrder = screen->getCurrentRenderOrder();
    numOfLayers = currentRenderOrder.size();

    *length = numOfLayers;
    *array = new uint[numOfLayers]; // TODO: safe, if size = 0?

    LayerListConstIterator iter = currentRenderOrder.begin();
    LayerListConstIterator iterEnd = currentRenderOrder.end();

    for (; iter != iterEnd; ++iter)
    {
        (*array)[arrayPos] = (*iter)->getID();
        ++arrayPos;
    }
    return true;
}

void Scene::getSurfaceIDs(uint* length, uint** array) const
{
    uint numOfSurfaces = m_surfaceMap.size();
    uint arrayPos = 0;

    *length = numOfSurfaces;
    *array = new uint[numOfSurfaces]; // TODO: safe, if size = 0?

    SurfaceMapConstIterator iter = m_surfaceMap.begin();
    SurfaceMapConstIterator iterEnd = m_surfaceMap.end();

    for (; iter != iterEnd; ++iter)
    {
        (*array)[arrayPos] = (*iter).first;
        ++arrayPos;
    }
}

bool Scene::isLayerInCurrentRenderOrder(const uint id)
{
    LmScreenListIterator iterScreen = m_screenList.begin();
    LmScreenListIterator iterScreenEnd = m_screenList.end();
    LayerList currentRenderOrder;
    LayerListIterator iterLayer;
    LayerListIterator iterLayerEnd;

    for (; iterScreen != iterScreenEnd; ++iterScreen)
    {
        currentRenderOrder = (*iterScreen)->getCurrentRenderOrder();

        iterLayer = currentRenderOrder.begin();
        iterLayerEnd = currentRenderOrder.end();
        for (; iterLayer != iterLayerEnd; ++iterLayer)
        {
            if (id == (*iterLayer)->getID())
            {
                return true;
            }
        }
    }
    return false;
}
