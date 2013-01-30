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

#ifndef MockLayerList_H_
#define MockLayerList_H_

#include "Scene.h"
#include <list>
#include <gmock/gmock.h>  // Brings in Google Mock.

class MockLayerList : public Scene {
 public:
  MOCK_METHOD1(createLayer, Layer*(unsigned int));
  MOCK_METHOD1(createSurface, Surface*(unsigned int));
  MOCK_METHOD1(removeLayer, bool(Layer*));
  MOCK_METHOD1(removeSurface, bool(Surface*));
  MOCK_CONST_METHOD1(getLayer, Layer*(unsigned int));
  MOCK_CONST_METHOD1(getSurface, Surface*(unsigned int));
  MOCK_CONST_METHOD2(getLayerIDs, void(unsigned int*,unsigned int**));
  MOCK_CONST_METHOD3(getLayerIDsOfScreen, bool(unsigned int,unsigned int*,unsigned int**));
  MOCK_CONST_METHOD2(getSurfaceIDs, void(unsigned int*,unsigned int**));
  MOCK_METHOD0(lockScene, void());
  MOCK_METHOD0(unlockScene, void());
};

#endif /* MockLayerList_H_ */
