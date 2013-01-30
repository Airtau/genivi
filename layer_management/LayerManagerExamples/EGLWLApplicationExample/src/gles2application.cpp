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
#include "egl_helper.h"
#include "gles2application.h"
#include "IlmMatrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct t_shaderObject
{
    GLuint fragmentShaderId;
    GLuint vertexShaderId;
    GLuint shaderProgramId;
    GLint matrixLocation;
    GLint colorLocation;
} gles2Shader;

typedef struct vertexBufferObject
{
    GLuint vbo;
} gles2VertexBuffer;

static gles2Shader shader;
static gles2VertexBuffer vertexBuffer;

// Fragment and vertex shaders code
const char * sourceFragShader = "\
        uniform mediump vec4    u_color;\
        void main (void)\
        {\
                gl_FragColor = u_color;\
        }";

const char * sourceVertShader = "\
        attribute highp vec4    a_vertex;\
        uniform mediump mat4    u_matrix;\
        void main(void)\
        {\
                gl_Position = u_matrix*a_vertex;\
        }";

GLfloat triangleVertexData[] =
{
        -0.4f, -0.4f, 0.0f, -0.2f, -0.4f, 0.0f, -0.3f, -0.2f, 0.0f,
        -0.2f, -0.4f, 0.0f, -0.0f, -0.4f, 0.0f, -0.1f, -0.2f, 0.0f,
        -0.0f, -0.4f, 0.0f,  0.2f, -0.4f, 0.0f,  0.1f, -0.2f, 0.0f,
         0.2f, -0.4f, 0.0f,  0.4f, -0.4f, 0.0f,  0.3f, -0.2f, 0.0f,
        -0.3f, -0.2f, 0.0f, -0.1f, -0.2f, 0.0f, -0.2f, -0.0f, 0.0f,
         0.1f, -0.2f, 0.0f,  0.3f, -0.2f, 0.0f,  0.2f, -0.0f, 0.0f,
        -0.2f, -0.0f, 0.0f, -0.0f, -0.0f, 0.0f, -0.1f,  0.2f, 0.0f,
         0.0f, -0.0f, 0.0f,  0.2f, -0.0f, 0.0f,  0.1f,  0.2f, 0.0f,
        -0.1f,  0.2f, 0.0f,  0.1f,  0.2f, 0.0f,  0.0f,  0.4f, 0.0f
};

t_ilm_bool initGlApplication()
{
    if (!initShader())
    {
        return ILM_FALSE;
    }

    if (!initVertexBuffer())
    {
        return ILM_FALSE;
    }

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    return ILM_TRUE;
}

t_ilm_bool initShader()
{
    t_ilm_bool result = ILM_TRUE;

    // Create the fragment shader object
    shader.fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Load Fragment Source
    glShaderSource(shader.fragmentShaderId, 1, (const char**) &sourceFragShader, NULL);

    // Compile the source code of fragment shader
    glCompileShader(shader.fragmentShaderId);

    glGetShaderiv(shader.fragmentShaderId, GL_COMPILE_STATUS, (GLint*) &result);

    if (!result)
    {
        t_ilm_int infoLength, numberChars;
        glGetShaderiv(shader.fragmentShaderId, GL_INFO_LOG_LENGTH, &infoLength);

        // Allocate Log Space
        char* info = (char*) malloc(sizeof(char) * infoLength);
        glGetShaderInfoLog(shader.fragmentShaderId, infoLength, &numberChars, info);

        // Print the error
        printf("Failed to compile fragment shader: %s\n", info);
        free(info);
        return ILM_FALSE;
    }

    // Create the fragment shader object
    shader.vertexShaderId = glCreateShader(GL_VERTEX_SHADER);

    // Load Fragment Source
    glShaderSource(shader.vertexShaderId, 1, (const char**) &sourceVertShader, NULL);

    // Compile the source code of fragment shader
    glCompileShader(shader.vertexShaderId);

    glGetShaderiv(shader.vertexShaderId, GL_COMPILE_STATUS, (GLint*) &result);

    if (!result)
    {
        t_ilm_int infoLength, numberChars;
        glGetShaderiv(shader.vertexShaderId, GL_INFO_LOG_LENGTH, &infoLength);

        // Allocate Log Space
        char* info = (char*) malloc(sizeof(char) * infoLength);
        glGetShaderInfoLog(shader.vertexShaderId, infoLength, &numberChars, info);

        // Print the error
        printf("Failed to compile vertex shader: %s\n", info);
        free(info);
        return ILM_FALSE;
    }

    shader.shaderProgramId = glCreateProgram();

    glAttachShader(shader.shaderProgramId, shader.fragmentShaderId);
    glAttachShader(shader.shaderProgramId, shader.vertexShaderId);

    glBindAttribLocation(shader.shaderProgramId, 0, "a_vertex");

    glLinkProgram(shader.shaderProgramId);

    glGetProgramiv(shader.shaderProgramId, GL_LINK_STATUS, (GLint*) &result);

    if (!result)
    {
        t_ilm_int infoLength, numberChars;
        glGetShaderiv(shader.shaderProgramId, GL_INFO_LOG_LENGTH, &infoLength);

        // Allocate Log Space
        char* info = (char*) malloc(sizeof(char) * infoLength);
        glGetShaderInfoLog(shader.shaderProgramId, infoLength, &numberChars,
                info);

        // Print the error
        printf("Failed to link program: %s\n", info);
        free(info);
        return ILM_FALSE;
    }
    glUseProgram(shader.shaderProgramId);
    shader.matrixLocation = glGetUniformLocation(shader.shaderProgramId, "u_matrix");
    shader.colorLocation = glGetUniformLocation(shader.shaderProgramId, "u_color");
    return result;
}

