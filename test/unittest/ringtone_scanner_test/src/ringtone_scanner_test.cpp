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

#include "ringtone_scanner_test.h"

#include <string>

#include "ability_context_impl.h"
#include "get_self_permissions.h"
#include "rdb_store.h"
#include "rdb_utils.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"
#define private public
#include "ringtone_scanner.h"
#undef private
#include "ringtone_scanner_db.h"
#include "ringtone_scanner_utils.h"
#include "ringtone_unittest_utils.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
void RingtoneScannerTest::SetUpTestCase()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);

    vector<string> perms;
    perms.push_back("ohos.permission.WRITE_RINGTONE");

    uint64_t tokenId = 0;
    RingtonePermissionUtilsUnitTest::SetAccessTokenPermission("RingtoneDataManagerUnitTest", perms, tokenId);
    ASSERT_TRUE(tokenId != 0);
}

void RingtoneScannerTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneScannerTest::SetUp() {}

void RingtoneScannerTest::TearDown(void) {}

HWTEST_F(RingtoneScannerTest, scanner_Scan_test_001, TestSize.Level0)
{
    const string path = "/storage/media";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(path, callback, RingtoneScannerObj::FILE);
    ringtoneScannerObj.Scan();
    RingtoneScannerObj ringtoneScannerObjThree(path, callback, RingtoneScannerObj::DIRECTORY);
    ringtoneScannerObjThree.Scan();
    shared_ptr<bool> flag = make_shared<bool>();
    ringtoneScannerObjThree.SetStopFlag(flag);
    int type = 3;
    RingtoneScannerObj ringtoneScannerObjTwo(path, callback, static_cast<RingtoneScannerObj::ScanType>(type));
    ringtoneScannerObjTwo.Scan();
    RingtoneScannerObj ringtoneScannerObjTest(path, callback, RingtoneScannerObj::START);
    int ret = ringtoneScannerObjTest.GetFileMetadata();
    ringtoneScannerObjTest.Scan();
    EXPECT_NE(ret, E_OK);
}

HWTEST_F(RingtoneScannerTest, scanner_Scan_Vibrate_test_001, TestSize.Level0)
{
    const string path = "/sys_prod/resource/media";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(path, callback, RingtoneScannerObj::FILE);
    ringtoneScannerObj.Scan();
    RingtoneScannerObj ringtoneScannerObjThree(path, callback, RingtoneScannerObj::DIRECTORY);
    ringtoneScannerObjThree.Scan();
    shared_ptr<bool> flag = make_shared<bool>();
    ringtoneScannerObjThree.SetStopFlag(flag);
    RingtoneScannerObj ringtoneScannerObjTest(path, callback, RingtoneScannerObj::START);
    int ret = ringtoneScannerObjTest.GetFileMetadata();
    ringtoneScannerObjTest.Scan();
    EXPECT_NE(ret, E_OK);
}

HWTEST_F(RingtoneScannerTest, scanner_ScanFileInTraversal_test_001, TestSize.Level0)
{
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(STORAGE_FILES_DIR, callback, RingtoneScannerObj::DIRECTORY);
    int32_t ret = ringtoneScannerObj.ScanFileInTraversal(STORAGE_FILES_DIR);
    EXPECT_NE(ret, E_FILE_HIDDEN);
    const string path = "scanner_ScanDirInternal_test_001/.test";
    ret = ringtoneScannerObj.ScanFileInTraversal(path);
    EXPECT_EQ(ret, E_FILE_HIDDEN);
}

HWTEST_F(RingtoneScannerTest, scanner_WalkFileTree_test_001, TestSize.Level0)
{
    string dir = "/storage/cloud/files";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    const int errorPath = 4096;
    const string path(errorPath, 'a');
    RingtoneScannerObj ringtoneScannerObj(dir, callback, RingtoneScannerObj::DIRECTORY);
    int32_t ret = ringtoneScannerObj.WalkFileTree(path);
    EXPECT_EQ(ret, ERR_INCORRECT_PATH);
    dir = "/storage/media";
    shared_ptr<bool> flag = make_shared<bool>(true);
    ringtoneScannerObj.SetStopFlag(flag);
    ret = ringtoneScannerObj.WalkFileTree(dir);
    EXPECT_EQ(ret, E_STOP);
}

HWTEST_F(RingtoneScannerTest, scanner_AddToTransaction_test_001, TestSize.Level0)
{
    const string dir = "/storage/cloud/files";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(dir, callback, RingtoneScannerObj::FILE);
    const int count = 5;
    for (int i = 0; i < count; i++) {
        ringtoneScannerObj.data_ = make_unique<RingtoneMetadata>();
        ringtoneScannerObj.data_->SetToneId(i);
        auto ret = ringtoneScannerObj.AddToTransaction();
        EXPECT_EQ(ret, E_OK);
    }
}

