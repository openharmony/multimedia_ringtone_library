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

#include "ringtone_scanner_db_test.h"

#include <string>

#include "ability_context_impl.h"
#include "rdb_helper.h"
#include "ringtone_data_manager.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"
#define private public
#include "ringtone_scanner_db.h"
#undef private
#include "ringtone_unittest_utils.h"
#include "ringtone_scanner_utils.h"
#include "ringtone_type.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
class ConfigTestOpenCall : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    static const string CREATE_TABLE_TEST;
};

const string ConfigTestOpenCall::CREATE_TABLE_TEST = string("CREATE TABLE IF NOT EXISTS test ") +
    "(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, age INTEGER, salary REAL, blobType BLOB)";

int ConfigTestOpenCall::OnCreate(NativeRdb::RdbStore &store)
{
    return store.ExecuteSql(CREATE_TABLE_TEST);
}

int ConfigTestOpenCall::OnUpgrade(NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    return 0;
}
shared_ptr<RingtoneUnistore> g_uniStore = nullptr;

void RingtoneScannerDbTest::SetUpTestCase() {}

void RingtoneScannerDbTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneScannerDbTest::SetUp()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    EXPECT_NE(abilityContextImpl, nullptr);
    abilityContextImpl->SetStageContext(stageContext);
    g_uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    EXPECT_NE(g_uniStore, nullptr);
    int32_t ret = g_uniStore->Init();
    EXPECT_EQ(ret, E_OK);
}

void RingtoneScannerDbTest::TearDown(void) {}

HWTEST_F(RingtoneScannerDbTest, scannerDb_QueryRingtoneRdb_test_001, TestSize.Level0)
{
    RingtoneScannerDb ringtoneScannerDb;
    string whereClause = {};
    vector<string> whereArgs = {};
    vector<string> columns;
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    int ret = ringtoneScannerDb.QueryRingtoneRdb(whereClause, whereArgs, columns, resultSet);
    EXPECT_EQ(ret, E_OK);
    g_uniStore->Stop();
    ret = ringtoneScannerDb.QueryRingtoneRdb(whereClause, whereArgs, columns, resultSet);
    EXPECT_EQ(ret, E_RDB);
}

HWTEST_F(RingtoneScannerDbTest, scannerDb_InsertMetadata_test_001, TestSize.Level0)
{
    RingtoneScannerDb ringtoneScannerDb;
    RingtoneMetadata metadata;
    string tableName;
    g_uniStore->Stop();
    int32_t ret = ringtoneScannerDb.InsertMetadata(metadata, tableName);
    // stop g_uniStore will cause insert failed
    EXPECT_EQ(ret, E_DB_FAIL);
    struct stat statInfo;
    metadata.SetData(STORAGE_FILES_DIR);
    metadata.SetDisplayName(RingtoneScannerUtils::GetFileNameFromUri(STORAGE_FILES_DIR));
    metadata.SetTitle(RingtoneScannerUtils::GetFileTitle(metadata.GetDisplayName()));
    metadata.SetMediaType(static_cast<RingtoneMediaType>(RINGTONE_MEDIA_TYPE_AUDIO));
    metadata.SetSize(statInfo.st_size);
    metadata.SetDateModified(statInfo.st_mtime);
    const int64_t DATE_TAKEN = 1;
    metadata.SetDateTaken(DATE_TAKEN);
    ret = ringtoneScannerDb.InsertMetadata(metadata, tableName);
    EXPECT_EQ(ret, E_DB_FAIL);
    const int64_t DATE_ADD = 1;
    metadata.SetDateAdded(DATE_ADD);
    ret = ringtoneScannerDb.InsertMetadata(metadata, tableName);
    EXPECT_EQ(ret, E_DB_FAIL);
}

HWTEST_F(RingtoneScannerDbTest, scannerDb_UpdateMetadata_test_001, TestSize.Level0)
{
    RingtoneScannerDb ringtoneScannerDb;
    RingtoneMetadata metadata;
    string tableName;
    g_uniStore->Stop();
    int32_t ret = ringtoneScannerDb.UpdateMetadata(metadata, tableName);
    EXPECT_EQ(ret, E_RDB);
}

