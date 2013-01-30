/***************************************************************************
*
* Copyright 2010,2011 BMW Car IT GmbH
* Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
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
#ifndef _SHADER_H_
#define _SHADER_H_

#include <string>
using std::string;

#include "UniformMap.h"
#include "ShaderUniform.h"
#include "ShaderProgram.h"

/**
 * Represents a shader instance.
 *
 * It stores a set of uniform parameters and refers to an OpenGL program object.
 * Some uniform variables are pre-defined, like surface position and size, 
 * opacity, etc... Additionally, there may be user-defined uniforms.
 */
class Shader
{
public:
    /**
     * Creates a new shader instance by vertex and fragment shader name.
     * @param vertFileName   File name of vertex shader.
     * @param fragFileName   File name of fragment shader.
     * @return new Shader instance, NULL if shader could not be loaded, compiled or linked.
     */
    static Shader* createShader(const string& vertFileName, const string& fragFileName);

    /**
     * Destructor
     */
    ~Shader();

    /**
     * @return Unique ID of this instance
     */
    int getId(void) const
    {
        return m_uniqueShaderId;
    }

    /**
     * Start using this shader for rendering.
     */
    void use(void) const
    {
        m_program.use();
    }

    /**
     * Set a uniform.
     */
    void setUniform(const ShaderUniform& uniform);

    /**
     * Update uniform values.
     * Please note that this method doesn't update the standard uniforms
     * used for position, size, etc... They need to be set separately
     * by loadCommonUniforms().
     */
    void loadUniforms(void);

    /**
     * Load uniform values for common surface properties, like position,
     * size, opacity, etc...
     *
     * @param uniforms   Uniform values
     * @param texCount   texture count
     */
    void loadCommonUniforms(const ShaderProgram::CommonUniforms& uniforms, const int texCount) const
    {
        m_program.loadCommonUniforms(uniforms, texCount);
    }

private:
    /**
     * Private constructor.
     * New instances of this class are supposed to be created by createShader(...).
     *
     * @param program  Program object to be used
     */
    Shader(ShaderProgram& program);

private:
    /// unique shader ID
    const unsigned int m_uniqueShaderId;

    /// reference to OpenGL program object used by this shader instance
    ShaderProgram& m_program;

    /// a map of user-defined uniforms
    UniformMap m_uniformMap;

    /// next unique ID
    static unsigned int m_nextUniqueShaderId;
};

#endif /* _SHADER_H_ */

