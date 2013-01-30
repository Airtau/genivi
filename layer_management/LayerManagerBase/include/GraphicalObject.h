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

#ifndef _GRAPHICALOBJECT_H_
#define _GRAPHICALOBJECT_H_

#include "ObjectType.h"
#include "ApplicationReferenceList.h"

class Shader;

/**
 * Base class of all objects representing graphical objects within the layermanagement.
 */
class GraphicalObject
{
public:

    GraphicalObject(ObjectType type, double opacity, bool visibility, int creatorPid);

    GraphicalObject(int externalId, ObjectType type, double opacity, bool visibility, int creatorPid);

    virtual ~GraphicalObject() {}

    /**
     * @brief Set alpha value
     * @param[in] newOpacity The new Alpha Value between 0.0 (full transparency) and 1.0 (fully visible)
     * @return TRUE if the new Alpha Value is not equal to the current Alpha Value
     *         FALSE if they are equal
     */
    virtual bool setOpacity(double newOpacity);

    /**
     * @brief Get alpha value
     * @return The current Alpha Value between 0.0 (full transparency) and 1.0 (fully visible)
     */
    double getOpacity() const;

    /**
     * @brief Set chroma key enabled value
     * @param[in] newEnabled The new ChromaKey enable (true) or disable (false)
     * @return TRUE if the new ChromaKey Enabled Value are not equal to the current Value
     *         FALSE if they are equal
     */
    virtual bool setChromaKeyEnabled(bool newEnabled);

    /**
     * @brief Get chroma key enabled value
     * @return The current chromakey enable (true) or disable (false)
     */
    bool getChromaKeyEnabled() const;

    /**
     * @brief Set chroma key
     * @param[in] newRed The new Red Value between 0 and 255
     * @param[in] newGreen The new Green Value between 0 and 255
     * @param[in] newBlue The new Blue Value between 0 and 255
     * @return TRUE if the new ChromaKey Values are not equal to the current ChromaKey Values
     *         FALSE if they are equal
     */
    virtual bool setChromaKey(unsigned char newRed, unsigned char newGreen, unsigned char newBlue);

    /**
     * @brief Get chroma key values
     * @param[out] red The current Red Value between 0 and 255
     * @param[out] green The current Green Value between 0 and 255
     * @param[out] blue The current Blue Value between 0 and 255
     */
    void getChromaKey(unsigned char& red, unsigned char& green, unsigned char& blue) const;

    /**
     * Set the visibility
     * @param[in] newVisibility set this object visible (true) or invisible (false)
     * @return TRUE if the new visiblity value is not equal to the current visibility value
     *         FALSE if they are equal
     */
    virtual bool setVisibility(bool newVisibility);

    bool getVisibility() const;

    /**
     * @brief Get external ID for graphical object
     * @return external id of graphical object
     */
    virtual unsigned int getID();

    /**
     * Assign custom shader for rendering
     *
     * @param[in] s Custom shader. If NULL, default shader will be used.
     * @return TRUE if the new custom shader is different from the current custom shader
     *         FALSE if they are same
     */
    bool setShader(Shader* s);

    /**
     * @brief get the currently assigned custom shader object
     * @return currently assigned custom shader object
     */
    Shader* getShader();

    /**
     * @brief add a client application to be notified on property changes of this graphical object.
     * \param[in] client handle to connected client that wants to receive notifications on changes of this object
     */
    void addNotification(t_ilm_client_handle client);

    /**
     * @brief remove a client application from the notification list on property changes of this graphical object.
     * \param[in] client handle to connected client that does not want to receive notifications on changes of this object any longer
     */
    void removeNotification(t_ilm_client_handle client);

    /**
     * @brief get list of client that registered to a notification for this object
     */
    ApplicationReferenceList& getNotificationClients();

    /**
     * @brief get process id of process that created this object
     */
    int getCreatorPid();


public:
    static const unsigned int INVALID_ID;
    ObjectType type;
    bool renderPropertyChanged;
    bool damaged;

    ///     Pointer to currently assigned shader. If NULL, a default shader will be used.
    Shader* shader;
    double opacity;
    bool visibility;
    bool chromaKeyEnabled;
    unsigned char chromaKeyRed;
    unsigned char chromaKeyGreen;
    unsigned char chromaKeyBlue;

protected:
    unsigned int graphicInternalId;
    unsigned int graphicExternalId;
    ApplicationReferenceList applicationList;

private:
    static unsigned int nextGraphicId[TypeMax];
    int createdByPid;
};

inline GraphicalObject::GraphicalObject(ObjectType type, double opacity, bool visibility, int creatorPid)
: type(type)
, renderPropertyChanged(false)
, damaged(false)
, shader(0)
, opacity(opacity)
, visibility(visibility)
, chromaKeyEnabled(false)
, chromaKeyRed(0)
, chromaKeyGreen(0)
, chromaKeyBlue(0)
, graphicInternalId(nextGraphicId[type]++)
, createdByPid(creatorPid)
{
    graphicExternalId = graphicInternalId;
}

inline GraphicalObject::GraphicalObject(int externalId, ObjectType type, double opacity, bool visibility, int creatorPid)
: type(type)
, renderPropertyChanged(false)
, damaged(false)
, shader(0)
, opacity(opacity)
, visibility(visibility)
, chromaKeyEnabled(false)
, chromaKeyRed(0)
, chromaKeyGreen(0)
, chromaKeyBlue(0)
, graphicInternalId(nextGraphicId[type]++)
, graphicExternalId(externalId)
, createdByPid(creatorPid)
{
}

inline bool GraphicalObject::setOpacity(double newOpacity)
{
    if (opacity != newOpacity)
    {
        opacity = newOpacity;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline double GraphicalObject::getOpacity() const
{
    return opacity;
}

inline bool GraphicalObject::setVisibility(bool newVisibility)
{
    if (visibility != newVisibility)
    {
        visibility = newVisibility;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline bool GraphicalObject::getVisibility() const
{
    return visibility;
}

inline bool GraphicalObject::setChromaKeyEnabled(bool newEnabled)
{
    if (chromaKeyEnabled != newEnabled)
    {
        chromaKeyEnabled = newEnabled;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline bool GraphicalObject::getChromaKeyEnabled() const
{
    return chromaKeyEnabled;
}

inline bool GraphicalObject::setChromaKey(unsigned char newRed, unsigned char newGreen, unsigned char newBlue)
{
    if ((chromaKeyRed != newRed) || (chromaKeyGreen != newGreen) || (chromaKeyBlue != newBlue))
    {
        chromaKeyRed = newRed;
        chromaKeyGreen = newGreen;
        chromaKeyBlue = newBlue;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline void GraphicalObject::getChromaKey(unsigned char& red, unsigned char& green, unsigned char& blue) const
{
    red = chromaKeyRed;
    green = chromaKeyGreen;
    blue = chromaKeyBlue;
}

inline unsigned int GraphicalObject::getID()
{
    return graphicExternalId;
}

inline bool GraphicalObject::setShader(Shader* s)
{
    if (shader != s)
    {
        shader = s;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline Shader* GraphicalObject::getShader()
{
    return shader;
}

inline void GraphicalObject::addNotification(t_ilm_client_handle client)
{
    applicationList.push_back(client);
}

inline void GraphicalObject::removeNotification(t_ilm_client_handle client)
{
    applicationList.remove(client);
}

inline ApplicationReferenceList& GraphicalObject::getNotificationClients()
{
    return applicationList;
}

inline int GraphicalObject::getCreatorPid()
{
    return createdByPid;
}

#endif /* _GRAPHICALOBJECT_H_ */
