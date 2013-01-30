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
#include "ScreenDumpCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult ScreenDumpCommand::execute(ICommandExecutor* executor)
{
	ExecutionResult result = ExecutionFailed;
    RendererList& m_rendererList = *(executor->getRendererList());

    bool status = false;

    LOG_INFO("ScreenDumpCommand","making screenshot, output file: " << m_filename);

    status = (m_id == 0);

    if (status)
    {
        // call screen shot on all renderers for now TODO
        RendererListConstIterator iter = m_rendererList.begin();
        RendererListConstIterator iterEnd = m_rendererList.end();

        for (; iter != iterEnd; ++iter)
        {
            IRenderer* renderer = *iter;

            if (renderer)
            {
                renderer->doScreenShot(m_filename);
            }
        }
        result = ExecutionSuccessRedraw;
    }

    return result;
}

const std::string ScreenDumpCommand::getString()
{
    std::stringstream description;
    description << "ScreenDumpCommand("
                << "filename=" << m_filename
                << ", id=" << m_id
                << ")";
    return description.str();
}
