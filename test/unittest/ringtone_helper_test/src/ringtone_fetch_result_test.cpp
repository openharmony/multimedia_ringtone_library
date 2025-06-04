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
#include "ringtone_rdbstore.h"
#include "ringtone_scanner_db.h"


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
const string RAINNING = "rainning";
const int NUMBER_OF_TIMES = 10;
shared_ptr<RingtoneFetchResult<RingtoneAsset>> g_fetchResult;
shared_ptr<RingtoneUnistore> g_uniStore = nullptr;
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

    g_uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    EXPECT_NE(g_uniStore, nullptr);
    ret = g_uniStore->Init();
    EXPECT_EQ(ret, E_OK);
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
    unique_ptr<RingtoneAsset> asset = nullptr;
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
    valuesBucket.Put(RINGTONE_COLUMN_MEDIA_TYPE, static_cast<int>(RINGTONE_MEDIA_TYPE_AUDIO));
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
    valuesBucket.Put(RINGTONE_COLUMN_MEDIA_TYPE, static_cast<int>(RINGTONE_MEDIA_TYPE_AUDIO));
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    auto dataManager = RingtoneDataManager::GetInstance();
    EXPECT_NE(dataManager, nullptr);
    auto retVal = dataManager->Insert(cmd, valuesBucket);
    EXPECT_EQ((retVal > 0), true);
    DataShare::DataShareValuesBucket valuesBucketInsert;
    valuesBucketInsert.Put(RINGTONE_COLUMN_DATA,
        static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(1) + MTP_FORMAT_MP3));
    valuesBucketInsert.Put(RINGTONE_COLUMN_MEDIA_TYPE, static_cast<int>(RINGTONE_MEDIA_TYPE_AUDIO));
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

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with GetCount
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test GetCount for RingtoneAsset
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_GetCount_test_001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_GetCount_test_001 start.");
    std::shared_ptr<DataShare::DataShareResultSet> dataShare = make_shared<DataShare::DataShareResultSet>();
    std::shared_ptr<RingtoneFetchResult<RingtoneAsset>> fetchResult =
        make_shared<RingtoneFetchResult<RingtoneAsset>>(dataShare);
    ASSERT_NE(fetchResult, nullptr);
    fetchResult->resultset_ = nullptr;
    int count = fetchResult->GetCount();
    EXPECT_EQ(count, 0);

    fetchResult->Close();
    EXPECT_EQ(fetchResult->resultset_, nullptr);
    fetchResult->Close();
    RINGTONE_INFO_LOG("fetchResult_GetCount_test_001 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with GetCount
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test GetCount for VibrateAsset
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_GetCount_test_002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_GetCount_test_002 start.");
    std::shared_ptr<DataShare::DataShareResultSet> dataShare = make_shared<DataShare::DataShareResultSet>();
    std::shared_ptr<RingtoneFetchResult<VibrateAsset>> fetchResult =
        make_shared<RingtoneFetchResult<VibrateAsset>>(dataShare);
    ASSERT_NE(fetchResult, nullptr);
    fetchResult->resultset_ = nullptr;
    int count = fetchResult->GetCount();
    EXPECT_EQ(count, 0);

    fetchResult->Close();
    EXPECT_EQ(fetchResult->resultset_, nullptr);
    fetchResult->Close();
    RINGTONE_INFO_LOG("fetchResult_GetCount_test_002 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with GetObjectAtPosition
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test GetObjectAtPosition for RingtoneAsset
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_GetObjectAtPosition_test_002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_GetObjectAtPosition_test_002 start.");
    Uri uri(RINGTONE_PATH_URI);
    auto dataManager = RingtoneDataManager::GetInstance();
    ASSERT_NE(dataManager, nullptr);
    for (int index = 0; index < NUMBER_OF_TIMES; index++) {
        AbilityRuntime::DataShareValuesBucket values;
        values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
            TEST_INSERT_RINGTONE_LIBRARY + to_string(index) + MTP_FORMAT_MP3));
        values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
        values.Put(RINGTONE_COLUMN_TONE_TYPE, static_cast<int>(TEST_RINGTONE_COLUMN_TONE_TYPE));
        values.Put(RINGTONE_COLUMN_MIME_TYPE, MP3);
        values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_MP3);
        values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
        values.Put(RINGTONE_COLUMN_MEDIA_TYPE, static_cast<int>(RINGTONE_MEDIA_TYPE_AUDIO));
        RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
        auto retVal = dataManager->Insert(cmd, values);
        EXPECT_EQ((retVal > 0), true);
    }

    RingtoneDataCommand cmdQuery(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_MIME_TYPE, MP3);
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, { RINGTONE_COLUMN_SIZE } };
    int32_t errCode;
    auto queryResultSet = dataManager->Query(cmdQuery, columns, queryPredicates, errCode);
    ASSERT_NE(queryResultSet, nullptr);
    shared_ptr<AbilityRuntime::DataShareResultSet> resultSet =
        make_shared<AbilityRuntime::DataShareResultSet>(queryResultSet);
    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    ASSERT_NE(results, nullptr);

    for (int index = 0; index < NUMBER_OF_TIMES; index++) {
        auto ringtoneObject = results->GetObjectAtPosition(index);
        ASSERT_NE(ringtoneObject, nullptr);
    }
    results->Close();
    RINGTONE_INFO_LOG("fetchResult_GetObjectAtPosition_test_002 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with GetObjectAtPosition
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test GetObjectAtPosition for RingtoneAsset when resultSet row is empty
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_GetObjectAtPosition_test_003, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_GetObjectAtPosition_test_003 start.");
    Uri uri(RINGTONE_PATH_URI);
    auto dataManager = RingtoneDataManager::GetInstance();
    ASSERT_NE(dataManager, nullptr);

    RingtoneDataCommand cmdQuery(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_MIME_TYPE, "mp4");
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, { RINGTONE_COLUMN_SIZE } };
    int32_t errCode;
    auto queryResultSet = dataManager->Query(cmdQuery, columns, queryPredicates, errCode);
    ASSERT_NE(queryResultSet, nullptr);
    shared_ptr<AbilityRuntime::DataShareResultSet> resultSet =
        make_shared<AbilityRuntime::DataShareResultSet>(queryResultSet);
    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    ASSERT_NE(results, nullptr);

    for (int index = 0; index < NUMBER_OF_TIMES; index++) {
        auto ringtoneObject = results->GetObjectAtPosition(index);
        EXPECT_EQ(ringtoneObject, nullptr);
    }
    results->Close();
    RINGTONE_INFO_LOG("fetchResult_GetObjectAtPosition_test_003 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with GetObjectAtPosition
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test GetObjectAtPosition for abnormal branches
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_GetObjectAtPosition_test_004, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_GetObjectAtPosition_test_004 start.");
    std::shared_ptr<DataShare::DataShareResultSet> dataShare = make_shared<DataShare::DataShareResultSet>();
    std::shared_ptr<RingtoneFetchResult<VibrateAsset>> fetchResult =
        make_shared<RingtoneFetchResult<VibrateAsset>>(dataShare);
    fetchResult->resultset_ = nullptr;
    std::unique_ptr<VibrateAsset> resultObject = fetchResult->GetObjectAtPosition(0);
    EXPECT_EQ(resultObject, nullptr);

    std::shared_ptr<RingtoneFetchResult<RingtoneAsset>> result =
        make_shared<RingtoneFetchResult<RingtoneAsset>>(dataShare);
    ASSERT_NE(result, nullptr);
    result->resultset_ = nullptr;
    std::unique_ptr<RingtoneAsset> object = result->GetObjectAtPosition(0);
    EXPECT_EQ(object, nullptr);

    RINGTONE_INFO_LOG("fetchResult_GetObjectAtPosition_test_004 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with GetObjectAtPosition
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test GetObjectAtPosition for VibrateAsset
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_GetObjectAtPosition_test_005, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_GetObjectAtPosition_test_005 start.");
    Uri uri(VIBRATE_PATH_URI);
    auto dataManager = RingtoneDataManager::GetInstance();
    ASSERT_NE(dataManager, nullptr);
    g_uniStore->ExecuteSql("INSERT INTO " + VIBRATE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/ringtones/Carme.ogg'," +
        " 26177, 'RingtoneTest.ogg', 'rainning', 'Chinese', 1, 1, 1505707241000, 1505707241846, 1505707241," +
        " 1, 0)");
    g_uniStore->ExecuteSql("INSERT INTO " + VIBRATE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/ringtones/Carme.ogg2'," +
        " 26177, 'RingtoneTest2.ogg', 'rainning', 'Chinese', 1, 1, 1505707241000, 1505707241846, 1505707241," +
        " 1, 0)");
    g_uniStore->ExecuteSql("INSERT INTO " + VIBRATE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/ringtones/Carme.ogg3'," +
        " 26177, 'RingtoneTest3.ogg', 'rainning', 'Chinese', 1, 1, 1505707241000, 1505707241846, 1505707241," +
        " 1, 0)");
    RingtoneDataCommand cmdQuery(uri, VIBRATE_TABLE, RingtoneOperationType::QUERY);
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(VIBRATE_COLUMN_TITLE, static_cast<string>(RAINNING));
    vector<string> columns = { { VIBRATE_COLUMN_VIBRATE_ID }, { VIBRATE_COLUMN_DATA }, { VIBRATE_COLUMN_SIZE } };
    int32_t errCode;
    auto queryResultSet = dataManager->Query(cmdQuery, columns, queryPredicates, errCode);
    ASSERT_NE(queryResultSet, nullptr);
    shared_ptr<AbilityRuntime::DataShareResultSet> resultSet =
        make_shared<AbilityRuntime::DataShareResultSet>(queryResultSet);
    auto results = make_unique<RingtoneFetchResult<VibrateAsset>>(move(resultSet));
    ASSERT_NE(results, nullptr);

    auto ringtoneObject = results->GetObjectAtPosition(0);
    ASSERT_NE(ringtoneObject, nullptr);
    results->Close();
    RINGTONE_INFO_LOG("fetchResult_GetObjectAtPosition_test_005 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with GetFirstObject GetNextObject GetLastObject
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test GetFirstObject GetNextObject GetLastObject for RingtoneAsset when resultSet row is empty
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_GetFirstObject_test_001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_GetFirstObject_test_001 start.");
    Uri uri(RINGTONE_PATH_URI);
    auto dataManager = RingtoneDataManager::GetInstance();
    ASSERT_NE(dataManager, nullptr);

    RingtoneDataCommand cmdQuery(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_MIME_TYPE, "mp4");
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, { RINGTONE_COLUMN_SIZE } };
    int32_t errCode;
    auto queryResultSet = dataManager->Query(cmdQuery, columns, queryPredicates, errCode);
    ASSERT_NE(queryResultSet, nullptr);
    shared_ptr<AbilityRuntime::DataShareResultSet> resultSet =
        make_shared<AbilityRuntime::DataShareResultSet>(queryResultSet);
    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    ASSERT_NE(results, nullptr);
    auto ringtoneObject = results->GetFirstObject();
    EXPECT_EQ(ringtoneObject, nullptr);

    ringtoneObject = results->GetNextObject();
    EXPECT_EQ(ringtoneObject, nullptr);

    ringtoneObject = results->GetLastObject();
    EXPECT_EQ(ringtoneObject, nullptr);
    results->Close();
    RINGTONE_INFO_LOG("fetchResult_GetFirstObject_test_001 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with GetFirstObject GetNextObject GetLastObject
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test GetFirstObject GetNextObject GetLastObject for VibrateAsset when resultSet row is empty
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_GetFirstObject_test_002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_GetFirstObject_test_002 start.");
    Uri uri(VIBRATE_PATH_URI);
    auto dataManager = RingtoneDataManager::GetInstance();
    ASSERT_NE(dataManager, nullptr);

    RingtoneDataCommand cmdQuery(uri, VIBRATE_TABLE, RingtoneOperationType::QUERY);
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(VIBRATE_COLUMN_TITLE, "test");
    vector<string> columns = { { VIBRATE_COLUMN_VIBRATE_ID }, { VIBRATE_COLUMN_DATA }, { VIBRATE_COLUMN_SIZE } };
    int32_t errCode;
    auto queryResultSet = dataManager->Query(cmdQuery, columns, queryPredicates, errCode);
    ASSERT_NE(queryResultSet, nullptr);
    shared_ptr<AbilityRuntime::DataShareResultSet> resultSet =
        make_shared<AbilityRuntime::DataShareResultSet>(queryResultSet);
    auto results = make_unique<RingtoneFetchResult<VibrateAsset>>(move(resultSet));
    ASSERT_NE(results, nullptr);
    auto ringtoneObject = results->GetFirstObject();
    EXPECT_EQ(ringtoneObject, nullptr);

    ringtoneObject = results->GetNextObject();
    EXPECT_EQ(ringtoneObject, nullptr);

    ringtoneObject = results->GetLastObject();
    EXPECT_EQ(ringtoneObject, nullptr);
    results->Close();
    RINGTONE_INFO_LOG("fetchResult_GetFirstObject_test_002 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with IsAtLastRow
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test IsAtLastRow for RingtoneAsset when resultSet row is not empty
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_IsAtLastRow_test_002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_IsAtLastRow_test_002 start.");
    Uri uri(RINGTONE_PATH_URI);
    auto dataManager = RingtoneDataManager::GetInstance();
    ASSERT_NE(dataManager, nullptr);

    RingtoneDataCommand cmdQuery(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_MIME_TYPE, MP3);
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, { RINGTONE_COLUMN_SIZE } };
    int32_t errCode;
    auto queryResultSet = dataManager->Query(cmdQuery, columns, queryPredicates, errCode);
    ASSERT_NE(queryResultSet, nullptr);
    shared_ptr<AbilityRuntime::DataShareResultSet> resultSet =
        make_shared<AbilityRuntime::DataShareResultSet>(queryResultSet);
    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    ASSERT_NE(results, nullptr);
    auto ringtoneObject = results->GetLastObject();
    EXPECT_NE(ringtoneObject, nullptr);

    bool ret = results->IsAtLastRow();
    EXPECT_TRUE(ret);
    results->Close();
    RINGTONE_INFO_LOG("fetchResult_IsAtLastRow_test_002 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with IsAtLastRow
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test IsAtLastRow for VibrateAsset when resultSet row is not empty
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_IsAtLastRow_test_003, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_IsAtLastRow_test_003 start.");
    Uri uri(VIBRATE_PATH_URI);
    auto dataManager = RingtoneDataManager::GetInstance();
    ASSERT_NE(dataManager, nullptr);

    RingtoneDataCommand cmdQuery(uri, VIBRATE_TABLE, RingtoneOperationType::QUERY);
    DataShare::DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(VIBRATE_COLUMN_TITLE, static_cast<string>(RAINNING));
    vector<string> columns = { { VIBRATE_COLUMN_VIBRATE_ID }, { VIBRATE_COLUMN_DATA }, { VIBRATE_COLUMN_SIZE } };
    int32_t errCode;
    auto queryResultSet = dataManager->Query(cmdQuery, columns, queryPredicates, errCode);
    ASSERT_NE(queryResultSet, nullptr);
    shared_ptr<AbilityRuntime::DataShareResultSet> resultSet =
        make_shared<AbilityRuntime::DataShareResultSet>(queryResultSet);
    auto results = make_unique<RingtoneFetchResult<VibrateAsset>>(move(resultSet));
    ASSERT_NE(results, nullptr);
    auto ringtoneObject = results->GetLastObject();
    EXPECT_NE(ringtoneObject, nullptr);

    bool ret = results->IsAtLastRow();
    EXPECT_TRUE(ret);
    results->Close();
    RINGTONE_INFO_LOG("fetchResult_IsAtLastRow_test_003 end.");
}

/*
 * Feature: Service
 * Function: Test RingtoneFetchResult with GetRowValFromColumn
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test GetRowValFromColumn when resultSet is not empty
 */
HWTEST_F(RingtoneFetchResultTest, fetchResult_GetRowValFromColumn_test_003, TestSize.Level0)
{
    RINGTONE_INFO_LOG("fetchResult_GetRowValFromColumn_test_003 start.");
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
    EXPECT_EQ(ret, E_OK);

    std::shared_ptr<RingtoneFetchResult<RingtoneAsset>> fetchResult = make_unique<RingtoneFetchResult<RingtoneAsset>>();
    ASSERT_NE(fetchResult, nullptr);
    RingtoneResultSetDataType dataType = RingtoneResultSetDataType::DATA_TYPE_DOUBLE;
    std::string columnName = RINGTONE_COLUMN_DATA;
    auto res = fetchResult->GetRowValFromColumn(columnName, dataType, resultSet);
    variant<int32_t, int64_t, string, double> doubleVal = 0;
    EXPECT_EQ(res, doubleVal);

    dataType = RingtoneResultSetDataType::DATA_TYPE_INT64;
    res = fetchResult->GetRowValFromColumn(columnName, dataType, resultSet);
    variant<int32_t, int64_t, string, double> longVal = static_cast<int64_t>(0);
    EXPECT_EQ(res, longVal);

    std::unique_ptr<RingtoneAsset> asset = std::make_unique<RingtoneAsset>();
    fetchResult->SetRingtoneAsset(asset, resultSet);
    RINGTONE_INFO_LOG("fetchResult_GetRowValFromColumn_test_003 end.");
}

} // namespace Media
} // namespace OHOS