HWTEST_F(RingtoneScannerDbTest, scannerDb_UpdateRingtoneRdb_test_001, TestSize.Level0)
{
    RingtoneScannerDb ringtoneScannerDb;
    NativeRdb::ValuesBucket valuesBucket;
    string whereClause = {};
    vector<string> whereArgs = {};
    int ret = ringtoneScannerDb.UpdateRingtoneRdb(valuesBucket, whereClause, whereArgs);
    EXPECT_EQ(ret, E_DB_FAIL);
    g_uniStore->Stop();
    ret = ringtoneScannerDb.UpdateRingtoneRdb(valuesBucket, whereClause, whereArgs);
    EXPECT_EQ(ret, E_RDB);
}

HWTEST_F(RingtoneScannerDbTest, scannerDb_GetFileBasicInfo_test_001, TestSize.Level0)
{
    RingtoneScannerDb ringtoneScannerDb;
    const string path = "scannerDb_GetFileBasicInfo_test_001";
    unique_ptr<RingtoneMetadata> metadata = make_unique<RingtoneMetadata>();
    metadata->SetData(STORAGE_FILES_DIR);
    metadata->SetDisplayName(RingtoneScannerUtils::GetFileNameFromUri(STORAGE_FILES_DIR));
    metadata->SetTitle(RingtoneScannerUtils::GetFileTitle(metadata->GetDisplayName()));
    metadata->SetMediaType(static_cast<RingtoneMediaType>(RINGTONE_MEDIA_TYPE_AUDIO));
    g_uniStore->Stop();
    int ret = ringtoneScannerDb.GetFileBasicInfo(path, metadata);
    EXPECT_EQ(ret, E_DB_FAIL);
}

/*
 * Feature: Service
 * Function: Test ringtoneScannerDb with UpdateScannerFlag
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test UpdateScannerFlag for Normal branches
 */
HWTEST_F(RingtoneScannerDbTest, scannerDb_UpdateScannerFlag_test_001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scannerDb_UpdateScannerFlag_test_001 start.");
    RingtoneScannerDb ringtoneScannerDb;
    g_uniStore->ExecuteSql("INSERT INTO " + RINGTONE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/alarms/Adara.ogg'," +
        " 10414, 'Adara.ogg', 'Adara', 2, 0, 'audio/ogg', 2, 1505707241000, 1505707241846, 1505707241," +
        " 600, 0, -1, 0, -1, 0, -1, 1, 2, '1', 1)");
    bool res = ringtoneScannerDb.UpdateScannerFlag();
    EXPECT_EQ(res, true);

    string whereClause = RINGTONE_COLUMN_DISPLAY_NAME + " = ?";
    vector<string> whereArgs;
    whereArgs.push_back("Adara.ogg");
    vector<string> columns = {RINGTONE_COLUMN_SCANNER_FLAG};
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    int ret = ringtoneScannerDb.QueryRingtoneRdb(whereClause, whereArgs, columns, resultSet);
    ASSERT_NE(resultSet, nullptr);
    EXPECT_EQ(ret, E_OK);
    int scannerFlag;
    resultSet->GetInt(0, scannerFlag);
    EXPECT_EQ(scannerFlag, 0);
    RINGTONE_INFO_LOG("scannerDb_UpdateScannerFlag_test_001 end.");
}

/*
 * Feature: Service
 * Function: Test ringtoneScannerDb with UpdateScannerFlag
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test UpdateScannerFlag for Normal branches
 */
