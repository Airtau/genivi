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

#ifndef _RENDERUTIL_H_
#define _RENDERUTIL_H_

#include "GLES2/gl2.h"
#include "Log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

// Loads a file containing shader code
static char* RenderUtilLoadShaderFile(const char *szFilename)
{
    if(!szFilename)
    {
        return(NULL);
    }

    FILE* pFile = fopen(szFilename, "rb");
    if(!pFile)
    {
        LOG_ERROR("RenderUtilLoadShaderFile","Unable to open ShaderFile " << szFilename);
        return(NULL);
    }

    fseek(pFile, 0, SEEK_END);
    long size = ftell(pFile);

    if (size < 0)
    {
        LOG_ERROR("RenderUtilLoadShaderFile","Unable get size of ShaderFile " << szFilename);
        fclose(pFile);
        return(NULL);
    }

    fseek(pFile, 0, SEEK_SET);

    char *pBuffer = new char[size+1];
    if(!pBuffer)
    {
        LOG_ERROR("RenderUtilLoadShaderFile","Unable to allocate Memory for ShaderFile " << szFilename);
        fclose(pFile);
        return(NULL);
    }

    if(1 != fread(pBuffer, size, 1, pFile))
    {
        LOG_ERROR("RenderUtilLoadShaderFile","Unable to allocate Memory for ShaderFile " << szFilename);
        fclose(pFile);
        delete [] pBuffer;
        return(NULL);
    }

    pBuffer[size] = 0;
    fclose(pFile);
    LOG_DEBUG("RenderUtilShaderDebug","loaded file with bytes: " << size);
    return(pBuffer);
}



// If shader compilation fails, prints the associated logs
static void RenderUtilShaderDebug(GLuint obj, GLenum status, const char* op)
{
    char errorMessage[1024];
    int success;
    // log output.
    int len;
    char *str = NULL;
    if (status == GL_COMPILE_STATUS)
    {
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            str = new char[len];
            glGetShaderInfoLog(obj, len, NULL, str);
        }
    }
    else
    { // LINK
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            str = new char[len];
            glGetProgramInfoLog(obj, len, NULL, str);
        }
    }
    if (str != NULL && *str != '\0')
    {
        strncpy(errorMessage, "--- ",1024); // TODO: sizeof(errorMessage)
        strncat(errorMessage, op, 1000); // TODO: why 1000? dangerous
        strncat(errorMessage, " log ---", 8);
        LOG_ERROR("RenderUtilShaderDebug",errorMessage);
        LOG_ERROR("RenderUtilShaderDebug",str);
    }

    if (str)
    {
        delete[] str;
    }

    // check the compile / link status.
    if (status == GL_COMPILE_STATUS)
    {
        glGetShaderiv(obj, status, &success);
        if (!success)
        {
            glGetShaderiv(obj, GL_SHADER_SOURCE_LENGTH, &len);
            if (len > 0)
            {
                str = new char[len];
                glGetShaderSource(obj, len, NULL, str);
                if (str != NULL && *str != '\0')
                {
                    strncpy(errorMessage, "--- ",1024);// TODO: sizeof(errorMessage)
                    strncat(errorMessage,op, 1000); // TODO: why 1000? dangerous
                    strncat(errorMessage, " log ---", 8);
                    LOG_ERROR("RenderUtilShaderDebug",errorMessage);
                    LOG_ERROR("RenderUtilShaderDebug",str);
                }
                delete[] str;
            }
        }
    }
    else
    { // LINK
        glGetProgramiv(obj, status, &success);
    }

    if (!success)
    {
        strncpy(errorMessage, "--- ",1024);// TODO: sizeof(errorMessage)
           strncat(errorMessage,op, 1000); // TODO: why 1000? dangerous
          strncat(errorMessage, " failed ---", 7);
        LOG_ERROR("RenderUtilShaderDebug",errorMessage);
    }
}

// Take precompiled shader binaries and builds a shader program

