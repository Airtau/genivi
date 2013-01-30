// File: renderer_vert_2surf.glslv

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
*       http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
****************************************************************************/

// Vertex position. Will be in range 0 to 1
attribute highp vec2 aPosition;
attribute highp vec2 aTexCoords;
attribute highp vec2 aTexCoords2;

uniform mediump mat4 uMatrix;
// normalized window position
// (0, 0 is lower left,  1, 1 is upper right)
uniform mediump float uX;
uniform mediump float uY;

// normalized window size
// 1.0 will cover the entire viewport
uniform mediump float uWidth;
uniform mediump float uHeight;

// texrange describes the width and height of texture area which should be rendered
uniform mediump vec2 uTexRange[2];
// texrange describes the x and y position of the texture area which should be rendered
uniform mediump vec2 uTexOffset[2];

// texture coordinates
varying mediump vec2 vTexout[2];
void main()
{
    highp vec4 position;

    // Apply offset and range
    position.xy = vec2(uX + uWidth * aPosition.x,  uY + uHeight * aPosition.y);

    // Convert from [0, 1] to [-1, 1]
    position.xy = 2.0 * position.xy - 1.0;
    position.zw = vec2(0.0, 1.0);

    gl_Position = uMatrix * position;

    // Invert t coordinates as textures provided by window systems are stored top-down instead of the GL convention of bottom-up
    vTexout[0] = vec2(uTexOffset[0].x + uTexRange[0].x * aTexCoords.x, uTexOffset[0].y + uTexRange[0].y * (1.0 - aTexCoords.y));
    vTexout[1] = vec2(uTexOffset[1].x + uTexRange[1].x * aTexCoords2.x, uTexOffset[1].y + uTexRange[1].y * (1.0 - aTexCoords2.y));
}
