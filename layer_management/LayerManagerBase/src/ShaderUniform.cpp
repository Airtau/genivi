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

#include <ShaderUniform.h>
#include <ShaderProgram.h>
#include "Log.h"

#include <iostream>
#include <sstream>

using namespace std;

const int ShaderUniform::m_vectorSizesOfDataTypes[] =
{ 0, // Undefined
        1, // Vector1f
        2, // Vector2f
        3, // Vector3f
        4, // Vector4f
        4, // Matrix2f
        9, // Matrix3f
        16 // Matrix4f
        };

ShaderUniform* ShaderUniform::createFromStringDescription(
        const string& description)
{
    istringstream iss(description);
    vector<float> floatData;
    Type type = Undefined;
    string name;
    string token;

    // get name
    iss >> name;

    // get type
    iss >> token;
    if (token == "1f")
    {
        type = Vector1f;
    }
    else if (token == "2f")
    {
        type = Vector2f;
    }
    else if (token == "3f")
    {
        type = Vector3f;
    }
    else if (token == "4f")
    {
        type = Vector4f;
    }
    else if (token == "m2f")
    {
        type = Matrix2f;
    }
    else if (token == "m3f")
    {
        type = Matrix3f;
    }
    else if (token == "m4f")
    {
        type = Matrix4f;
    }
    else
    {
        LOG_ERROR("Shader Uniform","Can not parse Type " << token);
        return NULL;
    }

    // get element count
    unsigned int count = 0;
    iss >> count;
    if (iss.fail())
    {
        // failed to parse element count
        LOG_ERROR("Shader Uniform","Fail to parse element count.");
        return NULL;
    }

    // get transpose parameter if uniform is a matrix
    bool transpose = false;
    if (type == Matrix2f || type == Matrix3f || type == Matrix4f)
    {
        if ((iss >> transpose).fail())
        {
            LOG_ERROR("Shader Uniform","Fail to parse value ");
            return NULL;
        }
    }

    // parse data values
    size_t numValuesExpected = count * m_vectorSizesOfDataTypes[type];
    for (size_t i = 0; i < numValuesExpected; i++)
    {
        float value = 0.0;
        iss >> value;
        if (iss.fail())
        {
            // failed to parse value
            LOG_ERROR("Shader Uniform","Fail to parse value " << value);
            return NULL;
        }
        floatData.push_back(value);
    }

    if (floatData.size() != numValuesExpected)
    {
        LOG_ERROR("Shader Uniform","Invalid number of values");
        return NULL;
    }

    ShaderUniform* uniform = new ShaderUniform(name);
    uniform->setData(type, count, floatData, transpose);

    return uniform;
}

ShaderUniform::ShaderUniform(const string& name, int location) :
    m_name(name), m_location(location), m_type(Undefined), m_count(0),
            m_floatData(), m_transpose(false)
{
}

ShaderUniform::ShaderUniform(const ShaderUniform& other) :
    m_name(other.m_name), m_location(other.m_location), m_type(other.m_type),
            m_count(other.m_count), m_floatData(other.m_floatData),
            m_transpose(other.m_transpose)
{
}

ShaderUniform::~ShaderUniform()
{
}

void ShaderUniform::setData(Type type, int count,
        const std::vector<float>& values, bool transpose)
{
    size_t numValuesExpected = m_vectorSizesOfDataTypes[type] * count;

    if (values.size() == numValuesExpected)
    {
        m_floatData = values;
        m_type = type;
        m_count = count;
        m_transpose = transpose;
    }
    else
    {
        // invalid number of floats in vector
    }
}

void ShaderUniform::setData(const ShaderUniform& other)
{
    m_type = other.m_type;
    m_count = other.m_count;
    m_floatData = other.m_floatData;
    m_transpose = other.m_transpose;
}

void ShaderUniform::load(ShaderProgram& program)
{
    LOG_DEBUG("ShaderUniform","Load Uniform " << getName() << " location " << m_location);
    if ( m_location == 0 ) 
    {
        m_location = program.getUniformLocation(getName().c_str());
    }
    LOG_DEBUG("ShaderUniform","Load Uniform " << getName() << " location " << m_location);
    if (m_location == -1)
    {
        // TODO: error? or silently fail?
        LOG_WARNING("ShaderUniform","Load Uniform failed, location not defined" );
        return;
    }

    // Get C array from vector. We assume the data is internally stored in consecutive memory locations.
    const float* values = &(*m_floatData.begin());

    switch (m_type)
    {
    case Vector1f:
        LOG_DEBUG("ShaderUniform","Load Uniform1fv : " << values[0] );
        program.uniform1fv(m_location, m_count, values);
        break;

    case Vector2f:
        LOG_DEBUG("ShaderUniform","Load Uniform2fv : " << values[0] << "," << values[1] );
        program.uniform2fv(m_location, m_count, values);
        break;

    case Vector3f:
        LOG_DEBUG("ShaderUniform","Load Uniform3fv : " << values[0] << "," << values[1] << "," << values[2] );
        program.uniform3fv(m_location, m_count, values);
        break;

    case Vector4f:
        LOG_DEBUG("ShaderUniform","Load Uniform4fv : " << values[0] << "," << values[1] << "," << values[2] << "," << values[3] );
        program.uniform4fv(m_location, m_count, values);
        break;

    case Matrix2f:
        program.uniformMatrix2fv(m_location, m_count, m_transpose, values);
        break;

    case Matrix3f:
        program.uniformMatrix3fv(m_location, m_count, m_transpose, values);
        break;

    case Matrix4f:
        program.uniformMatrix4fv(m_location, m_count, m_transpose, values);
        break;

    default:
        break;
    }
}