static GLuint RenderUtilLoadShaderBinaries(
    const char* vertBin, GLuint vertBinSize,
    const char* fragBin, GLuint fragBinSize,
    GLuint binaryFormat,
    GLboolean debugging)
{
    LOG_DEBUG("RenderUtil","Loading shaders binaries. Size " << vertBinSize << " : " << fragBinSize );
    // Binary shaders not supportable for non-ES OpenGL
    GLuint prog;
    GLuint vertShader;
    GLuint fragShader;

    // Create the program
    prog = glCreateProgram();

    // Create the GL shader objects
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Load the binary data into the shader objects
    glShaderBinary(1, &vertShader,
                    binaryFormat, vertBin, vertBinSize);
    glShaderBinary(1, &fragShader,
                    binaryFormat, fragBin, fragBinSize);

    // Attach the shaders to the program
    glAttachShader(prog, vertShader);
    glAttachShader(prog, fragShader);

#if 0
    // Delete the shaders
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
#endif

    // Link the shader program
    glLinkProgram(prog);
    if (debugging) 
    {
        RenderUtilShaderDebug(prog, GL_LINK_STATUS, "Program Link");
    }

    return prog;
}

// Takes shader source files, compiles them, and builds a shader program
static GLuint RenderUtilLoadShaderSources(const char* vertFile, const char* fragFile, GLboolean debugging)
{
    LOG_DEBUG("RenderUtilShaderDebug","loading shaders sources");
    LOG_DEBUG("RenderUtilShaderDebug","loading vertex shader: " << vertFile);
    LOG_DEBUG("RenderUtilShaderDebug","loading fragment shader: " << fragFile);

    GLuint prog = 0;
    char*  vertSource;
    char*  fragSource;
    GLint  vertSourceLen;
    GLint  fragSourceLen;
    GLuint vertShader;
    GLuint fragShader;
    GLint compiled;
    GLint linked;

    // Load the shader files
    vertSource    =RenderUtilLoadShaderFile(vertFile);
    fragSource    = RenderUtilLoadShaderFile(fragFile);
    LOG_DEBUG("RenderUtilShaderDebug","loaded shaders source files");
    if (!vertSource || !fragSource) goto done;
    vertSourceLen = (GLint)strlen(vertSource);
    fragSourceLen = (GLint)strlen(fragSource);

    // Create the program
    prog = glCreateProgram();
    if (prog==0)
    {
        LOG_DEBUG("RenderUtilShaderDebug","glError occured : " << glGetError()); 
        LOG_DEBUG("RenderUtilShaderDebug","could not create prog, prog is 0");
    }

    // Create the GL shader objects
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Load shader sources into GL and compile
    glShaderSource(vertShader, 1, (const char**)&vertSource, &vertSourceLen);
    glCompileShader(vertShader);
    if (debugging)
    {
        RenderUtilShaderDebug(vertShader, GL_COMPILE_STATUS, "Vert Compile");
    }

    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
    if (compiled)
    {
        glShaderSource(fragShader, 1, (const char**)&fragSource, &fragSourceLen);
        glCompileShader(fragShader);
        if (debugging)
        {
            RenderUtilShaderDebug(fragShader, GL_COMPILE_STATUS, "Frag Compile");
        }
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
        if (compiled)
        {
            // Attach the shaders to the program
            glAttachShader(prog, vertShader);
            glAttachShader(prog, fragShader);

            // Delete the shaders
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);

            // Link and validate the shader program
            glLinkProgram(prog);

            if (debugging)
            {
                RenderUtilShaderDebug(prog, GL_LINK_STATUS, "Program Link");
            }
            glValidateProgram(prog);
            if (debugging)
            {
                RenderUtilShaderDebug(prog, GL_VALIDATE_STATUS, "Program Validate");
            }

            glGetProgramiv(prog, GL_LINK_STATUS, &linked);
            if (!linked)
            {
                LOG_DEBUG("RenderUtilShaderDebug","could not link shader");
            }
        }
        else
        {
         LOG_DEBUG("RenderUtilShaderDebug","could not compile fragment shader");
        }
    }
    else
    {
        LOG_DEBUG("RenderUtilShaderDebug","could not compile vertex shader");
    }

    if (!compiled || !linked)
    {
        glDeleteProgram(prog);
        prog = 0;
    }

    done:

    delete[] fragSource;
    delete[] vertSource;
    return prog;
}

#endif /* _RENDERUTIL_H_ */
