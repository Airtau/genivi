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
#include "Shader.h"
#include <string>
using std::string;

class ShaderTest : public ::testing::Test
{
public:
    void SetUp()
    {
        string vertShaderPath("");
        string fragShaderPath("");
        m_pShader = Shader::createShader(vertShaderPath, fragShaderPath);
        ASSERT_TRUE(m_pShader);
    }

    void TearDown()
    {
        if (m_pShader)
        {
            delete m_pShader;
            m_pShader = 0;
        }
    }

    Shader* m_pShader;
};

TEST_F(ShaderTest, DISABLED_construction)
{
    //static Shader* createShader(const string& vertFileName, const string& fragFileName);
    //~Shader();
}

TEST_F(ShaderTest, DISABLED_getId)
{

}

TEST_F(ShaderTest, DISABLED_use)
{

}

TEST_F(ShaderTest, DISABLED_setUniform)
{

}

TEST_F(ShaderTest, DISABLED_loadUniforms)
{

}

TEST_F(ShaderTest, DISABLED_loadCommonUniforms)
{

}
