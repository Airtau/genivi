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
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "WindowSystems/WaylandInputEvent.h"
#include "WindowSystems/WaylandBaseWindowSystem.h"

#define HAVE_MKOSTEMP 1

/////////////////////////////////////////////////////////////////////////////

#ifndef HAVE_MKOSTEMP
static int
setCloexecOrClose(int fd)
{
    long flags;
    if (fd == -1)
        return -1;

    flags = fcntl(fd, F_GETFD);
    if (flags == -1)
        goto err;

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
        goto err;

    return fd;
err:
    close(fd);
    return -1;
}
#endif

static int
createTmpFileCloexec(char *tmpname)
{
    int fd;
#ifdef HAVE_MKOSTEMP
    fd = mkostemp(tmpname, O_CLOEXEC);
    if (fd >= 0){
        unlink(tmpname);
    }
#else
    fd = mkstemp(tmpname);
    if (fd >= 0){
        fd = setCloexecOrClose(fd);
        unlink(tmpname);
    }
#endif
    return fd;
}

/////////////////////////////////////////////////////////////////////////////

WaylandInputEvent::WaylandInputEvent(WaylandBaseWindowSystem *windowSystem)
: m_windowSystem(windowSystem)
, m_inputDevice(NULL)
, m_wlEventSource(NULL)
, m_fd(0)
, m_xkbContext(NULL)
{
    initInputEvent();
}

WaylandInputEvent::~WaylandInputEvent()
{
    if (!m_wlEventSource)
        wl_event_source_remove(m_wlEventSource);

    if (m_xkbInfo.keymap)
        xkb_map_unref(m_xkbInfo.keymap);

    if (m_xkbInfo.keymap_area)
        munmap(m_xkbInfo.keymap_area, m_xkbInfo.keymap_size);

    if (m_xkbInfo.keymap_fd >= 0)
        close(m_xkbInfo.keymap_fd);

    xkb_context_unref(m_xkbContext);

    free((char*)m_xkbNames.rules);
    free((char*)m_xkbNames.model);
    free((char*)m_xkbNames.layout);
    free((char*)m_xkbNames.variant);
    free((char*)m_xkbNames.options);
}

void
WaylandInputEvent::initInputEvent()
{
    LOG_DEBUG("WaylandInputEvent", "initInputEvent IN");
    m_inputDevice = new WaylandInputDevice(m_windowSystem->getNativeDisplayHandle());
    if (!m_inputDevice){
        LOG_ERROR("WaylandInputEvent", "Failed to create WaylandInputDevice");
        return;
    }

    memset(&m_xkbInfo, 0, sizeof(m_xkbInfo));
    memset(&m_xkbState, 0, sizeof(m_xkbState));
    memset(&m_xkbNames, 0, sizeof(m_xkbNames));

    LOG_DEBUG("WaylandInputEvent", "initInputEvent OUT");
}

void
WaylandInputEvent::setupInputEvent()
{
    if (!m_xkbContext){
        m_xkbContext = xkb_context_new((enum xkb_context_flags)0);
        if (!m_xkbContext){
            LOG_ERROR("WaylandInputEvent", "Failed to create XKB context");
            return;
        }
    }

    m_xkbNames.rules  = strdup("evdev");
    m_xkbNames.model  = strdup("pc105");
    m_xkbNames.layout = strdup("us");
}

void
WaylandInputEvent::initPointerDevice()
{
    if (m_inputDevice->hasPointer())
        return;
    m_inputDevice->initPointerDevice();
}

void
WaylandInputEvent::initKeyboardDevice(struct xkb_keymap *keymap)
{
    if (m_inputDevice->hasKeyboard())
        return;

    if (keymap){
        m_xkbInfo.keymap = xkb_map_ref(keymap);
        createNewKeymap();
    } else {
        buildGlobalKeymap();
    }

    m_xkbState.state = xkb_state_new(m_xkbInfo.keymap);
    if (!m_xkbState.state){
        LOG_ERROR("WaylandInputEvent", "Failed to initialize XKB state");
        return;
    }

    m_inputDevice->initKeyboardDevice();
}

void
WaylandInputEvent::initTouchDevice()
{
    if (m_inputDevice->hasTouch())
        return;
    m_inputDevice->initTouchDevice();
}

