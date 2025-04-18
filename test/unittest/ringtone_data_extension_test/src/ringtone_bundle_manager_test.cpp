/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ringtone_bundle_manager_test.h"

#define private public
#include "ringtone_bundle_manager.h"
#include "ringtone_data_command.h"
#include "ipc_skeleton.h"
#include "ringtone_errno.h"
#undef private


using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
shared_ptr<RingtoneBundleManager> g_bundleManager = nullptr;

void RingtoneBundleManagerTest::SetUpTestCase()
{
    g_bundleManager = RingtoneBundleManager::GetInstance();
    EXPECT_NE(g_bundleManager, nullptr);
}
void RingtoneBundleManagerTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneBundleManagerTest::SetUp() {}

void RingtoneBundleManagerTest::TearDown(void) {}

HWTEST_F(RingtoneBundleManagerTest, bundleManager_GetClientBundleName_test_001, TestSize.Level0)
{
    auto ret = g_bundleManager->GetClientBundleName();
    EXPECT_EQ(ret, "");
    g_bundleManager->Clear();
    g_bundleManager = RingtoneBundleManager::GetInstance();
    EXPECT_NE(g_bundleManager, nullptr);
}

HWTEST_F(RingtoneBundleManagerTest, dataCommand_GetBundleName_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    const string bundleName =  "dataCommand_GetBundleName_test_001";
    cmd.SetBundleName(bundleName);
    auto ret = cmd.GetBundleName();
    EXPECT_EQ(ret, bundleName);
}

HWTEST_F(RingtoneBundleManagerTest, dataCommand_GetResult_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    const string result =  "dataCommand_GetResult_test_001";
    cmd.SetResult(result);
    auto ret = cmd.GetResult();
    EXPECT_EQ(ret, result);
    auto type = cmd.GetOprnType();
    EXPECT_EQ(type, RingtoneOperationType::INSERT);
}

HWTEST_F(RingtoneBundleManagerTest, dataCommand_GetClientBundleName_test_002, TestSize.Level0)
{
    int32_t uid = 0;
    auto& cacheList = g_bundleManager->cacheList_;
    auto& cacheMap = g_bundleManager->cacheMap_;
    cacheList.emplace_front(0, "bundle1");
    cacheList.emplace_front(1, "bundle2");
    cacheMap[uid] = cacheList.begin();
    auto ret = g_bundleManager->GetClientBundleName();
    EXPECT_EQ(ret, "bundle2");
    g_bundleManager->Clear();
    g_bundleManager = RingtoneBundleManager::GetInstance();
    EXPECT_NE(g_bundleManager, nullptr);
}
} // namespace Media
} // namespace OHOS
