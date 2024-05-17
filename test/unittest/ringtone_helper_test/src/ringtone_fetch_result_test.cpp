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

#include "ringtone_fetch_result_test.h"

#include "ability_context_impl.h"
#include "datashare_helper.h"
#include "iservice_registry.h"
#include "ringtone_asset.h"
#include "ringtone_data_manager.h"
#include "ringtone_errno.h"
#define private public
#include "ringtone_fetch_result.h"
#undef private
#include "ringtone_type.h"
#include "ringtone_log.h"


using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const string RINGTONE_LIBRARY_PATH = "/storage/cloud/files/Ringtone";
const string MTP_FORMAT_MP3 = ".mp3"; // MP3 audio files
const string TEST_INSERT_RINGTONE_LIBRARY = "test_insert_ringtone_library";
const int TEST_RINGTONE_COLUMN_SIZE = 1022;
const int TEST_RINGTONE_COLUMN_TONE_TYPE = 2;
const string SLASH_STR = "/";
const string MP3 = "mp3";
shared_ptr<RingtoneFetchResult<RingtoneAsset>> g_fetchResult;
void RingtoneFetchResultTest::SetUpTestCase()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    auto ret = RingtoneDataManager::GetInstance()->Init(abilityContextImpl);
    CHECK_AND_RETURN_LOG(ret == E_OK, "InitMediaLibraryMgr failed, ret: %{public}d", ret);
    std::shared_ptr<DataShare::DataShareResultSet> dataShare = make_shared<DataShare::DataShareResultSet>();
    g_fetchResult = make_shared<RingtoneFetchResult<RingtoneAsset>>(dataShare);
    EXPECT_NE(g_fetchResult, nullptr);
    system("rm -rf /storage/cloud/files/");
    system("mkdir /storage/cloud/files");
    system("mkdir /storage/cloud/files/Ringtone");
}

void RingtoneFetchResultTest::TearDownTestCase()
{
    system("rm -rf /storage/cloud/files/");
    auto dataManager = RingtoneDataManager::GetInstance();
    EXPECT_NE(dataManager, nullptr);
    dataManager->ClearRingtoneDataMgr();
}

// SetUp:Execute before each test case
void RingtoneFetchResultTest::SetUp() {}

void RingtoneFetchResultTest::TearDown(void) {}

HWTEST_F(RingtoneFetchResultTest, fetchResult_IsAtLastRow_test_001, TestSize.Level0)
{
    auto ringtoneAsset = g_fetchResult->GetFirstObject();
    EXPECT_EQ(ringtoneAsset, nullptr);
    bool ret = g_fetchResult->IsAtLastRow();
    EXPECT_EQ(ret, false);
    shared_ptr<RingtoneFetchResult<RingtoneAsset>> fetchResult = make_shared<RingtoneFetchResult<RingtoneAsset>>();
    ret = fetchResult->IsAtLastRow();
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtoneFetchResultTest, fetchResult_GetRowValFromColumn_test_001, TestSize.Level0)
{
    auto ringtoneAsset = g_fetchResult->GetNextObject();
    EXPECT_EQ(ringtoneAsset, nullptr);
    string columnName;
    RingtoneResultSetDataType dataType = RingtoneResultSetDataType::DATA_TYPE_STRING;
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    auto ret = g_fetchResult->GetRowValFromColumn(columnName, dataType, resultSet);
    variant<int32_t, int64_t, string, double> cellValue = "";
    EXPECT_EQ(ret, cellValue);
    shared_ptr<RingtoneFetchResult<RingtoneAsset>> fetchResult = make_shared<RingtoneFetchResult<RingtoneAsset>>();
    dataType = RingtoneResultSetDataType::DATA_TYPE_INT64;
    ret = fetchResult->GetRowValFromColumn(columnName, dataType, resultSet);
    variant<int32_t, int64_t, string, double> longVal = static_cast<int64_t>(0);
    EXPECT_EQ(ret, longVal);
    dataType = RingtoneResultSetDataType::DATA_TYPE_DOUBLE;
    ret = fetchResult->GetRowValFromColumn(columnName, dataType, resultSet);
    variant<int32_t, int64_t, string, double> doubleVal = 0;
    EXPECT_EQ(ret, doubleVal);
    fetchResult->Close();
}

