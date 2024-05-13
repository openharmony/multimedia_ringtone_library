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
#undef protected
#undef private
#include "ringtone_source.h"
#include "ringtone_restore_factory.h"
#include "ringtone_unittest_utils.h"
#include "ringtone_rdbstore.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ability_context_impl.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
static const int32_t INVALID_QUERY_OFFSET = -1;
const string TEST_BACKUP_PATH = "/data/test/backup";
const string TEST_BACKUP_DB_PATH = TEST_BACKUP_PATH + "/data/storage/el2/database/rdb/ringtone_library.db";
const string LOCAL_DB_PATH =
    "/data/app/el2/100/database/com.ohos.ringtonelibrary.ringtonelibrarydata/rdb/ringtone_library.db";

unique_ptr<RingtoneRestore> restoreService = nullptr;
RingtoneSource ringtoneSource;
shared_ptr<RingtoneUnistore> g_rdbStore;

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
    ExecuteSqls(restoreService->localRdb_, CLEAR_SQLS);
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

void RingtoneRestoreTest::SetUpTestCase(void) {
    Init(ringtoneSource, TEST_BACKUP_DB_PATH);
    restoreService = make_unique<RingtoneRestore>();
    restoreService->localRdb_ = ringtoneSource.localRdbPtr_; // local database
    restoreService->restoreRdb_ = ringtoneSource.restoreRdbPtr_; // source database
    restoreService->settingMgr_ = make_unique<RingtoneSettingManager>(ringtoneSource.localRdbPtr_);
}

void RingtoneRestoreTest::TearDownTestCase(void) {
    ClearData();
    restoreService->localRdb_ = nullptr;
    restoreService->restoreRdb_ = nullptr;
    restoreService->settingMgr_ = nullptr;
    NativeRdb::RdbHelper::DeleteRdbStore(TEST_BACKUP_DB_PATH);
}

void RingtoneRestoreTest::SetUp() {}

void RingtoneRestoreTest::TearDown() {}

HWTEST_F(RingtoneRestoreTest, ringtone_restore_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_restore_test_0001 start");
    ClearData();
    auto infos = restoreService->QueryFileInfos(INVALID_QUERY_OFFSET);
    EXPECT_EQ(infos.empty(), false);
    EXPECT_NE(infos.size(), 0);
    RINGTONE_INFO_LOG("ringtone_restore_test_0001 end");
}

HWTEST_F(RingtoneRestoreTest, ringtone_restore_test_0002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_restore_test_0002 start");
    ClearData();
    auto infos = restoreService->QueryFileInfos(INVALID_QUERY_OFFSET);
    EXPECT_EQ(infos.empty(), false);
    EXPECT_NE(infos.size(), 0);
    string alarmsDir = restoreService->GetRestoreDir(ToneType::TONE_TYPE_ALARM);
    EXPECT_EQ(alarmsDir, RINGTONE_RESTORE_DIR + "/alarms");
    string notificationsDir = restoreService->GetRestoreDir(ToneType::TONE_TYPE_NOTIFICATION);
    EXPECT_EQ(notificationsDir, RINGTONE_RESTORE_DIR + "/notifications");
    string ringtonesDir = restoreService->GetRestoreDir(ToneType::TONE_TYPE_RINGTONE);
    EXPECT_EQ(ringtonesDir, RINGTONE_RESTORE_DIR + "/ringtones");
    RINGTONE_INFO_LOG("ringtone_restore_test_0002 end");
}

HWTEST_F(RingtoneRestoreTest, ringtone_restore_test_0003, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_restore_test_0003 start");
    ClearData();
    auto infos = restoreService->QueryFileInfos(INVALID_QUERY_OFFSET);
    EXPECT_EQ(infos.empty(), false);
    EXPECT_NE(infos.size(), 0);
    restoreService->backupPath_ = TEST_BACKUP_PATH;
    vector<NativeRdb::ValuesBucket> values = restoreService->MakeInsertValues(infos);
    EXPECT_EQ(values.size(), infos.size());
    int64_t rowNum = 0;
    int32_t errCode = restoreService->BatchInsert(RINGTONE_TABLE, values, rowNum);
    EXPECT_EQ(errCode, Media::E_OK);
    RINGTONE_INFO_LOG("ringtone_restore_test_0003 end");
}

HWTEST_F(RingtoneRestoreTest, ringtone_restore_test_0004, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_restore_test_0004 start");
    int32_t ringtoneCount = RingtoneRestoreDbUtils::QueryRingtoneCount(restoreService->localRdb_);
    EXPECT_EQ(ringtoneCount, 4);
    RINGTONE_INFO_LOG("ringtone_restore_test_0004 end");
}
} // namespace Media
} // namespace OHOS