HWTEST_F(RingtoneScannerTest, scanner_CommitTransaction_test_001, TestSize.Level0)
{
    const string dir = "/storage/cloud/files";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(dir, callback, RingtoneScannerObj::FILE);
    ringtoneScannerObj.GetFileMetadata();
    int32_t ret = ringtoneScannerObj.CommitTransaction();
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneScannerTest, scanner_BuildFileInfo_test_001, TestSize.Level0)
{
    const string dir = "/storage";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(dir, callback, RingtoneScannerObj::FILE);
    RingtoneScannerObj ringtoneScannerObjTwo(RingtoneScannerObj::FILE);
    int32_t ret = ringtoneScannerObj.GetFileMetadata();
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    ret = ringtoneScannerObj.BuildFileInfo();
    ringtoneScannerObj.WaitFor();
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneScannerTest, scanner_ScanDir_test_001, TestSize.Level0)
{
    const string dir = "./scanner_ScanDir_test_001";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(dir, callback, RingtoneScannerObj::DIRECTORY);
    int32_t ret = ringtoneScannerObj.ScanDir();
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
    ringtoneScannerObj.dir_ = "scanner_ScanDir_test_001/.test";
    ret = ringtoneScannerObj.ScanDir();
    EXPECT_EQ(ret, E_DIR_HIDDEN);
}

/*
 * Feature: Service
 * Function: Test RingtoneScanner with BuildVibrateData
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test BuildVibrateData for abnormal branches
 */
HWTEST_F(RingtoneScannerTest, scanner_BuildVibrateData_test_001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scanner_BuildVibrateData_test_001 start.");
    struct stat statInfo = {};
    statInfo.st_mode = S_IFDIR | 0755;
    const string dir = "./scanner_ScanDir_test_001";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(dir, callback, RingtoneScannerObj::DIRECTORY);
    int32_t ret = ringtoneScannerObj.BuildVibrateData(statInfo);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    RINGTONE_INFO_LOG("scanner_BuildVibrateData_test_001 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneScanner with ScanFileInternal
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test ScanFileInternal for abnormal branches
 */
HWTEST_F(RingtoneScannerTest, scanner_BuildVibrateData_test_002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scanner_BuildVibrateData_test_002 start.");
    struct stat statInfo = {};
    statInfo.st_mode = S_IFDIR | 0755;
    const string dir = "./scanner_ScanDir_test_001";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(dir, callback, RingtoneScannerObj::DIRECTORY);
    ringtoneScannerObj.path_ = "/sys_prod/resource/media/haptics/map.json";
    int32_t ret = ringtoneScannerObj.ScanFileInternal();
    EXPECT_FALSE(ringtoneScannerObj.isVibrateFile_);

    ringtoneScannerObj.path_ = "/sys_prod/resource/media/haptics/map.ogg";
    ret = ringtoneScannerObj.ScanFileInternal();
    EXPECT_EQ(ret, E_INVALID_PATH);
    RINGTONE_INFO_LOG("scanner_BuildVibrateData_test_002 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneScanner with ScanVibrateFile
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test ScanVibrateFile when path is empty
 */
HWTEST_F(RingtoneScannerTest, scanner_ScanVibrateFile_test_001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scanner_ScanVibrateFile_test_001 start.");
    const string dir = "./scanner_ScanDir_test_001";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(dir, callback, RingtoneScannerObj::DIRECTORY);
    ringtoneScannerObj.path_ = "";
    int32_t ret = ringtoneScannerObj.ScanVibrateFile();
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    RINGTONE_INFO_LOG("scanner_ScanVibrateFile_test_001 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneScanner with Commit
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test Commit when scan vibrate files
 */
HWTEST_F(RingtoneScannerTest, scanner_Commit_test_001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scanner_Commit_test_001 start.");
    const string dir = "./scanner_ScanDir_test_001";
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(dir, callback, RingtoneScannerObj::DIRECTORY);
    ringtoneScannerObj.isVibrateFile_ = true;
    ringtoneScannerObj.vibrateData_ = make_unique<VibrateMetadata>();
    ASSERT_NE(ringtoneScannerObj.vibrateData_, nullptr);
    ringtoneScannerObj.vibrateData_->SetVibrateId(FILE_ID_DEFAULT);
    int32_t ret = ringtoneScannerObj.Commit();
    EXPECT_EQ(ret, E_OK);

    ringtoneScannerObj.vibrateData_->SetVibrateId(static_cast<int32_t>(64));
    ret = ringtoneScannerObj.Commit();
    EXPECT_EQ(ret, E_OK);
    RINGTONE_INFO_LOG("scanner_Commit_test_001 end.");
}

HWTEST_F(RingtoneScannerTest, scanner_BootScan_test_001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scanner_BootScan_test_001 start.");
    RingtoneScannerObj ringtoneScannerObj(RingtoneScannerObj::START);
    std::shared_ptr<RingtoneUnistore> rdbStore = RingtoneRdbStore::GetInstance();
    int32_t res = ringtoneScannerObj.BootScan();
    EXPECT_EQ(res, E_OK);
    RINGTONE_INFO_LOG("scanner_BootScan_test_001 end.");
}

HWTEST_F(RingtoneScannerTest, scanner_ScanFileInTraversal_test_002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scanner_ScanFileInTraversal_test_002 start.");
    shared_ptr<IRingtoneScannerCallback> callback = nullptr;
    RingtoneScannerObj ringtoneScannerObj(STORAGE_FILES_DIR, callback, RingtoneScannerObj::DIRECTORY);
    int32_t ret = ringtoneScannerObj.ScanFileInTraversal(STORAGE_FILES_DIR);
    EXPECT_NE(ret, E_FILE_HIDDEN);
    const string path = "/sys_prod/variant/region_comm/china/resource/media/haptics/test.ogg";
    ret = ringtoneScannerObj.ScanFileInTraversal(path);
    EXPECT_EQ(ret, E_OK);
    RINGTONE_INFO_LOG("scanner_ScanFileInTraversal_test_002 start.");
}
} // namespace Media
} // namespace OHOS