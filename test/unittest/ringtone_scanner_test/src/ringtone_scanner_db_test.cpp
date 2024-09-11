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
    abilityContextImpl->SetStageContext(stageContext);
    g_uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
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
    EXPECT_EQ(ret, E_OK);
    struct stat statInfo;
    metadata.SetData(ROOT_MEDIA_DIR);
    metadata.SetDisplayName(RingtoneScannerUtils::GetFileNameFromUri(ROOT_MEDIA_DIR));
    metadata.SetTitle(RingtoneScannerUtils::GetFileTitle(metadata.GetDisplayName()));
    metadata.SetMediaType(static_cast<RingtoneMediaType>(RINGTONE_MEDIA_TYPE_AUDIO));
    metadata.SetSize(statInfo.st_size);
    metadata.SetDateModified(statInfo.st_mtime);
    const int64_t DATE_TAKEN = 1;
    metadata.SetDateTaken(DATE_TAKEN);
    ret = ringtoneScannerDb.InsertMetadata(metadata, tableName);
    EXPECT_EQ(ret, E_OK);
    const int64_t DATE_ADD = 1;
    metadata.SetDateAdded(DATE_ADD);
    ret = ringtoneScannerDb.InsertMetadata(metadata, tableName);
    EXPECT_EQ(ret, E_OK);
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
    metadata->SetData(ROOT_MEDIA_DIR);
    metadata->SetDisplayName(RingtoneScannerUtils::GetFileNameFromUri(ROOT_MEDIA_DIR));
    metadata->SetTitle(RingtoneScannerUtils::GetFileTitle(metadata->GetDisplayName()));
    metadata->SetMediaType(static_cast<RingtoneMediaType>(RINGTONE_MEDIA_TYPE_AUDIO));
    g_uniStore->Stop();
    int ret = ringtoneScannerDb.GetFileBasicInfo(path, metadata);
    EXPECT_EQ(ret, E_DB_FAIL);
}
} // namespace Media
} // namespace OHOS
