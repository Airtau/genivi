
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

#ifndef _IAPPLICATION_REFERENCE_H_
#define _IAPPLICATION_REFERENCE_H_
#include <locale>
#include <string.h>
#include <stdlib.h>

/**
 * Abstract Base Class for all Application References 
 */
class IApplicationReference
{
public:
    /**
     * Constructor: Contructs a Object with the provided Application id and SerialId
     * 
     */
    IApplicationReference(char* processName, unsigned int processId);
    ~IApplicationReference();

     const char* getProcessName();
     unsigned int getProcessId();

protected:
    char* m_processName;
    unsigned int m_processId;
};

inline IApplicationReference::IApplicationReference(char* processName, unsigned int processId)
: m_processName(strdup(processName))
, m_processId(processId)
{
}

inline IApplicationReference::~IApplicationReference()
{
    if (m_processName)
    {
        free(m_processName);
    }
}

inline unsigned int IApplicationReference::getProcessId()
{
    return m_processId;
}

inline const char* IApplicationReference::getProcessName()
{
    return m_processName;
}

#endif /* _IAPPLICATION_REFERENCE_H_ */
