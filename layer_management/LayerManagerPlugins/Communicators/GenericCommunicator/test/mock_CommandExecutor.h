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

#ifndef MockCommandExecutor_H_
#define MockCommandExecutor_H_

#include "GraphicalSurface.h"
#include "ICommand.h"
#include "ICommandExecutor.h"
#include "Configuration.h"
#include <list>
#include <gmock/gmock.h>  // Brings in Google Mock.
using ::testing::DefaultValue;
class MockCommandExecutor : public ICommandExecutor {
 public:
  MOCK_METHOD1(execute, bool(ICommand* commandToBeExecuted));

  MOCK_METHOD0(startManagement, bool());
  MOCK_METHOD0(stopManagement, bool());

  MOCK_METHOD0(getScene, Scene*());

  MOCK_METHOD0(getRendererList, RendererList*());
  MOCK_METHOD0(getCommunicatorList, CommunicatorList*());
  MOCK_METHOD0(getSceneProviderList, SceneProviderList*());
  MOCK_METHOD0(getHealthMonitorList, HealthMonitorList*());

  MOCK_METHOD0(getApplicationReferenceMap, ApplicationReferenceMap*());
  MOCK_METHOD2(addApplicationReference, void(t_ilm_client_handle client, IApplicationReference* reference));
  MOCK_METHOD1(removeApplicationReference, void(t_ilm_client_handle client));
  MOCK_METHOD1(getSenderPid, t_ilm_uint(t_ilm_client_handle client));
  MOCK_METHOD1(getSenderName, const char*(t_ilm_client_handle client));
  MOCK_METHOD1(getSenderName, const char*(unsigned int pid));

  MOCK_CONST_METHOD1(getLayerTypeCapabilities, uint(LayerType));
  MOCK_CONST_METHOD1(getNumberOfHardwareLayers, uint(uint));
  MOCK_CONST_METHOD1(getScreenResolution, uint*(uint));
  MOCK_CONST_METHOD1(getScreenIDs, uint*(uint*));

  MOCK_METHOD2(addClientNotification, void(GraphicalObject* object, t_ilm_notification_mask mask));
  MOCK_METHOD0(getClientNotificationQueue, NotificationQueue&());

  MOCK_METHOD0(getHealth, HealthCondition());

  MOCK_METHOD1(getEnqueuedCommands, CommandList&(unsigned int clientPid));
};

#endif /* MockCommandExecutor_H_ */
