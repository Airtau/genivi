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
#ifndef __HEALTHMONITORLIST_H__
#define __HEALTHMONITORLIST_H__

#include <list>
#include "IHealthMonitor.h"

typedef std::list<IHealthMonitor*> HealthMonitorList;
typedef std::list<IHealthMonitor*>::iterator HealthMonitorListIterator;
typedef std::list<IHealthMonitor*>::const_iterator HealthMonitorListConstIterator;

#endif // __HEALTHMONITORLIST_H__
