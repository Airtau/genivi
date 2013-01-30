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

#include <ShaderProgram.h>
#include <ShaderProgramFactory.h>
#include "ShaderProgramList.h"

// global program list
ShaderProgramList ShaderProgram::m_programList;

ShaderProgram* ShaderProgram::obtain(const string& vertFileName,
        const string& fragFileName)
{
    ShaderProgram* program = 0;

    ShaderProgramListIterator iter = m_programList.begin();
    ShaderProgramListIterator iterEnd = m_programList.end();

    // search for a matching shader program in the global list
    for (; iter != iterEnd; ++iter)
    {
        ShaderProgram* currentProgram = *iter;

        if (!currentProgram)
        {
            continue;
        }

        bool vertexFileNameMatch = (currentProgram->getVertexName()
                == vertFileName);
        bool fragFileNameMatch = (currentProgram->getFragmentName()
                == fragFileName);

        if (vertexFileNameMatch && fragFileNameMatch)
        {
            program = currentProgram;
            break;
        }
    }

    if (!program)
    {
        // no matching program object found in global list -> create a new one

        // we are using a factory approach to create platform specific shader programs:
        program = ShaderProgramFactory::createProgram(vertFileName,
                fragFileName);

        if (program)
        {
            // add program to global list
            m_programList.push_back(program);
        }
    }

    return program;
}

ShaderProgram::ShaderProgram(const string& vertFileName, const string& fragFileName)
: m_referenceCounter(0)
, m_vertFileName(vertFileName)
, m_fragFileName(fragFileName)
, m_xLoc(0)
, m_yLoc(0)
, m_widthLoc(0)
, m_heightLoc(0)
, m_matrixLoc(0)
, m_opacityLoc(0)
, m_texRangeLoc(0)
, m_texOffsetLoc(0)
, m_texUnitLoc(0)
, m_chromaKeyLoc(0)
{
    // void
}

ShaderProgram::~ShaderProgram()
{
    // void
}

void ShaderProgram::ref(void)
{
    m_referenceCounter++;
}

void ShaderProgram::unref(void)
{
    m_referenceCounter--;
    if (m_referenceCounter <= 0)
    {
        // remove program from global list
        m_programList.remove(this);

        // destroy object
        delete this; // TODO: always safe?
    }
}

void ShaderProgram::loadCommonUniforms(const CommonUniforms& uniforms, const int texCount) const
{
    if (m_xLoc >= 0)
    {
        uniform1fv(m_xLoc, 1, &uniforms.x);
    }
    if (m_yLoc >= 0)
    {
        uniform1fv(m_yLoc, 1, &uniforms.y);
    }
    if (m_widthLoc >= 0)
    {
        uniform1fv(m_widthLoc, 1, &uniforms.width);
    }
    if (m_heightLoc >= 0)
    {
        uniform1fv(m_heightLoc, 1, &uniforms.height);
    }
    if (m_matrixLoc >= 0)
    {
        uniformMatrix4fv(m_matrixLoc, 1, false, uniforms.matrix);
    }

    // Per-texture uniforms
    if (m_opacityLoc >= 0)
    {
        uniform1fv(m_opacityLoc, texCount, uniforms.opacity);
    }
    if (m_texRangeLoc >= 0)
    {
        uniform2fv(m_texRangeLoc, texCount, uniforms.texRange[0]);
    }
    if (m_texOffsetLoc >= 0)
    {
        uniform2fv(m_texOffsetLoc, texCount, uniforms.texOffset[0]);
    }
    if (m_texUnitLoc >= 0)
    {
        uniform1iv(m_texUnitLoc, texCount, uniforms.texUnit);
    }
    if (m_chromaKeyLoc>= 0)
    {
        uniform3fv(m_chromaKeyLoc, 1, uniforms.chromaKey);
    }
}

void ShaderProgram::updateCommonUniformLocations(void)
{
    // get uniform locations for common surface properties
    m_xLoc = getUniformLocation("uX");
    m_yLoc = getUniformLocation("uY");
    m_widthLoc = getUniformLocation("uWidth");
    m_heightLoc = getUniformLocation("uHeight");
    m_matrixLoc = getUniformLocation("uMatrix");
    m_opacityLoc = getUniformLocation("uAlphaVal");
    m_texRangeLoc = getUniformLocation("uTexRange");
    m_texOffsetLoc = getUniformLocation("uTexOffset");
    m_texUnitLoc = getUniformLocation("uTexUnit");
    m_chromaKeyLoc = getUniformLocation("uChromaKey");
}