HWTEST_F(RingtoneFetchResultTest, fetchResult_GetValByIndex_test_001, TestSize.Level0)
{
    auto ringtoneAsset = g_fetchResult->GetLastObject();
    EXPECT_EQ(ringtoneAsset, nullptr);
    int32_t index = 0;
    RingtoneResultSetDataType dataType = RingtoneResultSetDataType::DATA_TYPE_STRING;
    shared_ptr<NativeRdb::ResultSet> resultSet;
    auto ret = g_fetchResult->GetValByIndex(index, dataType, resultSet);
    variant<int32_t, int64_t, string, double> stringVal = "";
    EXPECT_EQ(ret, stringVal);
    dataType = RingtoneResultSetDataType::DATA_TYPE_INT32;
    ret = g_fetchResult->GetValByIndex(index, dataType, resultSet);
    variant<int32_t, int64_t, string, double> integerVal = 0;
    EXPECT_EQ(ret, integerVal);
    dataType = RingtoneResultSetDataType::DATA_TYPE_INT64;
    ret = g_fetchResult->GetValByIndex(index, dataType, resultSet);
    variant<int32_t, int64_t, string, double> longVal = static_cast<int64_t>(0);
    EXPECT_EQ(ret, longVal);
    dataType = RingtoneResultSetDataType::DATA_TYPE_DOUBLE;
    ret = g_fetchResult->GetValByIndex(index, dataType, resultSet);
    variant<int32_t, int64_t, string, double> doubleVal = 0.0;
    EXPECT_EQ(ret, doubleVal);
    dataType = RingtoneResultSetDataType::DATA_TYPE_NULL;
    ret = g_fetchResult->GetValByIndex(index, dataType, resultSet);
    variant<int32_t, int64_t, string, double> cellValue ;
    EXPECT_EQ(ret, cellValue);
    shared_ptr<RingtoneFetchResult<RingtoneAsset>> fetchResult = make_shared<RingtoneFetchResult<RingtoneAsset>>();
    shared_ptr<NativeRdb::ResultSet> nativeRdbResultSet = nullptr;
    fetchResult->GetValByIndex(index, dataType, nativeRdbResultSet);
    EXPECT_EQ(ret, cellValue);
}

HWTEST_F(RingtoneFetchResultTest, fetchResult_GetObjectFromRdb_test_001, TestSize.Level0)
{
    int32_t index = 0;
    shared_ptr<NativeRdb::ResultSet> resultSet;
    auto ringtoneAsset = g_fetchResult->GetObjectFromRdb(resultSet, index);
    EXPECT_EQ(ringtoneAsset, nullptr);
    auto ringtoneObject = g_fetchResult->GetObject();
    EXPECT_NE(ringtoneObject, nullptr);
    shared_ptr<RingtoneFetchResult<RingtoneAsset>> fetchResult = make_shared<RingtoneFetchResult<RingtoneAsset>>();
    RingtoneAsset *asset = nullptr;
    shared_ptr<NativeRdb::ResultSet> nativeRdbResultSet = nullptr;
    fetchResult->SetRingtoneAsset(asset, nativeRdbResultSet);
    ringtoneAsset = fetchResult->GetObjectFromRdb(resultSet, index);
    EXPECT_EQ(ringtoneAsset, nullptr);
}

HWTEST_F(RingtoneFetchResultTest, fetchResult_GetObjectAtPosition_test_001, TestSize.Level0)
{
    int32_t index = 0;
    auto ringtoneObject = g_fetchResult->GetObjectAtPosition(index);
    EXPECT_EQ(ringtoneObject, nullptr);
    auto resultSet = g_fetchResult->GetDataShareResultSet();
    EXPECT_NE(resultSet, nullptr);
}

