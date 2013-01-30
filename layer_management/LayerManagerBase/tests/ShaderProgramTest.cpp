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

#include <gtest/gtest.h>
#include "ShaderProgram.h"
#include "ShaderProgramFactory.h"
#include <string>
using std::string;

class ShaderProgramTest : public ::testing::Test
{
public:
    void SetUp()
    {
        string vertShaderPath("");
        string fragShaderPath("");
        m_pShaderProgram = ShaderProgramFactory::createProgram(vertShaderPath, fragShaderPath);
        ASSERT_TRUE(m_pShaderProgram);
    }

    void TearDown()
    {
        if (m_pShaderProgram)
        {
            delete m_pShaderProgram;
            m_pShaderProgram = 0;
        }
    }

    ShaderProgram* m_pShaderProgram;
};

TEST_F(ShaderProgramTest, DISABLED_obtain)
{

}

TEST_F(ShaderProgramTest, DISABLED_use)
{

}

TEST_F(ShaderProgramTest, DISABLED_loadCommonUniforms)
{

}

TEST_F(ShaderProgramTest, DISABLED_getVertexName)
{

}

TEST_F(ShaderProgramTest, DISABLED_getFragmentName)
{

}

TEST_F(ShaderProgramTest, DISABLED_ref)
{

}

TEST_F(ShaderProgramTest, DISABLED_unref)
{

}

TEST_F(ShaderProgramTest, DISABLED_getUniformLocation)
{

}

TEST_F(ShaderProgramTest, DISABLED_uniform1iv)
{

}

TEST_F(ShaderProgramTest, DISABLED_uniform1fv)
{

}

TEST_F(ShaderProgramTest, DISABLED_uniform2fv)
{

}

TEST_F(ShaderProgramTest, DISABLED_uniform3fv)
{

}

TEST_F(ShaderProgramTest, DISABLED_uniform4fv)
{

}

TEST_F(ShaderProgramTest, DISABLED_uniformMatrix2fv)
{

}

TEST_F(ShaderProgramTest, DISABLED_uniformMatrix3fv)
{

}

TEST_F(ShaderProgramTest, DISABLED_uniformMatrix4fv)
{

}
