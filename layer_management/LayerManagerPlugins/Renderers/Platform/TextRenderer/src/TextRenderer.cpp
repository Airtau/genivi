/***************************************************************************
*
* Copyright 2012 BMW Car IT GmbH
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

#include "TextRenderer.h"
#include "Configuration.h"
#include "Log.h"
#include <string.h>
#include <fstream>

TextRenderer::TextRenderer(ICommandExecutor& executor, Configuration& config)
: BaseRenderer(executor, config)
, m_width(0)
, m_height(0)
{
    LOG_DEBUG("TextRenderer", "created");
}

bool TextRenderer::start(int width, int height, const char* displayname)
{
    m_height = height;
    m_width = width;

    LOG_DEBUG("TextRenderer", "start("
              << "width=" << width
              << ", height=" << height
              << ", displayname=" << displayname << ")");
    return true;
}

void TextRenderer::stop()
{
    LOG_DEBUG("TextRenderer", "stop");
}

TextRenderer::~TextRenderer()
{
    LOG_DEBUG("TextRenderer", "destroyed");
}

void TextRenderer::doScreenShot(std::string fileToSave)
{
    LOG_DEBUG("TextRenderer", "doScreenShot("
              << "fileToSave=" << fileToSave << ")");
    std::fstream file(fileToSave.c_str());
}

void TextRenderer::doScreenShotOfLayer(std::string fileToSave,uint id)
{
    LOG_DEBUG("TextRenderer", "doScreenShotOfLayer("
              << "fileToSave=" << fileToSave
              << ", id=" << id << ")");
    std::fstream file(fileToSave.c_str());
}

void TextRenderer::doScreenShotOfSurface(std::string fileToSave, uint id, uint layer_id)
{
    LOG_DEBUG("TextRenderer", "doScreenShotOfSurface("
              << "fileToSave=" << fileToSave
              << ", id=" << id
              << ", layer_id=" << layer_id << ")");
    std::fstream file(fileToSave.c_str());
}

uint TextRenderer::getNumberOfHardwareLayers(uint screenID)
{
    LOG_DEBUG("TextRenderer", "getNumberOfHardwareLayers(screenID=" << screenID << ")");
    return 0;
}

uint* TextRenderer::getScreenResolution(uint screenID)
{
    LOG_DEBUG("TextRenderer", "getScreenResolution(screenID=" << screenID << ")");

    uint * resolution = new uint[2];
    resolution[0] = m_width;
    resolution[1] = m_height;
    return resolution;
}

uint* TextRenderer::getScreenIDs(uint* length)
{
    LOG_DEBUG("TextRenderer", "getScreenIDs");

    // Screens in X11 can be addresses/accessed by just the number - we must only know how many there are
    uint numberOfScreens = 1;
    uint* screenIDS = new uint[numberOfScreens];
    for (uint i = 0; i < numberOfScreens; i++)
    {
        screenIDS[i] = i;
    }
    *length = numberOfScreens;
    return screenIDS;
}

void TextRenderer::signalWindowSystemRedraw()
{
    LOG_DEBUG("TextRenderer", "signalWindowSystemRedraw");
}

void TextRenderer::forceCompositionWindowSystem()
{
    LOG_DEBUG("TextRenderer", "forceCompositionWindowSystem");
}

bool TextRenderer::setOptimizationMode(OptimizationType id, OptimizationModeType mode)
{
    LOG_DEBUG("TextRenderer", "setOptimizationMode("
              << "id=" << id
              << ", mode=" << mode << ")");
    m_optimizationMode = mode;
    return true;
}

bool TextRenderer::getOptimizationMode(OptimizationType id, OptimizationModeType* mode)
{
    (void)mode;
    LOG_DEBUG("TextRenderer", "getOptimizationMode("
              << "id=" << id << ")");
    *mode = m_optimizationMode;
    return true;
}

HealthCondition TextRenderer::pluginGetHealth()
{
    return HealthRunning;
}

t_ilm_const_string TextRenderer::pluginGetName() const
{
    return "TextRenderer";
}

DECLARE_LAYERMANAGEMENT_PLUGIN(TextRenderer)