t_ilm_bool destroyShader()
{
    t_ilm_bool result = ILM_TRUE;
    glDeleteProgram(shader.shaderProgramId);
    glDeleteShader(shader.fragmentShaderId);
    glDeleteShader(shader.vertexShaderId);
    return result;
}

t_ilm_bool initVertexBuffer()
{
    t_ilm_bool result = ILM_TRUE;

    glGenBuffers(1, &vertexBuffer.vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vbo);

    unsigned int uiSize = 27 * (sizeof(GLfloat) * 3); // TODO: remove hard coded values
    glBufferData(GL_ARRAY_BUFFER, uiSize, &triangleVertexData[0], GL_STATIC_DRAW);

    return result;
}

void attachVertexBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

}

void detachVertexBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void destroyVertexBuffer()
{
    glDeleteBuffers(1, &vertexBuffer.vbo);
}

void draw(t_ilm_uint animTime)
{
    static t_ilm_uint startTime = 0;
    static t_ilm_uint currentTime = 0;
    static float angle = 0;
    IlmMatrix matrix;

    currentTime = GetTickCount();

printf("currentTime=%d, startTime=%d, animTime=%d\n", currentTime, startTime, animTime);
    if ((currentTime - startTime) > animTime)
    {
        float currentAngle = 0;
        int i = 0;
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader.shaderProgramId);
        attachVertexBuffer();
        for (i = 10; i > 0; i--)
        {
            currentAngle = angle - ((float) i) * 10.0f;
            IlmMatrixIdentity(matrix);
            IlmMatrixRotateZ(matrix, currentAngle);
            float color[4] = { 0.0f, 1.0f, 1.0f, 0.5f + (0.3f / (float) i) };
            float lineColor[4] = { 0.0f, 0.0f, 0.0f, 0.5f + (0.4f / (float) i) };

            glUniformMatrix4fv(shader.matrixLocation, 1, GL_FALSE, &matrix.f[0]);
            glUniform4fv(shader.colorLocation, 1, &color[0]);
            glDrawArrays(GL_TRIANGLES, 0, 27); // TODO: remove hard coded values

            glUniform4fv(shader.colorLocation, 1, &lineColor[0]);
            int j = 0;
            for (j = 0; j < 9; j++) // TODO: remove hard coded values
            {
                glDrawArrays(GL_LINE_LOOP, j * 3, 3); // TODO: remove hard coded values
            }
        }
        angle += 10.0f;
        detachVertexBuffer();
        swapBuffers();
        startTime = currentTime;
    }

}

void destroyGlApplication()
{
    destroyShader();
    destroyVertexBuffer();
}
