/***************************************************************************
*
* Copyright 2010,2011 BMW Car IT GmbH
* Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#ifndef _LAYERSETCHROMAKEYCOMMAND_H_
#define _LAYERSETCHROMAKEYCOMMAND_H_

#include "ICommand.h"

class LayerSetChromaKeyCommand : public ICommand
{
public:
    /*!
     * \action    This command sets the chroma key of a layer within the GENIVI LayerManagement
     * \frequency Called in order to rearrange graphical output.
     * \param[in] sender process id of application that sent this command
     * \param[in] layerid Id of the layer to set the chromakey of.
     * \param[in] array array of color value which is defined in red, green, blue
     * \param[in] length length of array provided as argument array
     * \ingroup Commands
     */
    LayerSetChromaKeyCommand(pid_t sender, unsigned int layerid, unsigned int* array, unsigned  int length)
    : ICommand(ExecuteAsynchronous, sender)
    , m_layerid(layerid)
    , m_array(array)
    , m_length(length)
    {}

    /**
     * \brief default destructor
     */
    virtual ~LayerSetChromaKeyCommand() {}

    /**
     * \brief Execute this command.
     * \param[in] executor Pointer to instance executing the LayerManagement Commands
     * \return ExecutionSuccess: execution successful
     * \return ExecutionSuccessRedraw: execution successful and screen needs to be redrawn
     * \return ExecutionFailed: execution failed
     * \return ExecutionFailedRedraw: execution unsuccessful and screen needs to be redrawn
     */
    virtual ExecutionResult execute(ICommandExecutor* executor);

    /**
     * \brief Get description string for this command.
     * \return String object with description of this command object
     */
    virtual const std::string getString();

private:
    const unsigned int m_layerid;
    unsigned int* m_array;
    unsigned int m_length;
};


#endif /* _LAYERSETCHROMAKEYCOMMAND_H_ */
