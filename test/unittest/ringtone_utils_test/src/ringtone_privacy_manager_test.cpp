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

#include "ringtone_privacy_manager_test.h"

#include "ringtone_errno.h"
#define private public
#include "ringtone_privacy_manager.h"
#undef private

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const std::string STORAGE_MEDIA_DIR = "/storage/cloud/files/";
const string DEFAULT_STR = "";

void RingtonePrivacyManagerTest::SetUpTestCase() {}

void RingtonePrivacyManagerTest::TearDownTestCase() {}

void RingtonePrivacyManagerTest::SetUp() {}

void RingtonePrivacyManagerTest::TearDown(void) {}

HWTEST_F(RingtonePrivacyManagerTest, ringtonePrivacyManager_Open_test_001, TestSize.Level0)
{
    RingtonePrivacyManager ringtonePrivacyManager(DEFAULT_STR, DEFAULT_STR);
    int32_t ret = ringtonePrivacyManager.Open();
    EXPECT_EQ(ret, E_INVALID_PATH);
}

HWTEST_F(RingtonePrivacyManagerTest, ringtonePrivacyManager_Open_test_002, TestSize.Level0)
{
    const string path = ".Open";
    const string mode = "Z";
    RingtonePrivacyManager ringtonePrivacyManager(path, mode);
    int32_t ret = ringtonePrivacyManager.Open();
    EXPECT_EQ(ret, E_INVALID_PATH);
}

HWTEST_F(RingtonePrivacyManagerTest, ringtonePrivacyManager_Open_test_003, TestSize.Level0)
{
    const string path = ".ogg";
    RingtonePrivacyManager ringtonePrivacyManager(path, DEFAULT_STR);
    int32_t ret = ringtonePrivacyManager.Open();
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(RingtonePrivacyManagerTest, ringtonePrivacyManager_Open_test_004, TestSize.Level0)
{
    const string path = ".ogg";
    const string mode = "w";
    RingtonePrivacyManager ringtonePrivacyManager(path, mode);
    int32_t ret = ringtonePrivacyManager.Open();
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(RingtonePrivacyManagerTest, ringtonePrivacyManager_Open_test_005, TestSize.Level0)
{
    const string path = "/storage/cloud/files/open.ogg";
    const string mode = "w";
    RingtonePrivacyManager ringtonePrivacyManager(path, mode);
    int32_t ret = ringtonePrivacyManager.Open();
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(RingtonePrivacyManagerTest, ringtonePrivacyManager_Open_test_006, TestSize.Level0)
{
    const string mode = "w";
    RingtonePrivacyManager ringtonePrivacyManager(DEFAULT_STR, mode);
    int32_t ret = ringtonePrivacyManager.Open();
    EXPECT_EQ(ret, E_INVALID_PATH);
}

HWTEST_F(RingtonePrivacyManagerTest, ringtonePrivacyManager_Open_test_007, TestSize.Level0)
{
    const string mode = "w";
    RingtonePrivacyManager ringtonePrivacyManager(STORAGE_MEDIA_DIR, mode);
    int32_t ret = ringtonePrivacyManager.Open();
    EXPECT_EQ(ret, E_INVALID_PATH);
}
} // namespace Media
} // namespace OHOS
