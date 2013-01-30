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

#ifndef _OPTIMIZATION_H
#define _OPTIMIZATION_H

#include "ilm_types.h"

/**
 * Enumeration of renderer optimizations.
 */
enum OptimizationType
{
    OPT_MULTITEXTURE = ILM_OPT_MULTITEXTURE,
    OPT_SKIP_CLEAR = ILM_OPT_SKIP_CLEAR
};

const int OPT_COUNT = 2;

enum OptimizationModeType
{
    OPT_MODE_FORCE_OFF = ILM_OPT_MODE_FORCE_OFF,
    OPT_MODE_FORCE_ON = ILM_OPT_MODE_FORCE_ON,
    OPT_MODE_HEURISTIC = ILM_OPT_MODE_HEURISTIC,
    OPT_MODE_TOGGLE = ILM_OPT_MODE_TOGGLE
};

const int OPT_MODE_COUNT = 4;

#endif /* _OPTIMIZATION_H */
