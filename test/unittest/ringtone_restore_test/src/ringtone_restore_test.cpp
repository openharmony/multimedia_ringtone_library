/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define MLOG_TAG "RingtoneRestoreTest"

#include "ringtone_restore_test.h"

#define private public
#define protected public
#include "ringtone_restore.h"
#include "ringtone_restore_base.h"
#include "ringtone_restore_napi.h"
#include "ringtone_dualfw_restore.h"
#undef protected
#undef private
#include "ringtone_file_utils.h"
#include "ringtone_source.h"
#include "ringtone_restore_factory.h"
#include "ringtone_unittest_utils.h"
#include "ringtone_rdbstore.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ability_context_impl.h"
#include "datashare_helper.h"
#include "iservice_registry.h"
#include "ringtone_restore_type.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
constexpr int STORAGE_MANAGER_MANAGER_ID = 5003;
static const int32_t INVALID_QUERY_OFFSET = -1;
const string TEST_BACKUP_PATH = "/data/test/backup";
const string TEST_BACKUP_DB_PATH = TEST_BACKUP_PATH + "/data/storage/el2/database/rdb/ringtone_library.db";
const string LOCAL_DB_PATH =
    "/data/app/el2/100/database/com.ohos.ringtonelibrary.ringtonelibrarydata/rdb/ringtone_library.db";

unique_ptr<RingtoneRestore> g_restoreService = nullptr;
RingtoneSource g_ringtoneSource;
shared_ptr<RingtoneUnistore> g_rdbStore;
shared_ptr<DataShare::DataShareHelper> g_ringtoneshare;

const vector<string> CLEAR_SQLS = {
    "DELETE FROM " + RINGTONE_TABLE,
};

void ExecuteSqls(shared_ptr<NativeRdb::RdbStore> store, const vector<string> &sqls)
{
    for (const auto &sql : sqls) {
        int32_t errCode = store->ExecuteSql(sql);
        if (errCode == E_OK) {
            continue;
        }
        RINGTONE_ERR_LOG("Execute %{public}s failed: %{public}d", sql.c_str(), errCode);
    }
}

void ClearData()
{
    RINGTONE_INFO_LOG("Start clear data");
    ExecuteSqls(g_restoreService->localRdb_, CLEAR_SQLS);
    system("rm -rf /storage/media/local/files/Ringtone/alarms/*");
    system("rm -rf /storage/media/local/files/Ringtone/notifications/*");
    system("rm -rf /storage/media/local/files/Ringtone/ringtones/*");
    RINGTONE_INFO_LOG("End clear data");
}

void ClearCloneSource(RingtoneSource &source, const string &dbPath)
{
    source.restoreRdbPtr_ = nullptr;
    NativeRdb::RdbHelper::DeleteRdbStore(dbPath);
}

void Init(RingtoneSource &source, const string &path)
{
    RINGTONE_INFO_LOG("Start init clone source database");
    source.Init(path, LOCAL_DB_PATH);
}

shared_ptr<NativeRdb::ResultSet> GetResultSet(shared_ptr<NativeRdb::RdbStore> rdbStore, const string &querySql)
{
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("rdbStore is nullptr");
        return nullptr;
    }
    return rdbStore->QuerySql(querySql);
}

void QueryInt(shared_ptr<NativeRdb::RdbStore> rdbStore, const string &querySql, const string &columnName,
    int32_t &result)
{
    ASSERT_NE(rdbStore, nullptr);
    auto resultSet = rdbStore->QuerySql(querySql);
    ASSERT_NE(resultSet, nullptr);
    ASSERT_EQ(resultSet->GoToFirstRow(), E_OK);
    result = GetInt32Val(columnName, resultSet);
    RINGTONE_INFO_LOG("Query %{public}s result: %{public}d", querySql.c_str(), result);
}

void RingtoneRestoreTest::SetUpTestCase(void)
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remoteObj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    g_ringtoneshare = DataShare::DataShareHelper::Creator(remoteObj, "datashare:///ringtone");
    Init(g_ringtoneSource, TEST_BACKUP_DB_PATH);
    g_restoreService = make_unique<RingtoneRestore>();
    g_restoreService->localRdb_ = g_ringtoneSource.localRdbPtr_; // local database
    g_restoreService->restoreRdb_ = g_ringtoneSource.restoreRdbPtr_; // source database
    g_restoreService->settingMgr_ = make_unique<RingtoneSettingManager>(g_ringtoneSource.localRdbPtr_);
}

