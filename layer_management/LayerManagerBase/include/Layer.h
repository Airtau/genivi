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

#ifndef _LAYER_H_
#define _LAYER_H_

#include "SurfaceList.h"
#include "Surface.h"
#include "GraphicalSurface.h"
#include "LayerType.h"
#include "LayerCapabilities.h"
#include "SurfaceList.h"

/*
 * A graphical Layerobject which contains Surfaces.
 */
class Layer: public GraphicalSurface
{
    friend class Scene;

public:
    Layer(int creatorPid);
    Layer(int id, int creatorPid);

    virtual ~Layer() {}

    LayerType getLayerType() const;
    bool setLayerType(LayerType lt);
    bool setLayerCapabilities(unsigned int newCapabilities);
    unsigned int getCapabilities() const;
    bool addSurface(Surface* s);
    bool removeSurface(Surface* s);
    void applySurfaceTransform();
    SurfaceList& getAllSurfaces();
    bool removeAllSurfaces();
    unsigned int getContainingScreenId();
    void setContainingScreenId(unsigned int screenid);

private:
    unsigned int m_screenId;
    SurfaceList m_surfaces;
    LayerType m_layerType;
    uint m_capabilities;
};

inline Layer::Layer(int creatorPid)
: GraphicalSurface(TypeLayer, creatorPid)
, m_screenId(INVALID_ID)
, m_layerType(Software_2D)
, m_capabilities(0)
{
}

inline Layer::Layer(int id, int creatorPid)
: GraphicalSurface(id, TypeLayer, creatorPid)
, m_screenId(INVALID_ID)
, m_layerType(Software_2D)
, m_capabilities(0)
{
}

inline LayerType Layer::getLayerType() const
{
    return m_layerType;
}

inline bool Layer::setLayerType(LayerType lt)
{
    if (m_layerType != lt)
    {
        m_layerType = lt;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline bool Layer::setLayerCapabilities(uint newCapabilities)
{
    if (m_capabilities != newCapabilities)
    {
        m_capabilities = newCapabilities;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline uint Layer::getCapabilities() const
{
    return m_capabilities;
}

inline bool Layer::addSurface(Surface* s)
{
    if (s->getContainingLayerId() == INVALID_ID)
    {
        m_surfaces.push_back(s);
        s->setContainingLayerId(getID());
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline bool Layer::removeSurface(Surface* s)
{
    if (s->getContainingLayerId() == getID())
    {
        m_surfaces.remove(s);
        s->setContainingLayerId(INVALID_ID);
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline SurfaceList& Layer::getAllSurfaces()
{
    return m_surfaces;
}

inline bool Layer::removeAllSurfaces()
{
    SurfaceListConstIterator iter = m_surfaces.begin();
    SurfaceListConstIterator iterEnd = m_surfaces.end();

    if(iter == iterEnd)
        return false;

    for (; iter != iterEnd; ++iter)
    {
        (*iter)->setContainingLayerId(GraphicalObject::INVALID_ID);
    }
    m_surfaces.clear();
    renderPropertyChanged = true;
    return true;
}

inline unsigned int Layer::getContainingScreenId()
{
    return m_screenId;
}

inline void Layer::setContainingScreenId(unsigned int id)
{
    m_screenId = id;
}

inline void Layer::applySurfaceTransform()
{
	SurfaceListConstIterator iter = m_surfaces.begin();
	SurfaceListConstIterator iterEnd = m_surfaces.end();
	if(iter != iterEnd) 
	{
		for (; iter != iterEnd; ++iter)
		{
			(*iter)->calculateTargetDestination(getSourceRegion(),getDestinationRegion());
		}
	}
}

#endif /* _LAYER_H_ */
