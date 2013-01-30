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

#ifndef _SURFACEREMOVENATIVECONTENTCOMMAND_H_
#define _SURFACEREMOVENATIVECONTENTCOMMAND_H_

#include "ICommand.h"
#include "PixelFormat.h"
#include "IScene.h"

class SurfaceRemoveNativeContentCommand : public ICommand
{
public:
    /*!
     * \action    This command removes the native content (application content)
     *            of a surface within the GENIVI LayerManagement
     * \frequency Typically should not be needed unless a client wants to
     *            re-use the surface with multiple contents.
     * \param[in] sender process id of application that sent this command
     * \param[in] surfaceId id of surface
     * \ingroup Commands
     */
    SurfaceRemoveNativeContentCommand(pid_t sender, unsigned int surfaceId)
    : ICommand(ExecuteAsynchronous, sender)
    , m_surfaceId(surfaceId)
    {}

    /**
     * \brief default destructor
     */
    virtual ~SurfaceRemoveNativeContentCommand() {}

    /**
     * \brief Execute this command.
     * \param[in] executor Pointer to instance executing the LayerManagement COmmands
     * \return ExecutionSuccess: execution successful
     * \return ExecutionSuccessRedraw: execution successful and screen needs to be redrawn
     * \return ExecutionFailed: execution failed
     * \return ExecutionFailedRedraw: execution unsuccessful and screen needs to be redrawn
     */
    virtual ExecutionResult execute(ICommandExecutor* executor);

    /**
     * \brief Get description string for this command.
     * \return String object with description of this command object
     */
    virtual const std::string getString();

private:
    uint m_surfaceId;

    // for unit testing
    //template <typename nativeHandle_type, typename pixelformat_type, typename OriginalWidth_type, typename OriginalHeight_type> friend class SurfaceSetRenderBufferCommandEqMatcherP4;
};


#endif /* _SURFACEREMOVENATIVECONTENTCOMMAND_H_ */