void RingtoneRestoreTest::TearDownTestCase(void)
{
    g_ringtoneshare->Release();
    ClearData();
    g_restoreService->localRdb_ = nullptr;
    g_restoreService->restoreRdb_ = nullptr;
    g_restoreService->settingMgr_ = nullptr;
    NativeRdb::RdbHelper::DeleteRdbStore(TEST_BACKUP_DB_PATH);
}

void RingtoneRestoreTest::SetUp()
{
    system("mkdir /data/local/tmp/test");
    system("mkdir /data/local/tmp/testMove");
    system("mkdir -p /data/local/tmp/test/data/storage/el2/database/rdb");
    system("touch /data/local/tmp/test/data/storage/el2/database/rdb/ringtone_library.db");
}

void RingtoneRestoreTest::TearDown()
{
    system("rm -rf /data/local/tmp/test");
    system("rm -rf /data/local/tmp/testMove");
}

HWTEST_F(RingtoneRestoreTest, ringtone_restore_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_restore_test_0001 start");
    ClearData();
    auto infos = g_restoreService->QueryFileInfos(INVALID_QUERY_OFFSET);
    EXPECT_EQ(infos.empty(), false);
    EXPECT_NE(infos.size(), 0);
    const int32_t offset = 1;
    infos = g_restoreService->QueryFileInfos(offset);
    EXPECT_EQ(infos.empty(), false);
    EXPECT_NE(infos.size(), 0);
    RINGTONE_INFO_LOG("ringtone_restore_test_0001 end");
}

HWTEST_F(RingtoneRestoreTest, ringtone_restore_test_0002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_restore_test_0002 start");
    ClearData();
    auto infos = g_restoreService->QueryFileInfos(INVALID_QUERY_OFFSET);
    EXPECT_EQ(infos.empty(), false);
    EXPECT_NE(infos.size(), 0);
    string alarmsDir = g_restoreService->GetRestoreDir(ToneType::TONE_TYPE_ALARM);
    EXPECT_EQ(alarmsDir, RINGTONE_RESTORE_DIR + "/alarms");
    string notificationsDir = g_restoreService->GetRestoreDir(ToneType::TONE_TYPE_NOTIFICATION);
    EXPECT_EQ(notificationsDir, RINGTONE_RESTORE_DIR + "/notifications");
    string ringtonesDir = g_restoreService->GetRestoreDir(ToneType::TONE_TYPE_RINGTONE);
    EXPECT_EQ(ringtonesDir, RINGTONE_RESTORE_DIR + "/ringtones");
    RINGTONE_INFO_LOG("ringtone_restore_test_0002 end");
}

HWTEST_F(RingtoneRestoreTest, ringtone_restore_test_0003, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_restore_test_0003 start");
    ClearData();
    auto infos = g_restoreService->QueryFileInfos(INVALID_QUERY_OFFSET);
    EXPECT_EQ(infos.empty(), false);
    EXPECT_NE(infos.size(), 0);
    g_restoreService->backupPath_ = TEST_BACKUP_PATH;
    vector<NativeRdb::ValuesBucket> values = g_restoreService->MakeInsertValues(infos);
    int64_t rowNum = 0;
    int32_t errCode = g_restoreService->BatchInsert(RINGTONE_TABLE, values, rowNum);
    EXPECT_EQ(errCode, Media::E_OK);
    RINGTONE_INFO_LOG("ringtone_restore_test_0003 end");
}

HWTEST_F(RingtoneRestoreTest, ringtone_restore_test_0004, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_restore_test_0004 start");
    int32_t ringtoneCount = RingtoneRestoreDbUtils::QueryRingtoneCount(g_restoreService->localRdb_);
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = nullptr;
    ringtoneCount = RingtoneRestoreDbUtils::QueryRingtoneCount(rdbStore);
    EXPECT_EQ(ringtoneCount, 0);
    RINGTONE_INFO_LOG("ringtone_restore_test_0004 end");
}