int
WaylandInputEvent::createAnonymousFile(off_t size)
{
    static const char temp[] = "/weston-shared-XXXXXX";
    const char *path;
    char *name;
    int fd;

    path = getenv("XDG_RUNTIME_DIR");
    if (!path){
        return -1;
    }

    name = (char*)malloc(strlen(path) + sizeof(temp));
    if (!name){
        return -1;
    }

    strcpy(name, path);
    strcat(name, temp);
    fd = createTmpFileCloexec(name);

    free(name);
    if (fd < 0){
        return -1;
    }

    if (ftruncate(fd, size) < 0){
        close(fd);
        return -1;
    }

    return fd;
}

void
WaylandInputEvent::createNewKeymap()
{
    m_xkbInfo.shift_mod  = xkb_map_mod_get_index(m_xkbInfo.keymap, XKB_MOD_NAME_SHIFT);
    m_xkbInfo.caps_mod   = xkb_map_mod_get_index(m_xkbInfo.keymap, XKB_MOD_NAME_CAPS);
    m_xkbInfo.ctrl_mod   = xkb_map_mod_get_index(m_xkbInfo.keymap, XKB_MOD_NAME_CTRL);
    m_xkbInfo.alt_mod    = xkb_map_mod_get_index(m_xkbInfo.keymap, XKB_MOD_NAME_ALT);
    m_xkbInfo.mod2_mod   = xkb_map_mod_get_index(m_xkbInfo.keymap, "Mod2");
    m_xkbInfo.mod3_mod   = xkb_map_mod_get_index(m_xkbInfo.keymap, "Mod3");
    m_xkbInfo.super_mod  = xkb_map_mod_get_index(m_xkbInfo.keymap, XKB_MOD_NAME_LOGO);
    m_xkbInfo.mod5_mod   = xkb_map_mod_get_index(m_xkbInfo.keymap, "Mod5");
    m_xkbInfo.num_led    = xkb_map_led_get_index(m_xkbInfo.keymap, XKB_LED_NAME_NUM);
    m_xkbInfo.caps_led   = xkb_map_led_get_index(m_xkbInfo.keymap, XKB_LED_NAME_CAPS);
    m_xkbInfo.scroll_led = xkb_map_led_get_index(m_xkbInfo.keymap, XKB_LED_NAME_SCROLL);

    char *keymapStr = xkb_map_get_as_string(m_xkbInfo.keymap);
    if (keymapStr == NULL){
        LOG_ERROR("WaylandX11InputEvent", "Failed to get string version of keymap");
        return;
    }
    m_xkbInfo.keymap_size = strlen(keymapStr) + 1;

    m_xkbInfo.keymap_fd = createAnonymousFile(m_xkbInfo.keymap_size);
    if (m_xkbInfo.keymap_fd < 0){
        LOG_WARNING("WaylandX11InputEvent", "Creating a keymap file for " <<
                    (unsigned long)m_xkbInfo.keymap_size <<
                    " bytes failed");
        goto err_keymapStr;
    }

    m_xkbInfo.keymap_area = (char*)mmap(NULL,
                                        m_xkbInfo.keymap_size,
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED,
                                        m_xkbInfo.keymap_fd,
                                        0);
    if (m_xkbInfo.keymap_area == MAP_FAILED){
        LOG_WARNING("WaylandX11InputEvent", "Failed to mmap() " <<
                    (unsigned long) m_xkbInfo.keymap_size <<
                    " bytes");
        goto err_dev_zero;
    }
    strcpy(m_xkbInfo.keymap_area, keymapStr);
    free(keymapStr);

    return;

err_dev_zero:
    close(m_xkbInfo.keymap_fd);
    m_xkbInfo.keymap_fd = -1;

err_keymapStr:
    free(keymapStr);
    exit(EXIT_FAILURE);
}

void
WaylandInputEvent::buildGlobalKeymap()
{
    if (m_xkbInfo.keymap != NULL)
        return;

    m_xkbInfo.keymap = xkb_map_new_from_names(m_xkbContext,
                                              &m_xkbNames,
                                              static_cast<xkb_map_compile_flags>(0));
    if (m_xkbInfo.keymap == NULL){
        LOG_ERROR("WaylandInputEvent", "Failed to compile global XKB keymap");
        LOG_ERROR("WaylandInputEvent", "  tried rules: " << m_xkbNames.rules <<
                                       ", model: "       << m_xkbNames.model <<
                                       ", layout: "      << m_xkbNames.layout);
        return;
    }

    createNewKeymap();
}
