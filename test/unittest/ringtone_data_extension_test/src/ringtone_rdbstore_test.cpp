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
#define MLOG_TAG "FileExtUnitTest"

#include "ringtone_rdbstore_test.h"

#include "ability_context_impl.h"
#include "rdb_store.h"
#include "rdb_utils.h"
#include "ringtone_errno.h"
#define private public
#include "ringtone_rdbstore.h"
#undef private

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
shared_ptr<RingtoneUnistore> g_uniStore = nullptr;
const std::string CREATE_RINGTONE_TABLE = "CREATE TABLE IF NOT EXISTS " + RINGTONE_TABLE + "(" +
    RINGTONE_COLUMN_TONE_ID                       + " INTEGER  PRIMARY KEY AUTOINCREMENT, " +
    RINGTONE_COLUMN_DATA                          + " TEXT              , " +
    RINGTONE_COLUMN_SIZE                          + " BIGINT   DEFAULT 0, " + ")";

void RingtoneRdbStorelUnitTest::SetUpTestCase()
{
}

void RingtoneRdbStorelUnitTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneRdbStorelUnitTest::SetUp()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    g_uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = g_uniStore->Init();
    EXPECT_EQ(ret, E_OK);
}

void RingtoneRdbStorelUnitTest::TearDown(void) {}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Insert_test_001, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "test name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    const string data = "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    cmd.SetValueBucket(values);
    g_uniStore->Init();
    int64_t rowId = E_HAS_DB_ERROR;
    int32_t ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Insert_test_002, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "rdbStore_Insert_test_002";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    const string displayname = "rdbStore_Insert_test_002/test";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, displayname);
    cmd.SetValueBucket(values);
    int64_t rowId = E_HAS_DB_ERROR;
    int32_t ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Insert_test_003, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    int64_t rowId = E_HAS_DB_ERROR;
    int32_t ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Insert_test_004, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "rdbStore_Insert_test_004";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    cmd.SetValueBucket(values);
    g_uniStore->Stop();
    int64_t rowId = E_HAS_DB_ERROR;
    int32_t ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Query_test_001, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    vector<string> columns;
    columns.push_back(RINGTONE_COLUMN_DISPLAY_NAME);
    g_uniStore->Init();
    auto queryResultSet = g_uniStore->Query(cmd, columns);
    EXPECT_NE(queryResultSet, nullptr);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Query_test_002, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    vector<string> columns;
    columns.push_back(RINGTONE_COLUMN_TITLE);
    g_uniStore->Stop();
    auto queryResultSet = g_uniStore->Query(cmd, columns);
    EXPECT_EQ(queryResultSet, nullptr);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Delete_test_001, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    int32_t rowId = 1;

    DataShare::DataSharePredicates predicates;
    NativeRdb::RdbPredicates rdbPredicate = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, RINGTONE_TABLE);
    cmd.GetAbsRdbPredicates()->SetWhereClause(rdbPredicate.GetWhereClause());
    cmd.GetAbsRdbPredicates()->SetWhereArgs(rdbPredicate.GetWhereArgs());

    g_uniStore->Init();
    int32_t ret = g_uniStore->Delete(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Delete_test_002, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    g_uniStore->Stop();
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    int32_t rowId = 1;
    int32_t ret = g_uniStore->Delete(cmd, rowId);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Update_test_001, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::UPDATE);
    NativeRdb::ValuesBucket valuesBucket;
    const string title = "rdbStore_Update_test_001";
    valuesBucket.PutString(RINGTONE_COLUMN_TITLE, title);
    cmd.SetValueBucket(valuesBucket);
    g_uniStore->Init();
    int32_t rowId = E_HAS_DB_ERROR;
    int32_t ret = g_uniStore->Update(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Update_test_002, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::UPDATE);
    int32_t updatedRows = -1;
    int32_t ret = g_uniStore->Update(cmd, updatedRows);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_Update_test_003, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    g_uniStore->Stop();
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::UPDATE);
    int32_t rowId = E_HAS_DB_ERROR;
    int32_t ret = g_uniStore->Update(cmd, rowId);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_ExecuteSql_test_001, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    const string modifySql = "UPDATE " + RINGTONE_TABLE + " SET ";
    int32_t ret = g_uniStore->ExecuteSql(modifySql);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_ExecuteSql_test_002, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    g_uniStore->Stop();
    const string modifySql = "UPDATE " + RINGTONE_TABLE + " SET ";
    int32_t ret = g_uniStore->ExecuteSql(modifySql);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_QuerySql_test_001, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    g_uniStore->Init();
    auto queryResultSet = g_uniStore->QuerySql(CREATE_RINGTONE_TABLE);
    EXPECT_NE(queryResultSet, nullptr);
    EXPECT_NE(g_uniStore->GetRaw(), nullptr);
}

HWTEST_F(RingtoneRdbStorelUnitTest, rdbStore_QuerySql_test_002, TestSize.Level0)
{
    if (g_uniStore == nullptr) {
        exit(1);
    }
    g_uniStore->Stop();
    auto queryResultSet = g_uniStore->QuerySql(CREATE_RINGTONE_TABLE);
    EXPECT_EQ(queryResultSet, nullptr);
}

HWTEST_F(RingtoneRdbStorelUnitTest, dataCallBack_OnCreate_test_001, TestSize.Level0)
{
    RingtoneDataCallBack rdbDataCallBack;
    g_uniStore->Init();
    auto ret = rdbDataCallBack.OnCreate(*g_uniStore->GetRaw());
    EXPECT_EQ(ret, E_OK);;
    int32_t oldVersion = 0;
    int32_t newVersion = 1;
    ret = rdbDataCallBack.OnUpgrade(*g_uniStore->GetRaw(), oldVersion, newVersion);
    EXPECT_EQ(ret, E_OK);
}
} // namespace Media
} // namespace OHOS
