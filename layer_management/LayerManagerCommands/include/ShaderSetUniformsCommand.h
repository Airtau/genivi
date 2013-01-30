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
#ifndef _SHADERSETUNIFORMSCOMMAND_H_
#define _SHADERSETUNIFORMSCOMMAND_H_

#include "ICommand.h"
#include <string>
#include <vector>

class ShaderSetUniformsCommand : public ICommand
{
public:
    /*!
     * \action    This command sets the uniform value of a shader within the GENIVI LayerManagement
     * \frequency Typically for every rendered frame.
     * \param[in] sender process id of application that sent this command
     * \param[in] shaderid id of shader
     * \param[in] uniforms vector holding uniforms
     * \ingroup Commands
     */
    ShaderSetUniformsCommand(pid_t sender, unsigned int shaderid, const std::vector<std::string>& uniforms)
    : ICommand(ExecuteSynchronous, sender)
    , m_shaderid(shaderid)
    , m_uniforms(uniforms)
    {}

    /**
     * \brief default destructor
     */
    virtual ~ShaderSetUniformsCommand() {}

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

    const std::vector<std::string> getUniforms() const;
    unsigned int getShaderId() const;

private:
    const unsigned int m_shaderid;
    const std::vector<std::string> m_uniforms;
};

#endif /* _SHADERSETUNIFORMSCOMMAND_H_ */


