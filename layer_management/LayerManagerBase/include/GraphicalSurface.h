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

#ifndef _GRAPHICALSURFACE_H_
#define _GRAPHICALSURFACE_H_

#include "GraphicalObject.h"
#include "OrientationType.h"
#include "Rectangle.h"
#include "Vector2.h"

/**
 * Abstract Type representing a graphical surface.
 */
class GraphicalSurface : public GraphicalObject
{
public:
    GraphicalSurface(ObjectType type, int creatorPid);

    GraphicalSurface(int externalId, ObjectType type, int creatorPid);

    virtual ~GraphicalSurface() {}

    /**
     * @brief Set Orientation value
     * @param[in] newOrientation the new value. Multiples of 90 degrees. (0->0°, 1->90°, 2->180°,3->279°)
     * @return TRUE if the new orientation value is not equal to the current orientation value
     *         FALSE if they are equal
     */
    bool setOrientation(OrientationType newOrientation);

    OrientationType getOrientation() const;

    /**
     * @brief Set Source Viewport (only use portion of source graphics)
     * @param[in] newSource Rectangle defining position and size within surface (clip from the left)
     * @return TRUE if the new source rectangle is not equal to the current source rectangle
     *         FALSE if they are equal
     */
    bool setSourceRegion(const Rectangle& newSource);

    const Rectangle& getSourceRegion() const;

    /**
     * Set Destination Viewport (Scale output)
     * @param[in] newDestination Rectangle defining destination position and size
     * @return TRUE if the new destination rectangle is not equal to the current destination rectangle
     *         FALSE if they are equal
     */
    bool setDestinationRegion(const Rectangle& newDestination);

    bool setPosition(const unsigned int& x, const unsigned int& y);

    Vector2 getPosition();
    bool setDimension(const unsigned int& width, const unsigned int& height);

    const Rectangle& getDestinationRegion() const;
    Vector2 getDimension();

    /**
     * @brief Indicate if a x,y position is inside the destination region.
     *        Attention: Graphical Surface rotation is not yet supported.
     * @param x_DestCoordinateSyst x position in the destination coordinate system
     * @param y_DestCoordinateSyst y position in the destination coordinate system
     * @return TRUE if the position is inside the destination region
     */
    bool isInside(unsigned int x_DestCoordinateSyst, unsigned int y_DestCoordinateSyst) const;

    /**
     * @brief Transform a x,y position from destination coordinate system to
     *              source coordinate system. Attention, to get valid result the x,y
     *              positions given in parameter must be located within the destination
     *              region of the GraphicalSurface
     *
     * @param[in]  x x position in the destination coordinate system
     * @param[out] x x position in the source coordinate system
     * @param[in] y y position in the destination coordinate system
     * @param[out] y y position in the source coordinate system
     * @param check If TRUE, a test will be done to make sure the x,y positions
     *              given in parameter are located within the destination region.
     *
     * @return TRUE if the coordinates have been translated
     *         FALSE if an error occured, exp: The position is not in the destination region
     */
    bool DestToSourceCoordinates(int *x, int *y, bool check) const;

    int OriginalSourceWidth;
    int OriginalSourceHeight;    
    
private:
    OrientationType m_orientation; // Rotation of the graphical content
    Rectangle m_sourceViewport;
    Rectangle m_destinationViewport;
};


inline GraphicalSurface::GraphicalSurface(ObjectType type, int creatorPid)
: GraphicalObject(type, 1.0, false, creatorPid)
, OriginalSourceWidth(0)
, OriginalSourceHeight(0)
, m_orientation(Zero)
, m_sourceViewport(0,0,0,0)
, m_destinationViewport(0,0,0,0)
{
}

inline GraphicalSurface::GraphicalSurface(int externalId, ObjectType type, int creatorPid)
: GraphicalObject(externalId, type, 1.0, false, creatorPid)
, OriginalSourceWidth(0)
, OriginalSourceHeight(0)
, m_orientation(Zero)
, m_sourceViewport(0,0,0,0)
, m_destinationViewport(0,0,0,0)
{
}

inline bool GraphicalSurface::setOrientation(OrientationType newOrientation)
{
    if (m_orientation != newOrientation)
    {
        m_orientation = newOrientation;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline OrientationType GraphicalSurface::getOrientation() const
{
    return m_orientation;
}

inline bool GraphicalSurface::setSourceRegion(const Rectangle& newSource)
{
    if (!(m_sourceViewport == newSource))
    {
        m_sourceViewport = newSource;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline const Rectangle& GraphicalSurface::getSourceRegion() const
{
    return m_sourceViewport;
}

inline bool GraphicalSurface::setDestinationRegion(const Rectangle& newDestination)
{
    if (!(m_destinationViewport == newDestination))
    {
        m_destinationViewport = newDestination;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline bool GraphicalSurface::setPosition(const unsigned int& x, const unsigned int& y)
{
    if (m_destinationViewport.x != x || m_destinationViewport.y != y)
    {
        m_destinationViewport.x = x;
        m_destinationViewport.y = y;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline Vector2 GraphicalSurface::getPosition()
{
    return Vector2(m_destinationViewport.x,m_destinationViewport.y);
}

inline bool GraphicalSurface::setDimension(const unsigned int& width, const unsigned int& height)
{
    if (m_destinationViewport.width != width || m_destinationViewport.height != height)
    {
        m_destinationViewport.width = width;
        m_destinationViewport.height = height;
        renderPropertyChanged = true;
        return true;
    }
    return false;
}

inline const Rectangle& GraphicalSurface::getDestinationRegion() const
{
    return m_destinationViewport;
}

inline Vector2 GraphicalSurface::getDimension()
{
    return Vector2(m_destinationViewport.width, m_destinationViewport.height);
}

#endif /* _GRAPHICALSURFACE_H_ */
