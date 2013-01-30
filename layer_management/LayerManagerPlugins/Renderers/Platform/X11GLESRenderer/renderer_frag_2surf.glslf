// File: renderer_frag_2surf.glslf

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
#pragma profilepragma blendoperation( gl_FragColor, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_ONE, GL_ONE_MINUS_SRC_ALPHA )

// alpha value of the surfaces
uniform mediump float uAlphaVal[2];
// textureunit which is accessed
uniform sampler2D uTexUnit[2];

// texture coordinates sended by the vertex shader
varying mediump vec2 vTexout[2];

void main()
{
    // correct Texture Coords;
    mediump vec4 color0 = texture2D(uTexUnit[0], vTexout[0]);

    color0.a = color0.a * uAlphaVal[0];
    color0.rgb = color0.rgb * color0.a;

    gl_FragColor = texture2D(uTexUnit[1], vTexout[1]);

    gl_FragColor.a = gl_FragColor.a * uAlphaVal[1];
    gl_FragColor.rgb = mix(color0.rgb, gl_FragColor.rgb, gl_FragColor.a);

    gl_FragColor.a = gl_FragColor.a + color0.a - (gl_FragColor.a * color0.a);
    gl_FragColor.rgb = gl_FragColor.rgb / gl_FragColor.a;
}
