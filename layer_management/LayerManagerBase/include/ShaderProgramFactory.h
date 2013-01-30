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

#ifndef _SHADERPROGRAMFACTORY_H_
#define _SHADERPROGRAMFACTORY_H_

#include <ShaderProgram.h>
#include <string>
using std::string;

/**
 * Factory for creating platform specific shader programs.
 */
class ShaderProgramFactory
{
public:
    /**
     * Create a new shader program.
     *
     * @param vertFileName   File name of vertex shader.
     * @param fragFileName   File name of fragment shader.
     * @return new Program instance, NULL if program could not be created.
     */
    static ShaderProgram* createProgram(const string& vertFileName, const string& fragFileName);

    /**
     * Set shader program creator function
     *
     * @param creatorFunc   Pointer to shader program creator function
     */
    static void setCreatorFunc(PfnShaderProgramCreator creatorFunc);

private:
    /**
     * Dummy private constructor, copy-constructor, assignment operator.
     * Prevents creation of instances of this class.
     */
    ShaderProgramFactory(void);
    ShaderProgramFactory(const ShaderProgramFactory&);
    const ShaderProgramFactory& operator=(const ShaderProgramFactory&);

private:
    /// pointer to shader program creator function
    static PfnShaderProgramCreator m_creatorFunctionPointer;
};

#endif /* _SHADERPROGRAMFACTORY_H_ */