HWTEST_F(RingtoneRestoreTest, restore_MoveDirectory_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("restore_MoveDirectory_test_0001 start");
    g_restoreService->StartRestore();
    auto infos = g_restoreService->QueryFileInfos(INVALID_QUERY_OFFSET);
    EXPECT_EQ(infos.empty(), false);
    EXPECT_NE(infos.size(), 0);
    g_restoreService->InsertTones(infos);
    string oldPath = "/data/local/tmp/test/movefile_001.ogg";
    string srcDir = "/data/local/tmp/test/";
    string dstDir = "/data/local/tmp/testMove/";
    EXPECT_EQ(RingtoneFileUtils::CreateFile(oldPath), E_SUCCESS);
    auto ret = g_restoreService->MoveDirectory(srcDir, dstDir);
    EXPECT_EQ(ret, E_OK);
    RINGTONE_INFO_LOG("restore_MoveDirectory_test_0001 end");
}

HWTEST_F(RingtoneRestoreTest, restore_OnPrepare_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("restore_OnPrepare_test_0001 start");
    const string backupPath;
    auto ret = g_restoreService->Init(backupPath);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    g_restoreService->restoreRdb_ = nullptr;
    g_restoreService->StartRestore();
    int errCode = 0;
    NativeRdb::RdbStoreConfig localRdbConfig(LOCAL_DB_PATH);
    RingtoneLocalRdbOpenCb localDbHelper;
    NativeRdb::RdbHelper::GetRdbStore(localRdbConfig, 1, localDbHelper, errCode);
    g_restoreService->restoreRdb_ = g_ringtoneSource.restoreRdbPtr_;
    g_restoreService->backupPath_.clear();
    g_restoreService->StartRestore();
    g_restoreService->backupPath_ = TEST_BACKUP_PATH;
    FileInfo info;
    info.title = TITLE_DEFAULT;
    g_restoreService->UpdateRestoreFileInfo(info);
    string oldPath = "/data/local/tmp/test/movefile_001.ogg";
    EXPECT_EQ(RingtoneFileUtils::CreateFile(oldPath), E_SUCCESS);
    info.data = oldPath;
    vector<FileInfo> infos;
    infos.push_back(info);
    g_restoreService->CheckRestoreFileInfos(infos);
    const std::string destPath = "/restore_Init_test_0001/data";
    bool result = g_restoreService->OnPrepare(info, destPath);
    EXPECT_EQ(result, false);
    info.data = "restore_OnPrepare_test_0001";
    result = g_restoreService->OnPrepare(info, oldPath);
    EXPECT_EQ(result, false);
    info.data = "createfile_001./data";
    result = g_restoreService->OnPrepare(info, oldPath);
    EXPECT_EQ(result, false);
    RINGTONE_INFO_LOG("restore_OnPrepare_test_0001 end");
}

HWTEST_F(RingtoneRestoreTest, restore_GetRestoreDir_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("restore_GetRestoreDir_test_0001 start");
    const string backupPath = "/data/local/tmp/test";
    auto val = g_restoreService->Init(backupPath);
    EXPECT_EQ(val, E_OK);
    FileInfo fileInfo;
    auto ret = g_restoreService->SetInsertValue(fileInfo);
    fileInfo.restorePath = "restore_MoveDirectory_test_0001";
    ret = g_restoreService->SetInsertValue(fileInfo);
    std::vector<FileInfo> fileInfos;
    g_restoreService->localRdb_ = nullptr;
    g_restoreService->InsertTones(fileInfos);
    g_restoreService->localRdb_ = g_ringtoneSource.restoreRdbPtr_;
    int32_t toneType = static_cast<int32_t>(ToneType::TONE_TYPE_MAX);
    auto result = g_restoreService->GetRestoreDir(toneType);
    string strEmpty = {};
    EXPECT_EQ(result, strEmpty);
    RINGTONE_INFO_LOG("restore_GetRestoreDir_test_0001 end");
}

