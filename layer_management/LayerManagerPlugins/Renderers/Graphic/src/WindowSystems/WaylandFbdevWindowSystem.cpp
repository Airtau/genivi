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

#include "WindowSystems/WaylandFbdevWindowSystem.h"
#include "Log.h"
#if 0
#include "Layer.h"
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#endif
#include <linux/fb.h>
#include <fcntl.h>
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif
#include <iomanip>
#include "WindowSystems/WaylandEvdevInputEvent.h"

WaylandFbdevWindowSystem::WaylandFbdevWindowSystem(const char* displayname, int width, int height, Scene* pScene, InputManager* pInputManager)
: WaylandBaseWindowSystem(displayname, width, height, pScene, pInputManager)
, m_fdFB(-1)
{
    LOG_DEBUG("WaylandFbdevWindowSystem", "creating WaylandFbdevWindowSystem width:" << width << " height:" << height);
}

bool WaylandFbdevWindowSystem::initGraphicSystem()
{
    return graphicSystem->init((void*)m_fdFB, (void*)NULL);
}

bool WaylandFbdevWindowSystem::createNativeContext()
{
    m_fdFB = open("/dev/fb0", O_RDWR);
    if (0 > m_fdFB)
    {
        LOG_ERROR("WaylandFbdevWindowSystem", "failed to open FB");
        return false;
    }
    LOG_DEBUG("WaylandFbdevWindowSystem", "SUCCESS:open FB");
    return true;
}

bool WaylandFbdevWindowSystem::createInputEvent()
{
    m_inputEvent = new WaylandEvdevInputEvent(this);
    m_inputEvent->setupInputEvent();
    return true;
}
