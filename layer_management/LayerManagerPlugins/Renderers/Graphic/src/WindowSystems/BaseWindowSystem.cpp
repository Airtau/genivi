/***************************************************************************
 *
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

#include "WindowSystems/BaseWindowSystem.h"

void BaseWindowSystem::ClearDamage()
{
    LayerList layers = m_pScene->getCurrentRenderOrder(0);
    for (LayerListIterator layer = layers.begin(); layer != layers.end(); layer++)
    {
        SurfaceList surfaces = (*layer)->getAllSurfaces();
        for (SurfaceListIterator surface = surfaces.begin(); surface != surfaces.end(); surface++)
        {
            // Clear Surface Damage
            (*surface)->damaged = false;
            (*surface)->renderPropertyChanged = false;
        }
        // Clear Layer Damage
        (*layer)->damaged = false;
        (*layer)->renderPropertyChanged = false;
    }
    // Clear Window System Damage
    m_damaged = false;
}
