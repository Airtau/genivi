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
#include "IpcModuleLoader.h"
#include <gtest/gtest.h>

#define PLATFORM_PTR_SIZE sizeof(unsigned int*)

class Loopback : public ::testing::Test
{
public:
    void SetUp()
    {
        loadAndCheckIpcModule(&mService);
        //loadAndCheckIpcModule(&mClient);
    }

    void loadAndCheckIpcModule(IpcModule* ipcModule)
    {
        memset(ipcModule, 0, sizeof(*ipcModule));
        ASSERT_EQ(ILM_TRUE, loadIpcModule(ipcModule));

        int apiEntryCount = sizeof(*ipcModule) / PLATFORM_PTR_SIZE;
        unsigned int* base = (unsigned int*)ipcModule;
        for (int i = 0; i < apiEntryCount; ++i)
        {
            ASSERT_NE(0u, base[i]);
        }
    }

    void TearDown()
    {
        //memset(&mClient, 0, sizeof(mClient));
        memset(&mService, 0, sizeof(mService));
    }

protected:
    IpcModule mService;
    //IpcModule mClient;
};

TEST_F(Loopback, DISABLE_lifecycle)
{
    ASSERT_TRUE(mService.initClientMode());
    //ASSERT_TRUE(mClient.init(ILM_TRUE));

    //ASSERT_TRUE(mClient.destroy());
    ASSERT_TRUE(mService.destroy());
}
