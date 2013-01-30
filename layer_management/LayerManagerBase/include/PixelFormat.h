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

#ifndef _PIXELFORMAT_H_
#define _PIXELFORMAT_H_

#include "ilm_types.h"

/**
 * Enumeration of possible PixelFormats
 */
enum PixelFormat
{
    PIXELFORMAT_R8 = ILM_PIXELFORMAT_R_8,
    PIXELFORMAT_RGB888 = ILM_PIXELFORMAT_RGB_888,
    PIXELFORMAT_RGBA8888 = ILM_PIXELFORMAT_RGBA_8888,
    PIXELFORMAT_RGB565 = ILM_PIXELFORMAT_RGB_565,
    PIXELFORMAT_RGBA5551 = ILM_PIXELFORMAT_RGBA_5551,
    PIXELFORMAT_RGBA6661 = ILM_PIXELFORMAT_RGBA_6661,
    PIXELFORMAT_RGBA4444 = ILM_PIXELFORMAT_RGBA_4444,
    PIXELFORMAT_UNKNOWN = ILM_PIXEL_FORMAT_UNKNOWN
};

#define PixelFormatHasAlpha(pf) ((pf == PIXELFORMAT_RGBA8888) || (pf == PIXELFORMAT_RGBA5551) || (pf == PIXELFORMAT_RGBA6661) || (pf == PIXELFORMAT_RGBA4444))
#endif /* _PIXELFORMAT_H_ */
