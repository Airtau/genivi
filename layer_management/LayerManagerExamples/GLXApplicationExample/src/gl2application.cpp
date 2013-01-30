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
#include "glx_helper.h"
#include "config.h"
#include "gl2application.h"
#include "IlmMatrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

GLfloat triangleVertexData[] = {   -0.4f ,-0.4f ,0.0f,
                                   -0.2f ,-0.4f ,0.0f,
                                   -0.3f ,-0.2f ,0.0f,

                                   -0.2f ,-0.4f ,0.0f,
                                   -0.0f ,-0.4f ,0.0f,
                                   -0.1f ,-0.2f ,0.0f,

                                   -0.0f ,-0.4f ,0.0f,
                                    0.2f ,-0.4f ,0.0f,
                                    0.1f ,-0.2f ,0.0f,

                                    0.2f ,-0.4f ,0.0f,
                                    0.4f ,-0.4f ,0.0f,
                                    0.3f ,-0.2f ,0.0f,

                                    -0.3f ,-0.2f ,0.0f,
                                    -0.1f ,-0.2f ,0.0f,
                                    -0.2f ,-0.0f ,0.0f,

                                    0.1f ,-0.2f ,0.0f,
                                    0.3f ,-0.2f ,0.0f,
                                    0.2f ,-0.0f ,0.0f,

                                    -0.2f ,-0.0f ,0.0f,
                                    -0.0f ,-0.0f ,0.0f,
                                    -0.1f ,0.2f ,0.0f,

                                    0.0f ,-0.0f ,0.0f,
                                    0.2f ,-0.0f ,0.0f,
                                    0.1f ,0.2f ,0.0f,

                                    -0.1f ,0.2f ,0.0f,
                                    0.1f ,0.2f ,0.0f,
                                    0.0f ,0.4f ,0.0f
                               };


t_ilm_bool initGlApplication(GLuint width, GLuint height)
{
    t_ilm_bool result = ILM_TRUE;
    GLfloat aspectratio = (GLfloat) width / GLfloat(height);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (width <= height)
    {
        glOrtho(-1.0, 1.0, 1.0 / aspectratio, 1.0 / aspectratio, 1.0, -1.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	/* Make the background visible so that we can click on it */
	glClearColor(.2f, .4f, .5f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    return result;
}

void draw(t_ilm_uint animTime)
{
    static t_ilm_uint startTime = 0;
    static t_ilm_uint currentTime = 0;
    static float angle = 0;
    IlmMatrix matrix;

    currentTime = GetTickCount();

    if ((currentTime - startTime) > animTime)
    {
        float currentAngle = 0;
        glClear(GL_COLOR_BUFFER_BIT);
        int i = 0;
        for (i = 10; i > 0; i--)
        {
            currentAngle = angle - ((float) i) * 10.0f;
            IlmMatrixIdentity(matrix);
            IlmMatrixRotateZ(matrix, currentAngle);
            glLoadMatrixf(&matrix.f[0]);

            float color[4] = { 1.0, 0.0, 1.0, 0.5 + (0.3 / (float) i) };
            float lineColor[4] = { 0.0, 0.0, 0.0, 0.5 + (0.4 / (float) i) };

            glBegin(GL_TRIANGLES);

            for (int index = 0; index < 27; index++) // TODO: remove hard coded value
            {
                glColor4fv(color);
                glVertex3fv(&triangleVertexData[index * 3]);
            }
            glEnd();

            int j = 0;
            for (j = 0; j < 9; j++)
            {
                glBegin(GL_LINE_LOOP);
                glColor4fv(lineColor);
                glVertex3fv(&triangleVertexData[j * 9]);
                glColor4fv(lineColor);
                glVertex3fv(&triangleVertexData[j * 9 + 3]);
                glColor4fv(lineColor);
                glVertex3fv(&triangleVertexData[j * 9 + 6]);
                glEnd();
            }
        }
        angle += 10.0f;
        swapBuffers();
        startTime = currentTime;
    }
}

void destroyGlApplication()
{
}