HWTEST_F(RingtoneScannerDbTest, scannerDb_DeleteNotExist_test_001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scannerDb_DeleteNotExist_test_001 start.");
    RingtoneScannerDb ringtoneScannerDb;
    g_uniStore->ExecuteSql("INSERT INTO " + RINGTONE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/ringtones/Carme.ogg'," +
        " 26177, 'RingtoneTest.ogg', 'RingtoneTest', 2, 1, 'audio/ogg', 1, 1505707241000, 1505707241846, 1505707241," +
        " 1242, 0, -1, 0, -1, 3, 2, 0, -1, '1', 0)");
    string whereClause = RINGTONE_COLUMN_DISPLAY_NAME + " = ?";
    vector<string> whereArgs = {};
    whereArgs.push_back("RingtoneTest.ogg");
    vector<string> columns = {RINGTONE_COLUMN_SCANNER_FLAG};
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    int ret = ringtoneScannerDb.QueryRingtoneRdb(whereClause, whereArgs, columns, resultSet);
    ASSERT_NE(resultSet, nullptr);
    EXPECT_EQ(ret, E_OK);
    int scannerFlag;
    resultSet->GetInt(0, scannerFlag);
    EXPECT_EQ(scannerFlag, 0);

    bool res = ringtoneScannerDb.DeleteNotExist();
    EXPECT_EQ(res, true);
    resultSet = nullptr;
    ret = ringtoneScannerDb.QueryRingtoneRdb(whereClause, whereArgs, columns, resultSet);
    ASSERT_NE(resultSet, nullptr);
    EXPECT_EQ(ret, E_OK);
    int rowCount = 1;
    resultSet->GetRowCount(rowCount);
    EXPECT_EQ(rowCount, 0);
    RINGTONE_INFO_LOG("scannerDb_DeleteNotExist_test_001 end.");
}

/*
 * Feature: Service
 * Function: Test ringtoneScannerDb with QueryRingtoneRdb
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test QueryRingtoneRdb when column is not exist
 */
HWTEST_F(RingtoneScannerDbTest, scannerDb_QueryRingtoneRdb_test_002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scannerDb_QueryRingtoneRdb_test_002 start.");
    RingtoneScannerDb ringtoneScannerDb;
    g_uniStore->ExecuteSql("INSERT INTO " + RINGTONE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/ringtones/Carme.ogg'," +
        " 26177, 'Ringtone.ogg', 'RingtoneTest', 2, 1, 'audio/ogg', 1, 1505707241000, 1505707241846, 1505707241," +
        " 1242, 0, -1, 0, -1, 3, 2, 0, -1, '1', 0)");
    string whereClause = "test = ?";
    vector<string> whereArgs = {};
    whereArgs.push_back("Ringtone.ogg");
    vector<string> columns = {"test"};
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    int ret = ringtoneScannerDb.QueryRingtoneRdb(whereClause, whereArgs, columns, resultSet);
    EXPECT_EQ(ret, E_OK);
    RINGTONE_INFO_LOG("scannerDb_QueryRingtoneRdb_test_002 end.");
}

/*
 * Feature: Service
 * Function: Test ringtoneScannerDb with InsertVibrateMetadata
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test InsertVibrateMetadata for normal branches
 */
HWTEST_F(RingtoneScannerDbTest, scannerDb_InsertVibrateMetadata_test_001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("scannerDb_InsertVibrateMetadata_test_001 start.");
    RingtoneScannerDb ringtoneScannerDb;
    VibrateMetadata* metadata = new (std::nothrow) VibrateMetadata();
    ASSERT_NE(metadata, nullptr);
    metadata->SetData("test");
    metadata->SetDateAdded(static_cast<int64_t>(0));
    metadata->SetDateTaken(static_cast<int64_t>(0));
    metadata->SetDateModified(static_cast<int64_t>(0));
    std::string tableName = VIBRATE_TABLE;
    int32_t ret = ringtoneScannerDb.InsertVibrateMetadata(*metadata, tableName);
    EXPECT_NE(ret, 0);

    metadata->SetDateTaken(static_cast<int64_t>(1));
    ret = ringtoneScannerDb.InsertVibrateMetadata(*metadata, tableName);
    EXPECT_NE(ret, 0);
    RINGTONE_INFO_LOG("scannerDb_InsertVibrateMetadata_test_001 end.");
}
} // namespace Media
} // namespace OHOS
