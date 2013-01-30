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
#include "SurfaceSetNativeContentCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include <sstream>

ExecutionResult SurfaceSetNativeContentCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());
    ExecutionResult result = ExecutionFailed;

    Surface* surface = scene.getSurface(m_surfaceId);

    if (surface)
    {
        if(!surface->hasNativeContent())
        {
            surface->setNativeContent(m_nativeHandle);
            surface->setPixelFormat(m_pixelformat);
            surface->OriginalSourceWidth = m_originalWidth;
            surface->OriginalSourceHeight = m_originalHeight;

            // has to be set by application
            //surface->setDestinationRegion(Rectangle(0, 0, m_originalWidth, m_originalHeight));
            //surface->setSourceRegion(Rectangle(0, 0, m_originalWidth, m_originalHeight));

            result = ExecutionSuccessRedraw;
        }
        else if( (unsigned int) surface->getNativeContent() == m_nativeHandle)
        {
            result = ExecutionSuccess;
        }
    }

    return result;
}

const std::string SurfaceSetNativeContentCommand::getString()
{
    std::stringstream description;
    description << "SurfaceSetNativeContentCommand("
                << "surfaceId=" << m_surfaceId
                << ", nativeHandle=" << m_nativeHandle
                << ", pixelformat=" << m_pixelformat
                << ", OriginalWidth=" << m_originalWidth
                << ", OriginalHeight=" << m_originalHeight
                << ")";
    return description.str();
}
