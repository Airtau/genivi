/***************************************************************************
 *
 * Copyright 2010, 2011 BMW Car IT GmbH
 * Copyright (C) 2011 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 ****************************************************************************/

#ifndef _WAYLANDDRMWINDOWSYSTEM_H_
#define _WAYLANDDRMWINDOWSYSTEM_H_
#include "WindowSystems/WaylandBaseWindowSystem.h"

struct gbm_device;

class WaylandDrmWindowSystem: public WaylandBaseWindowSystem
{
public:
    WaylandDrmWindowSystem(const char* displayname, int width, int height, Scene* pScene, InputManager* pInputManager);
    virtual ~WaylandDrmWindowSystem();

protected:
    virtual bool createNativeContext();
    virtual bool initGraphicSystem();
    virtual bool createInputEvent();
    virtual void checkForNewSurfaceNativeContent();
    virtual void RedrawAllLayers(bool clear, bool swap);

private:
    int m_fdDev;
    gbm_device* m_gbm;
};

#endif /* _WAYLANDDRMWINDOWSYSTEM_H_ */
