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

#ifndef MockCommunicator_H_
#define MockCommunicator_H_

#include "AbstractCommunicator.h"
#include <list>
#include <gmock/gmock.h>  // Brings in Google Mock.
class MockCommunicator : public AbstractCommunicator {
 public:
     MockCommunicator(ICommandExecutor* executor, Scene* pScene) : AbstractCommunicator(executor,pScene){};
  MOCK_METHOD0(start, bool());
  MOCK_METHOD0(stop, void());
  MOCK_METHOD1(setdebug, void(bool onoff));

};

#endif /* MockCommunicator_H_ */
