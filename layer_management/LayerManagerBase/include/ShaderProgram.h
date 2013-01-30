/***************************************************************************
*
* Copyright 2010,2011 BMW Car IT GmbH

* Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#ifndef _SHADERPROGRAM_H_
#define _SHADERPROGRAM_H_

#include <string>
using std::string;

#include "ShaderProgramList.h"

class ShaderProgram;
typedef ShaderProgram* (*PfnShaderProgramCreator)(const string&, const string&);

#define MAX_MULTI_SURFACE 2

/**
 * Represents an OpenGL shader program.
 *
 * There is a global list of allocated shader programs. Each instance is reference
 * counted, so if program is no longer in use by any shader instance, it will
 * be deleted automatically.
 */
class ShaderProgram
{
public:
    /**
     * Stores uniform values for common surface properties,
     * like position, size, opacity, etc...
     */
    struct CommonUniforms
    {
        float x;
        float y;
        float width;
        float height;
        float* matrix;
        float chromaKey[3];
        bool chromaKeyEnabled;
        // Per-texture uniforms
        float opacity[MAX_MULTI_SURFACE];
        float texRange[MAX_MULTI_SURFACE][2];
        float texOffset[MAX_MULTI_SURFACE][2];
        int texUnit[MAX_MULTI_SURFACE];
    };

    /**
     * Return a shader program from the gobal list. If no matching instance is found, a new
     * one will be created and added to the list.
     *
     * @param vertFileName   File name of vertex shader.
     * @param fragFileName   File name of fragment shader.
     * @return new Program instance, NULL if shader could not be loaded, compiled or linked.
     */
    static ShaderProgram* obtain(const string& vertFileName, const string& fragFileName);

    /**
     * Destructor
     */
    virtual ~ShaderProgram();

    /**
     * Start using the shader program for rendering.
     */
    virtual void use(void) const = 0;

    /**
     * Load uniform values for common surface properties, like position,
     * size, opacity, etc...
     *
     * @param uniforms   Uniform values
     * @param texCount   texture count
     */
    void loadCommonUniforms(const CommonUniforms& uniforms, const int texCount) const;

    /**
     * @return The name of the vertex shader
     */
    const string& getVertexName(void) const
    {
        return m_vertFileName;
    }

    /**
     * @return The name of the fragment shader
     */
    const string& getFragmentName(void) const
    {
        return m_fragFileName;
    }

    /**
     * Increment usage counter for this object.
     */
    void ref(void);

    /**
     * Decrement usage counter for this object.
     * It will be delete if usage counter reaches 0.
     */
    void unref(void);

    virtual int getUniformLocation(const char* name) = 0;

    virtual void uniform1iv(int location, int count, const int* v) const = 0;

    virtual void uniform1fv(int location, int count, const float* v) const = 0;

    virtual void uniform2fv(int location, int count, const float* v) const = 0;

    virtual void uniform3fv(int location, int count, const float* v) const = 0;

    virtual void uniform4fv(int location, int count, const float* v) const = 0;

    virtual void uniformMatrix2fv(int location, int count, bool transpose, const float* v) const = 0;

    virtual void uniformMatrix3fv(int location, int count, bool transpose, const float* v) const = 0;

    virtual void uniformMatrix4fv(int location, int count, bool transpose, const float* v) const = 0;

protected:
    /**
     * Protected constructor.
     * New instances of this class are supposed to be created by the shader program factory.
     *
     * @param vertFileName    File name of vertex shader.
     * @param fragFileName    File name of fragment shader.
     */
    ShaderProgram(const string& vertFileName, const string& fragFileName);

    /**
     * Update the list of uniform locations
     */
    void updateCommonUniformLocations(void);

private:
    /// usage counter
    int m_referenceCounter;

    /// name of the vertex shader
    string m_vertFileName;

    /// name of the fragment shader
    string m_fragFileName;

    // uniform locations
    int m_xLoc;
    int m_yLoc;
    int m_widthLoc;
    int m_heightLoc;
    int m_matrixLoc;

    // per-texture uniforms
    int m_opacityLoc;
    int m_texRangeLoc;
    int m_texOffsetLoc;
    int m_texUnitLoc;
    int m_chromaKeyLoc;
    /// global list of programs
    static ShaderProgramList m_programList;
};

#endif /* _SHADERPROGRAM_H_ */

