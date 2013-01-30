/***************************************************************************
*
* Copyright 2012 Valeo
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


#include <cassert>

#include "Log.h"
#include "config.h"
#include "IScene.h"
#include "InputManager.h"


InputManager::InputManager(IScene* s) :
    m_pScene(s),
    m_KbdFocus(NULL),
    m_PointerFocus(NULL),
    m_TouchFocus(NULL)
{
    assert(s != NULL);
    pthread_mutex_init(&m_mutex, NULL);
}


InputManager::~InputManager()
{
    m_pScene = NULL;
    pthread_mutex_destroy(&m_mutex);
}


/**
 * @brief Set the keyboard focus on a specific surface.
 */
bool InputManager::setKeyboardFocusOn(unsigned int surfId)
{
    bool ret;
    Surface * surf;

    surf = m_pScene->getSurface(surfId);
    if (surf == NULL)
    {
        LOG_ERROR("InputManager", "setKeyboardFocusOn() called on an unknown surface id: " << surfId);
        ret = false;
    }
    else
    {
        if (surf->isInputEventAcceptedFrom(INPUT_DEVICE_KEYBOARD))
        {
            _setKbdFocus(surf);
            ret = true;
        }
        else
        {
            // Hum, the surface does not accept kbd events but we've been asked to set the focus on it
            _setKbdFocus(NULL);
            ret = false;
            LOG_ERROR("InputManager", "setKeyboardFocusOn() called on surface " << surfId << ". This surface does not accept keyboard event !");
        }
    }


    return ret;
}

/**
 * @brief Get the identifier of the surface which has the keyboard focus
 */
unsigned int InputManager::getKeyboardFocusSurfaceId()
{
    Surface *surf;
    unsigned int surfId;

    surf = _getKbdFocus();
    if (!surf)
    {
        surfId = GraphicalObject::INVALID_ID;
    }
    else
    {
        surfId = surf->getID();
    }

    return surfId;
}


bool InputManager::updateInputEventAcceptanceOn(unsigned int surfId, InputDevice devices, bool accept)
{
    bool ret;
    Surface * surf;

    surf = m_pScene->getSurface(surfId);
    if (surf == NULL)
    {
        LOG_ERROR("InputManager", "setInputAcceptanceOn() called on an unknown surface id: " << surfId);
        ret = false;
    }
    else
    {
        surf->updateInputEventAcceptanceFrom(devices, accept);
        ret = true;
    }

    return ret;
}



/**
 * @brief Report keyboard event.
 * @param[in] state The state of the key. Can be either INPUT_STATE_PRESSED or INPUT_STATE_RELEASED
 * @param[in] keyId A uniq identifier for the key being reported.
 * @return The Surface to which to report the event, or NULL
 */
Surface * InputManager::reportKeyboardEvent(InputEventState state, long keyId)
{
    Surface * elected;

    switch (state)
    {
        case INPUT_STATE_PRESSED:
            elected = _getKbdFocus();
            m_KeyMap[keyId] = elected;
            break;

        case INPUT_STATE_RELEASED:
            elected = m_KeyMap[keyId];
            break;

        default:
            elected = NULL;
            LOG_WARNING("InputManager", "Invalid input state reported for reportKeyboardEvent() : " << state);
            break;
    }

    return elected;
}


/**
 * @return The Surface to which to report the event, or NULL
 */
Surface * InputManager::reportPointerEvent(Point& p)
{
    Surface * elected;

    switch (p.state)
    {
        case INPUT_STATE_PRESSED:
            elected = electSurfaceForPointerEvent(p.x, p.y);
            _setPointerFocus(elected);
            /* Pointer pressed also assigns the focus for touch events */
            _setTouchFocus(elected);
            break;

        case INPUT_STATE_OTHER:
        case INPUT_STATE_MOTION:
        case INPUT_STATE_RELEASED:
            elected = _getPointerFocus();
            if (elected != NULL)
            {
                transformGlobalToLocalCoordinates(elected, p.x, p.y);
            }
            break;

        default:
            elected = NULL;
            LOG_WARNING("InputManager", "Invalid input state reported for reportPointerEvent() : " << p.state);
    }

    return elected;
}


Surface * InputManager::reportTouchEvent(PointVect& pv)
{
    Surface * elected;

    elected = _getTouchFocus();
    if (elected != NULL)
    {
        PointVectIterator it;
        for (it = pv.begin(); it != pv.end(); it++)
        {
            transformGlobalToLocalCoordinates(elected, it->x, it->y);
        }
    }

    return elected;
}


