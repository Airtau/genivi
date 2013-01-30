/***************************************************************************
 *
 * Copyright 2011 Valeo
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

#include "GraphicalSurface.h"


bool GraphicalSurface::isInside(unsigned int x_DestCoordinateSyst, unsigned int y_DestCoordinateSyst) const
{
	bool ret;

	switch (m_orientation)
	{
		case Zero:
			ret = (
			           ((x_DestCoordinateSyst >= m_destinationViewport.x) && (x_DestCoordinateSyst < m_destinationViewport.x + m_destinationViewport.width))
			        &&
			           ((y_DestCoordinateSyst >= m_destinationViewport.y) && (y_DestCoordinateSyst < m_destinationViewport.y + m_destinationViewport.height))
			      );
			break;

		case Ninety:
		case OneEighty:
		case TwoSeventy:
			/* Not yet supported */
			ret = false;
			break;

		default:
			ret = false;
			break;
	}

	return ret;
}



/**
 * We are going to change of coordinate system.
 * The input coordinates are in the Dest system, we have to change them to the Source system.
 * For this, 4 operations have to be undone, in order :
 *  - translation in source system
 *  - scaling
 *  - translation in destination system
 *  - rotation (not yet implemented)
 *
 */
bool GraphicalSurface::DestToSourceCoordinates(int *x, int *y, bool check) const
{
	bool 	ret;
	int   	TVxD, TVyD;  /* Translation vector x,y in destination system */	
	int   	TVxS, TVyS;  /* Translation vector x,y in source system */
	float 	SFx, SFy;    /* Scaling factor x,y  */

	if (!check || isInside(*x, *y))
	{
		/* The translation vector in the Destination system */
		TVxD = m_destinationViewport.x;
		TVyD = m_destinationViewport.y;

		/* The translation vector in the Source system */
		TVxS = m_sourceViewport.x;
		TVyS = m_sourceViewport.y;

		/* Compute the scaling factors */
		SFx = (float) m_sourceViewport.width  / (float) m_destinationViewport.width;
		SFy = (float) m_sourceViewport.height / (float) m_destinationViewport.height;

		/* Compute the rotation */
		// To be done ...

		/* Apply the transformations */
		*x = ((*x - TVxD) * SFx) + TVxS;
		*y = ((*y - TVyD) * SFy) + TVyS;

		ret = true;
	}
	else
	{
		ret = false;
	}

	return ret;
}

