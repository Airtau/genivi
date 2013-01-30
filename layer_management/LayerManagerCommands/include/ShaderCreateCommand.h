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

#ifndef _SHADERCREATECOMMAND_H_
#define _SHADERCREATECOMMAND_H_

#include "ICommand.h"
#include "Shader.h"
#include <string>

class ShaderCreateCommand : public ICommand
{
public:
    /*!
     * \action    This command creates a shader within the GENIVI LayerManagement
     * \frequency Once per shader.
     * \param[in] sender process id of application that sent this command
     * \param[in] vertName path and filename to vertex shader source file
     * \param[in] fragName path and filename to fragment shader source file
     * \param[in] id location to store shader id on execution
     * \ingroup Commands
     */
    ShaderCreateCommand(pid_t sender, const std::string& vertName, const std::string& fragName, unsigned int* id)
    : ICommand(ExecuteSynchronous, sender)
    , m_vertName(vertName)
    , m_fragName(fragName)
    , m_returnID(id)
    {}

    /**
     * \brief default destructor
     */
    virtual ~ShaderCreateCommand() {}

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

    const std::string getVertName() const;
    const std::string getFragName() const;

private:
    const std::string m_vertName;
    const std::string m_fragName;
    unsigned int* m_returnID;
};

#endif /* _SHADERCREATECOMMAND_H_ */
