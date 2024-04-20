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

#include "ringtone_permission_utils_test.h"

#include <algorithm>

#define private public
#include "permission_utils.h"
#undef private
#include "ringtone_errno.h"
#include "ringtone_log.h"

using std::string;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const string DEFAULT_STR = "";

void RingtonePermissionUtilsTest::SetUpTestCase() {}

void RingtonePermissionUtilsTest::TearDownTestCase() {}
void RingtonePermissionUtilsTest::SetUp() {}
void RingtonePermissionUtilsTest::TearDown(void) {}

HWTEST_F(RingtonePermissionUtilsTest, ringtonePermissionUtils_CheckCallerPermission_test_001, TestSize.Level0)
{
    bool ret = RingtonePermissionUtils::CheckCallerPermission(DEFAULT_STR);
    EXPECT_EQ(ret, false);
    string permissions = PERM_WRITE_RINGTONE;
    ret = RingtonePermissionUtils::CheckCallerPermission(permissions);
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtonePermissionUtilsTest, ringtonePermissionUtils_CheckCallerPermission_test_002, TestSize.Level0)
{
    std::vector<string> permissionVec = { DEFAULT_STR, PERM_WRITE_RINGTONE };
    bool ret = RingtonePermissionUtils::CheckCallerPermission(permissionVec);
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtonePermissionUtilsTest, ringtonePermissionUtils_GetSysBundleManager_test_001, TestSize.Level0)
{
    auto ret = RingtonePermissionUtils::GetSysBundleManager();
    EXPECT_NE(ret, nullptr);
}

HWTEST_F(RingtonePermissionUtilsTest, ringtonePermissionUtils_CheckHasPermission_test_001, TestSize.Level0)
{
    std::vector<string> perms = { DEFAULT_STR, PERM_WRITE_RINGTONE };
    bool ret = RingtonePermissionUtils::CheckHasPermission(perms);
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtonePermissionUtilsTest, ringtonePermissionUtils_GetClientBundle_test_001, TestSize.Level0)
{
    string bundleName = "audio";
    const int uid = 0;
    RingtonePermissionUtils::GetClientBundle(uid, bundleName);
    EXPECT_EQ(bundleName, DEFAULT_STR);
}

HWTEST_F(RingtonePermissionUtilsTest, ringtonePermissionUtils_GetTokenId_test_001, TestSize.Level0)
{
    uint32_t toneId = RingtonePermissionUtils::GetTokenId();
    EXPECT_EQ((toneId > 0), true);
}

HWTEST_F(RingtonePermissionUtilsTest, ringtonePermissionUtils_IsSystemApp_test_001, TestSize.Level0)
{
    bool ret = RingtonePermissionUtils::IsSystemApp();
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtonePermissionUtilsTest, ringtonePermissionUtils_CheckIsSystemAppByUid_test_001, TestSize.Level0)
{
    bool ret = RingtonePermissionUtils::CheckIsSystemAppByUid();
    EXPECT_EQ(ret, true);
}

HWTEST_F(RingtonePermissionUtilsTest, ringtonePermissionUtils_GetPackageNameByBundleName_test_001, TestSize.Level0)
{
    const string bundleName = "/com.ohos.ringtonelibrary.ringtonelibrarydata";
    string packageName = RingtonePermissionUtils::GetPackageNameByBundleName(bundleName);
    EXPECT_EQ(packageName, DEFAULT_STR);
}
} // namespace Media
} // namespace OHOS