HWTEST_F(RingtoneRestoreTest, restore_GetColumnInfoMap_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("restore_GetColumnInfoMap_test_0001 start");
    int32_t changeRows = 0;
    NativeRdb::ValuesBucket values;
    const string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    std::unique_ptr<NativeRdb::AbsRdbPredicates> predicates =
        make_unique<NativeRdb::AbsRdbPredicates>(RINGTONE_TABLE);
    int32_t ret = RingtoneRestoreDbUtils::Update(g_restoreService->localRdb_, changeRows, values, predicates);
    EXPECT_EQ(ret, E_OK);
    std::shared_ptr<NativeRdb::RdbStore> rdbStore = nullptr;
    ret = RingtoneRestoreDbUtils::Update(rdbStore, changeRows, values, predicates);
    EXPECT_EQ(ret, E_FAIL);
    auto result = RingtoneRestoreDbUtils::GetColumnInfoMap(g_restoreService->localRdb_, RINGTONE_TABLE);
    std::unordered_map<std::string, std::string> columnInfoMap;
    EXPECT_NE(result, columnInfoMap);
    result = RingtoneRestoreDbUtils::GetColumnInfoMap(rdbStore, RINGTONE_TABLE);
    EXPECT_EQ(result, columnInfoMap);
    RINGTONE_INFO_LOG("restore_GetColumnInfoMap_test_0001 end");
}

HWTEST_F(RingtoneRestoreTest, restore_TransactionCommit_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("restore_TransactionCommit_test_0001 start");
    unique_ptr<RingtoneRdbTransaction> rdbTransaction =
        make_unique<RingtoneRdbTransaction>(g_restoreService->localRdb_);
    auto ret = rdbTransaction->Start();
    EXPECT_EQ(ret, E_SUCCESS);
    ret = rdbTransaction->Start();
    EXPECT_EQ(ret, E_SUCCESS);
    ret = rdbTransaction->BeginTransaction();
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
    rdbTransaction->isInTransaction_.store(false);
    ret = rdbTransaction->TransactionCommit();
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
    rdbTransaction->isInTransaction_.store(true);
    rdbTransaction->rdbStore_ = nullptr;
    ret = rdbTransaction->TransactionCommit();
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
    rdbTransaction->rdbStore_ = g_restoreService->localRdb_;
    ret = rdbTransaction->TransactionCommit();
    EXPECT_EQ(ret, E_SUCCESS);
    RINGTONE_INFO_LOG("restore_TransactionCommit_test_0001 end");
}

HWTEST_F(RingtoneRestoreTest, restore_TransactionRollback_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("restore_TransactionRollback_test_0001 start");
    unique_ptr<RingtoneRdbTransaction> rdbTransaction =
        make_unique<RingtoneRdbTransaction>(g_restoreService->localRdb_);
    rdbTransaction->Finish();
    auto ret = rdbTransaction->Start();
    EXPECT_EQ(ret, E_SUCCESS);
    rdbTransaction->Finish();
    rdbTransaction->Finish();
    rdbTransaction->rdbStore_ = nullptr;
    rdbTransaction->BeginTransaction();
    rdbTransaction->TransactionRollback();
    rdbTransaction->rdbStore_ = g_restoreService->localRdb_;
    ret = rdbTransaction->Start();
    EXPECT_EQ(ret, E_SUCCESS);
    ret = rdbTransaction->TransactionRollback();
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
    RINGTONE_INFO_LOG("restore_TransactionRollback_test_0001 end");
}

HWTEST_F(RingtoneRestoreTest, restore_JSStartRestore_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_restore_test_0004 start");
    RestoreSceneType type = RestoreSceneType::RESTORE_SCENE_TYPE_SINGLE_CLONE;
    auto ret = RingtoneRestoreFactory::CreateObj(type);
    EXPECT_NE(ret, nullptr);
    type = RestoreSceneType::RESTORE_SCENE_TYPE_DUAL_CLONE;
    ret = RingtoneRestoreFactory::CreateObj(type);
    EXPECT_NE(ret, nullptr);
    type = RestoreSceneType::RESTORE_SCENE_TYPE_DUAL_UPGRADE;
    ret = RingtoneRestoreFactory::CreateObj(type);
    EXPECT_NE(ret, nullptr);
    RINGTONE_INFO_LOG("ringtone_restore_test_0004 end");
}
} // namespace Media
} // namespace OHOS