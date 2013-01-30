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
#ifndef LAYERMAP_H_
#define LAYERMAP_H_

#include <map>
#include "Layer.h"

typedef std::map<unsigned int, Layer*> LayerMap;
typedef std::map<unsigned int, Layer*>::iterator LayerMapIterator;
typedef std::map<unsigned int, Layer*>::const_iterator LayerMapConstIterator;

#endif /* LAYERMAP_H_ */
