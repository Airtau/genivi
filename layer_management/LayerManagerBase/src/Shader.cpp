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

#include <Shader.h>

unsigned int Shader::m_nextUniqueShaderId=1;

Shader* Shader::createShader(const string& vertFileName, const string& fragFileName)
{
    Shader* shader = 0;
    ShaderProgram* program = 0;

    // Obtain shader program. If a shader program of the same name already exists,
    // its instance will be returned. If it doesn't exist, a new program is returned.
    program = ShaderProgram::obtain(vertFileName, fragFileName);

    if (program)
    {
        // create shader instance
        shader = new Shader(*program);
    }
    else
    {
        // failed to load, compile or link the program
    }

    return shader;
}

Shader::Shader(ShaderProgram& program)
    : m_uniqueShaderId(m_nextUniqueShaderId++)
    , m_program(program)
    , m_uniformMap()
{
    // increase program reference counter
    m_program.ref();
}

Shader::~Shader()
{
    // decrease program reference counter.
    // This might destroy the program if no longer in use.
    m_program.unref();

    UniformMapIterator iter = m_uniformMap.begin();
    UniformMapIterator iterEnd = m_uniformMap.end();

    for (; iter != iterEnd; ++iter)
    {
        ShaderUniform* shaderUniform = (*iter).second;
        delete shaderUniform;
    }

    m_uniformMap.clear();
}

void Shader::setUniform(const ShaderUniform& uniform)
{
    const string& name = uniform.getName();

    if (0 == m_uniformMap.count(name))
    {
        // add new uniform
        int location = m_program.getUniformLocation(name.c_str());
        ShaderUniform* pNewUniform = new ShaderUniform(name, location);
        m_uniformMap[name] = pNewUniform;
    }

    // update uniform data
    m_uniformMap[name]->setData(uniform);
}

void Shader::loadUniforms(void)
{
    UniformMapConstIterator iter = m_uniformMap.begin();
    UniformMapConstIterator iterEnd = m_uniformMap.end();

    for (; iter != iterEnd; ++iter)
    {       
        ShaderUniform* uniform = (*iter).second;
        uniform->load(m_program);
    }
}