HWTEST_F(RingtoneFetchResultTest, fetchResult_GetRowValFromColumn_test_002, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    valuesBucket.Put(RINGTONE_COLUMN_DATA,
        static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(1) + MTP_FORMAT_MP3));
    valuesBucket.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    valuesBucket.Put(RINGTONE_COLUMN_TONE_TYPE, static_cast<int>(TEST_RINGTONE_COLUMN_TONE_TYPE));
    valuesBucket.Put(RINGTONE_COLUMN_MIME_TYPE, MP3); // todo construct by us
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    auto dataManager = RingtoneDataManager::GetInstance();
    EXPECT_NE(dataManager, nullptr);
    auto retVal = dataManager->Insert(cmd, valuesBucket);
    EXPECT_EQ((retVal > 0), true);

    int errCode = 0;
    RingtoneDataCommand cmdQuery(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_MIME_TYPE, MP3);
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, { RINGTONE_COLUMN_SIZE } };
    auto queryResultSet = dataManager->Query(cmdQuery, columns, queryPredicates, errCode);
    EXPECT_NE((queryResultSet == nullptr), true);
    shared_ptr<AbilityRuntime::DataShareResultSet> resultSet =
        make_shared<AbilityRuntime::DataShareResultSet>(queryResultSet);
    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    cout << "query count = " << to_string(results->GetCount()) << endl;
    RingtoneResultSetDataType dataType = RingtoneResultSetDataType::DATA_TYPE_DOUBLE;
    shared_ptr<NativeRdb::ResultSet> nativeRdbResultSet;
    results->GetRowValFromColumn(RINGTONE_COLUMN_DATA, dataType, nativeRdbResultSet);
    int32_t index = 0;
    auto ringtoneObject = results->GetObjectAtPosition(index);
    EXPECT_NE(ringtoneObject, nullptr);
    results->Close();

    DataShare::DataSharePredicates predicates;
    RingtoneDataCommand cmdDelete(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    EXPECT_NE(dataManager, nullptr);
    retVal = dataManager->Delete(cmdDelete, predicates);
    EXPECT_EQ((retVal > 0), true);
}

HWTEST_F(RingtoneFetchResultTest, fetchResult_GetNextObject_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataShare::DataShareValuesBucket valuesBucket;
    valuesBucket.Put(RINGTONE_COLUMN_DATA,
        static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_MP3));
    valuesBucket.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    valuesBucket.Put(RINGTONE_COLUMN_TONE_TYPE, static_cast<int>(TEST_RINGTONE_COLUMN_TONE_TYPE));
    valuesBucket.Put(RINGTONE_COLUMN_MIME_TYPE, MP3); // todo construct by us
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    auto dataManager = RingtoneDataManager::GetInstance();
    EXPECT_NE(dataManager, nullptr);
    auto retVal = dataManager->Insert(cmd, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    DataShare::DataShareValuesBucket valuesBucketInsert;
    valuesBucketInsert.Put(RINGTONE_COLUMN_DATA,
        static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(1) + MTP_FORMAT_MP3));
    retVal = dataManager->Insert(cmd, valuesBucketInsert);
    EXPECT_EQ((retVal > 0), true);

    int errCode = 0;
    RingtoneDataCommand cmdQuery(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_MIME_TYPE, MP3);
    vector<string> columns = { { RINGTONE_COLUMN_DATA } };
    auto queryResultSet = dataManager->Query(cmdQuery, columns, queryPredicates, errCode);
    EXPECT_NE((queryResultSet == nullptr), true);
    shared_ptr<AbilityRuntime::DataShareResultSet> resultSet =
        make_shared<AbilityRuntime::DataShareResultSet>(queryResultSet);
    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    cout << "query count = " << to_string(results->GetCount()) << endl;
    auto ringtoneObject = results->GetNextObject();
    EXPECT_NE(ringtoneObject, nullptr);
    ringtoneObject = results->GetFirstObject();
    EXPECT_NE(ringtoneObject, nullptr);
    ringtoneObject = results->GetLastObject();
    EXPECT_NE(ringtoneObject, nullptr);
    results->Close();

    DataShare::DataSharePredicates predicates;
    RingtoneDataCommand cmdDelete(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    retVal = dataManager->Delete(cmdDelete, predicates);
    EXPECT_EQ((retVal > 0), true);
}
} // namespace Media
} // namespace OHOS