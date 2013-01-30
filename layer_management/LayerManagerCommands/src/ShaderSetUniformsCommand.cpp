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
#include "ShaderSetUniformsCommand.h"
#include "ICommandExecutor.h"
#include "UniformMap.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult ShaderSetUniformsCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionSuccessRedraw;

    // get shader by its ID
    ShaderMap &shaderMap = scene.m_shaderMap;
    ShaderMapIterator iter = shaderMap.find(m_shaderid);
    ShaderMapIterator iterEnd = shaderMap.end();

    if (iter != iterEnd)
    {
        Shader* shader = (*iter).second;

        if (shader)
        {
            // get uniform descriptions
            const std::vector<std::string> uniformDesc = m_uniforms;
            std::vector<std::string>::const_iterator uiterator;

            for (uiterator=uniformDesc.begin(); uiterator != uniformDesc.end(); ++uiterator)
            {
                // parse description string and create uniform object
                ShaderUniform* uniform = ShaderUniform::createFromStringDescription(*uiterator);
                if (uniform)
                {
                    shader->setUniform(*uniform);
                    delete uniform;
                }
                else
                {
                    LOG_ERROR("ShaderSetUniformsCommand", "Failed to parse uniform description");
                    result = ExecutionFailed;
                }
            }
        }
        else
        {
            // shader not found
            LOG_ERROR("ShaderSetUniformsCommand", "shader ID " << m_shaderid << " found but object was null");
            result = ExecutionFailed;
        }
    }
    else
    {
        // shader not found
        LOG_ERROR("ShaderSetUniformsCommand", "shader ID "<<m_shaderid<<" not found");
        result = ExecutionFailed;
    }

    if (result == ExecutionSuccessRedraw)
    {
        IRenderer* renderer = *((executor->getRendererList())->begin());
        renderer->forceCompositionWindowSystem();
    }

    return result;
}

const std::vector<std::string> ShaderSetUniformsCommand::getUniforms() const
{
    return m_uniforms;
}

unsigned int ShaderSetUniformsCommand::getShaderId() const
{
    return m_shaderid;
}

const std::string ShaderSetUniformsCommand::getString()
{
    std::stringstream description;
    description << "ShaderSetUniformsCommand("
                << "shaderid=" << m_shaderid
                << "m_uniforms=[";

    std::vector<std::string>::const_iterator iter = m_uniforms.begin();
    std::vector<std::string>::const_iterator iterEnd = m_uniforms.end();

    for (; iter != iterEnd; ++iter)
    {
        description << *iter << ",";
    }

    description << ")";

    return description.str();
}
