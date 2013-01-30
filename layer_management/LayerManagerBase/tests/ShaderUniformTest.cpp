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
#include "ShaderUniform.h"
#include <string>
using std::string;

class ShaderUniformTest : public ::testing::Test
{
public:
    void SetUp()
    {
        string uniformName("");
        m_pShaderUniform = new ShaderUniform(uniformName);
        ASSERT_TRUE(m_pShaderUniform);
    }

    void TearDown()
    {
        if (m_pShaderUniform)
        {
            delete m_pShaderUniform;
            m_pShaderUniform = 0;
        }
    }

    ShaderUniform* m_pShaderUniform;
};

TEST_F(ShaderUniformTest, DISABLED_construction)
{
    // note default argument = separate test
    //ShaderUniform(const string& name, int location = -1);
    //ShaderUniform(const ShaderUniform& other);
    //~ShaderUniform();
}

TEST_F(ShaderUniformTest, DISABLED_createFromStringDescription)
{

}

TEST_F(ShaderUniformTest, DISABLED_getName)
{

}

TEST_F(ShaderUniformTest, DISABLED_setData)
{
    //void setData(Type type, int count, const std::vector<float>& values, bool transpose = false);
    //void setData(const ShaderUniform& other);
}

TEST_F(ShaderUniformTest, DISABLED_load)
{

}
