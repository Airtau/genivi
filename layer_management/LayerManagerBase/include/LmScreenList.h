/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#ifndef _LMSCREENLIST_H_
#define _LMSCREENLIST_H_

#include <list>
#include "LmScreen.h"

typedef std::list<LmScreen*> LmScreenList;
typedef std::list<LmScreen*>::iterator LmScreenListIterator;
typedef std::list<LmScreen*>::const_iterator LmScreenListConstIterator;
typedef std::list<LmScreen*>::reverse_iterator LmScreenListReverseIterator;
typedef std::list<LmScreen*>::const_reverse_iterator LmScreenListConstReverseIterator;

#endif /* _LMSCREENLIST_H_ */