Surface * InputManager::electSurfaceForPointerEvent(int& x, int& y)
{
    Surface* surf;
    LayerList &ll = m_pScene->getCurrentRenderOrder(0);
    LayerListConstReverseIterator currentLayer;
    SurfaceListConstReverseIterator currentSurf;
    int x_SurfCoordinate, y_SurfCoordinate;

    surf = NULL;
    /* Need to browse for all layers. 1st layer of m_currentRenderOrder is rendered
     * on bottom, last one is rendrered on top. So we have to reverse iterate */
    LayerListConstReverseIterator layerEnd(ll.rend());
    for (currentLayer = ll.rbegin();
         currentLayer != layerEnd && surf == NULL;
         currentLayer++)
    {
        if ( ((*currentLayer)->visibility) && ((*currentLayer)->getOpacity() != 0) )
        {
            if ((*currentLayer)->isInside(x, y))
            {
                x_SurfCoordinate = x;
                y_SurfCoordinate = y;
                (*currentLayer)->DestToSourceCoordinates(&x_SurfCoordinate, &y_SurfCoordinate, false);
                /* Need to browse for all surfaces */
                SurfaceListConstReverseIterator surfEnd((*currentLayer)->getAllSurfaces().rend());
                for (currentSurf = (*currentLayer)->getAllSurfaces().rbegin();
                     currentSurf !=  surfEnd && surf == NULL;
                     currentSurf++)
                {
                    if ( ((*currentSurf)->hasNativeContent()) && ((*currentSurf)->visibility) && ((*currentSurf)->getOpacity() != 0) )
                    {
                        if ((*currentSurf)->isInside(x_SurfCoordinate, y_SurfCoordinate))
                        {
                            if ((*currentSurf)->isInputEventAcceptedFrom(INPUT_DEVICE_POINTER))
                            {
                                surf = *currentSurf;
                                (*currentSurf)->DestToSourceCoordinates(&x_SurfCoordinate, &y_SurfCoordinate, false);
                                x = x_SurfCoordinate;
                                y = y_SurfCoordinate;
                            }
                        }
                    }
                }
            }
        }
    }
    return surf;
}


void InputManager::transformGlobalToLocalCoordinates(Surface* surf, int& x, int& y)
{
    Layer* layer;

    assert(surf != NULL);

    layer = m_pScene->getLayer(surf->getContainingLayerId());
    if (layer != NULL)
    {
        layer->DestToSourceCoordinates(&x,&y, false);
        surf->DestToSourceCoordinates(&x,&y, false);
    }
}


//@{
/*
 * m_KbdFocus can be concurrently accessed by :
 *   - reportKeyboardEvent(), called in the renderer thread context
 *   - setKeyboardFocusOn(), called in the IPC mechanism context
 *
 *   So read & Write to this variable must be protected by exclusive to avoid
 *   race conditions
 */

void InputManager::_setKbdFocus(Surface * s)
{
    pthread_mutex_lock(&m_mutex);
    m_KbdFocus = s;
    pthread_mutex_unlock(&m_mutex);
}

Surface * InputManager::_getKbdFocus()
{
    Surface * s;
    pthread_mutex_lock(&m_mutex);
    s = m_KbdFocus;
    pthread_mutex_unlock(&m_mutex);

    return s;
}
//@}



//@{
/*
 * m_PointerFocus can NOT be concurrently accessed as of today :
 * It is only accessed by reportPointerEvent(), called in the renderer thread context
 *
 * But it's cheap to add getter / setter if needed in the future & at least to have
 * a similar access mechanism than m_KbdFocus
 */
void InputManager::_setPointerFocus(Surface * s)
{
    m_PointerFocus = s;
}

Surface * InputManager::_getPointerFocus()
{
    return m_PointerFocus;
}
//@}


//@{
/*
 * m_TouchFocus can NOT be concurrently accessed as of today.
 * It is only accessed by reportPointerEvent() & reportTouchEvent(), both called in the renderer thread context.
 *
 * But it's cheap to add getter / setter if needed in the future & at least to have
 * a similar access mechanism than m_KbdFocus
 */
void InputManager::_setTouchFocus(Surface * s)
{
    m_TouchFocus = s;
}

Surface * InputManager::_getTouchFocus()
{
    return m_TouchFocus;
}


