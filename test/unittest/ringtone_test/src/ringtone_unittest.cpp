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

#include "ringtone_unittest.h"

#include "datashare_helper.h"
#include "iservice_registry.h"
#include "get_self_permissions.h"
#include "ringtone_asset.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_fetch_result.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_scanner_utils.h"
#include "ringtone_tracer.h"
#include "ringtone_type.h"
#include "accesstoken_kit.h"
#include "hap_token_info.h"
#include "token_setproc.h"
#include "ringtone_proxy_uri.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;
using namespace OHOS::DataShare;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace Media {
constexpr int STORAGE_MANAGER_ID = 5003;
std::shared_ptr<DataShare::DataShareHelper> g_dataShareHelper = nullptr;
std::shared_ptr<DataShare::DataShareHelper> g_dataShareHelperProxy = nullptr;
const int S2MS = 1000;
const int MS2NS = 1000000;
const string SELECTION = RINGTONE_COLUMN_TONE_ID + " <> ? LIMIT 1, 3 ";
const int NUMBER_OF_TIMES = 10;
const string RINGTONE_LIBRARY_PATH = "/data/storage/el2/base/files";
const int TEST_RINGTONE_COLUMN_SIZE = 1022;
const string MTP_FORMAT_OGG = ".ogg"; // OGG audio files
const string TEST_INSERT_RINGTONE_LIBRARY = "test_insert_ringtone_library";
const string RAINNING = "rainning";
const string TITLE_UPDATE = "run";
const string ZERO = "0";
const string SLASH_STR = "/";
const string MTP_FORMAT_JSON = ".json"; // OGG audio files

std::vector<PermissionStateFull> GetPermissionStateFulls()
{
    std::vector<PermissionStateFull> permissionStateFulls = {
        {
            .permissionName = "ohos.permission.WRITE_RINGTONE",
            .isGeneral = true,
            .resDeviceID = { "local" },
            .grantStatus = { PermissionState::PERMISSION_GRANTED },
            .grantFlags = { 1 }
        },
        {
            .permissionName = "ohos.permission.READ_AUDIO",
            .isGeneral = true,
            .resDeviceID = { "local" },
            .grantStatus = { PermissionState::PERMISSION_GRANTED },
            .grantFlags = { 1 }
        }
    };
    return permissionStateFulls;
}

void RingtoneUnitTest::SetUpTestCase()
{
    vector<string> perms;
    perms.push_back("ohos.permission.WRITE_RINGTONE");

    uint64_t tokenId = 0;
    RingtonePermissionUtilsUnitTest::SetAccessTokenPermission("RingtoneUnitTest", perms, tokenId);
    ASSERT_TRUE(tokenId != 0);

    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(saManager != nullptr);

    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_ID);
    ASSERT_TRUE(remoteObj != nullptr);

    g_dataShareHelper = DataShare::DataShareHelper::Creator(remoteObj, RINGTONE_URI);
    ASSERT_TRUE(g_dataShareHelper != nullptr);

    HapInfoParams info = {
        .userID = 100,
        .bundleName = "ohos.datashareclienttest.demo",
        .instIndex = 0,
        .appIDDesc = "ohos.datashareclienttest.demo"
    };

    auto permStateList = GetPermissionStateFulls();
    HapPolicyParams policy = {
        .apl = APL_NORMAL,
        .domain = "test.domain",
        .permList = {
            {
                .permissionName = "ohos.permission.test",
                .bundleName = "ohos.datashareclienttest.demo",
                .grantMode = 1,
                .availableLevel = APL_NORMAL,
                .label = "label",
                .labelId = 1,
                .description = "ohos.datashareclienttest.demo",
                .descriptionId = 1
            }
        },
        .permStateList = permStateList
    };
    AccessTokenKit::AllocHapToken(info, policy);
    auto testTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenIDEx(
        info.userID, info.bundleName, info.instIndex);
    SetSelfTokenID(testTokenId.tokenIDEx);

    CreateOptions options;
    options.enabled_ = true;
    g_dataShareHelperProxy = DataShare::DataShareHelper::Creator(RINGTONE_LIBRARY_PROXY_URI, options);
}

void RingtoneUnitTest::TearDownTestCase()
{
    RINGTONE_ERR_LOG("TearDownTestCase start");
    if (g_dataShareHelper != nullptr) {
        g_dataShareHelper->Release();
    }
    system("rm -rf /data/storage/el2/base/files/");
    RINGTONE_INFO_LOG("TearDownTestCase end");
}

void RingtoneUnitTest::SetUp(void) {}

void RingtoneUnitTest::TearDown(void)
{
    system("rm -rf /data/storage/el2/base/files/");
}

int64_t UTCTimeSeconds()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &t);
    return (int64_t) ((t.tv_sec * S2MS) + (t.tv_nsec / MS2NS));
}

HWTEST_F(RingtoneUnitTest, medialib_datashareQuery_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);
    vector<string> columns {
        RINGTONE_COLUMN_TONE_ID,
        RINGTONE_COLUMN_DATA,
        RINGTONE_COLUMN_SIZE,
        RINGTONE_COLUMN_DISPLAY_NAME,
        RINGTONE_COLUMN_TITLE,
        RINGTONE_COLUMN_MEDIA_TYPE,
        RINGTONE_COLUMN_TONE_TYPE,
        RINGTONE_COLUMN_MIME_TYPE,
    };

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareQuery10Column");
    for (int i = 0; i < NUMBER_OF_TIMES; i++) {
        auto result = g_dataShareHelper->Query(uri, predicates, columns);
        EXPECT_NE(result, nullptr);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareQueryColumn Cost: " << ((double) (end - start) / NUMBER_OF_TIMES) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareQuery_test_002, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    vector<string> columns;

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareQuery10Column");
    for (int i = 0; i < NUMBER_OF_TIMES; i++) {
        auto result = g_dataShareHelper->Query(uri, predicates, columns);
        EXPECT_NE(result, nullptr);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareQuery Cost: " << ((double) (end - start) / NUMBER_OF_TIMES) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareQuery_test_003, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);
    vector<string> columns;

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareQuery10Column");
    for (int i = 0; i < NUMBER_OF_TIMES; i++) {
        auto result = g_dataShareHelper->Query(uri, predicates, columns);
        EXPECT_NE(result, nullptr);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareQuery1-3Column Cost: " << ((double) (end - start) / NUMBER_OF_TIMES) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareQuery_test_004, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    vector<string> columns {
        RINGTONE_COLUMN_TONE_ID,
        RINGTONE_COLUMN_DATA,
        RINGTONE_COLUMN_SIZE,
        RINGTONE_COLUMN_DISPLAY_NAME,
        RINGTONE_COLUMN_TITLE,
        RINGTONE_COLUMN_MEDIA_TYPE,
        RINGTONE_COLUMN_TONE_TYPE,
        RINGTONE_COLUMN_MIME_TYPE,
    };

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareQuery10Column");
    for (int i = 0; i < NUMBER_OF_TIMES; i++) {
        auto result = g_dataShareHelper->Query(uri, predicates, columns);
        EXPECT_NE(result, nullptr);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareQuery Cost: " << ((double) (end - start) / NUMBER_OF_TIMES) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareQuery_test_005, TestSize.Level0)
{
    Uri uri(RINGTONE_URI);
    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);
    vector<string> columns {
        RINGTONE_COLUMN_TONE_ID,
        RINGTONE_COLUMN_DATA,
        RINGTONE_COLUMN_SIZE,
        RINGTONE_COLUMN_DISPLAY_NAME,
        RINGTONE_COLUMN_TITLE,
        RINGTONE_COLUMN_MEDIA_TYPE,
        RINGTONE_COLUMN_TONE_TYPE,
        RINGTONE_COLUMN_MIME_TYPE,
    };

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareQuery10Column");
    for (int i = 0; i < NUMBER_OF_TIMES; i++) {
        auto result = g_dataShareHelper->Query(uri, predicates, columns);
        EXPECT_EQ(result, nullptr);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareQueryColumn Cost: " << ((double) (end - start) / NUMBER_OF_TIMES) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareInsert_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareInsert10Column");
    for (int index = 0; index < NUMBER_OF_TIMES; index++) {
        DataShareValuesBucket values;
        values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
            TEST_INSERT_RINGTONE_LIBRARY + to_string(index) + MTP_FORMAT_OGG));
        values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
        values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
        values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
        auto result = g_dataShareHelper->Insert(uri, values);
        EXPECT_EQ((result > 0), true);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    DataSharePredicates predicates;
    auto ret = g_dataShareHelper->Delete(uri, predicates);
    EXPECT_EQ((ret > 0), true);
    GTEST_LOG_(INFO) << "DataShareInsertColumn Cost: " << ((double) (end - start) / NUMBER_OF_TIMES) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareInsert_test_002, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareInsertColumn");
    DataShareValuesBucket values;
    auto result = g_dataShareHelper->Insert(uri, values);
    EXPECT_EQ(result, E_INVALID_VALUES);
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareInsertColumn Cost: " << ((double) (end - start)) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareInsert_test_003, TestSize.Level0)
{
    Uri uri(RINGTONE_DEFAULT_STR);

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareInsertColumn");
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_TONE_ID, TONE_ID_DEFAULT);
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    auto result = g_dataShareHelper->Insert(uri, values);
    EXPECT_EQ(result, E_INVALID_URI);
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareInsertColumn Cost: " << ((double) (end - start)) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareInsert_test_004, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    const string errorRingtoneLibraryPath =
        "/storage/media/files/data/com.ohos.ringtonelibrary.ringtonelibrarydata";

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareInsertColumn");
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(errorRingtoneLibraryPath + RINGTONE_SLASH_CHAR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    auto result = g_dataShareHelper->Insert(uri, values);
    EXPECT_EQ(result, E_ERR);
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareInsertColumn Cost: " << ((double) (end - start)) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareInsert_test_005, TestSize.Level0)
{
    Uri uri(RINGTONE_URI);

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareInsertColumn");
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    auto result = g_dataShareHelper->Insert(uri, values);
    EXPECT_EQ(result, E_INVALID_URI);
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareInsertColumn Cost: " << ((double) (end - start)) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareUpdate_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    auto result = g_dataShareHelper->Insert(uri, values);
    EXPECT_EQ((result > 0), true);

    DataSharePredicates predicates;
    DataShareValuesBucket updateValues;
    updateValues.Put(RINGTONE_COLUMN_TITLE, TITLE_UPDATE);

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareUpdate10Column");
    for (int i = 0; i < NUMBER_OF_TIMES; i++) {
        auto result = g_dataShareHelper->Update(uri, predicates, updateValues);
        EXPECT_EQ((result > 0), true);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    result = g_dataShareHelper->Delete(uri, predicates);
    EXPECT_EQ((result > 0), true);
    GTEST_LOG_(INFO) << "DataShareUpdate1-3Column Cost: " << ((double) (end - start) / NUMBER_OF_TIMES) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareUpdate_test_002, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);
    DataShareValuesBucket updateValues;
    updateValues.Put(RINGTONE_COLUMN_TITLE, TITLE_UPDATE);

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareUpdateColumn");
    for (int i = 0; i < 1; i++) {
        auto result = g_dataShareHelper->Update(uri, predicates, updateValues);
        EXPECT_EQ(result, E_HAS_DB_ERROR);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareUpdateColumn Cost: " << ((double) (end - start)) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareUpdate_test_003, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    DataShareValuesBucket updateValues;

    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareUpdate10Column");
    for (int i = 0; i < 1; i++) {
        auto result = g_dataShareHelper->Update(uri, predicates, updateValues);
        EXPECT_EQ(result, E_INVALID_VALUES);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareUpdate1-3Column Cost: " << ((double) (end - start)) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareUpdate_test_004, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_TONE_ID, TONE_ID_DEFAULT);
    values.Put(RINGTONE_COLUMN_DATA, DATA_DEFAULT);
    values.Put(RINGTONE_COLUMN_SIZE, SIZE_DEFAULT);
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, DISPLAY_NAME_DEFAULT);
    values.Put(RINGTONE_COLUMN_TITLE, TITLE_DEFAULT);
    values.Put(RINGTONE_COLUMN_MEDIA_TYPE, METADATA_MEDIA_TYPE_DEFAULT);
    values.Put(RINGTONE_COLUMN_TONE_TYPE, TONE_TYPE_DEFAULT);
    values.Put(RINGTONE_COLUMN_MIME_TYPE, MIME_TYPE_DEFAULT);


    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareUpdate10Column");
    for (int i = 0; i < 1; i++) {
        auto result = g_dataShareHelper->Update(uri, predicates, values);
        EXPECT_EQ(result, E_HAS_DB_ERROR);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareUpdate1-3Column Cost: " << ((double) (end - start)) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareUpdate_test_005, TestSize.Level0)
{
    Uri uri(RINGTONE_URI);
    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_TONE_ID, TONE_ID_DEFAULT);
    values.Put(RINGTONE_COLUMN_DATA, DATA_DEFAULT);
    values.Put(RINGTONE_COLUMN_SIZE, SIZE_DEFAULT);
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, DISPLAY_NAME_DEFAULT);
    values.Put(RINGTONE_COLUMN_TITLE, TITLE_DEFAULT);
    values.Put(RINGTONE_COLUMN_MEDIA_TYPE, METADATA_MEDIA_TYPE_DEFAULT);
    values.Put(RINGTONE_COLUMN_TONE_TYPE, TONE_TYPE_DEFAULT);
    values.Put(RINGTONE_COLUMN_MIME_TYPE, MIME_TYPE_DEFAULT);


    int64_t start = UTCTimeSeconds();
    RingtoneTracer tracer;
    tracer.Start("DataShareUpdate10Column");
    for (int i = 0; i < 1; i++) {
        auto result = g_dataShareHelper->Update(uri, predicates, values);
        EXPECT_EQ(result, E_INVALID_URI);
    }
    tracer.Finish();
    int64_t end = UTCTimeSeconds();

    GTEST_LOG_(INFO) << "DataShareUpdate1-3Column Cost: " << ((double) (end - start)) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_datashareDelete_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    auto result = g_dataShareHelper->Insert(uri, values);
    EXPECT_EQ((result > 0), true);

    DataSharePredicates predicates;

    result = g_dataShareHelper->Delete(uri, predicates);
    EXPECT_EQ((result > 0), true);
}

HWTEST_F(RingtoneUnitTest, medialib_datashareDelete_test_002, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);

    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);

    auto result = g_dataShareHelper->Delete(uri, predicates);
    EXPECT_EQ(result, E_HAS_DB_ERROR);
}

HWTEST_F(RingtoneUnitTest, medialib_datashareDelete_test_003, TestSize.Level0)
{
    Uri uri(RINGTONE_DEFAULT_STR);

    DataSharePredicates predicates;

    auto result = g_dataShareHelper->Delete(uri, predicates);
    EXPECT_EQ(result, E_INVALID_URI);
}

HWTEST_F(RingtoneUnitTest, medialib_datashareDelete_test_004, TestSize.Level0)
{
    Uri uri(RINGTONE_DEFAULT_STR);

    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);

    auto result = g_dataShareHelper->Delete(uri, predicates);
    EXPECT_EQ(result, E_INVALID_URI);
}

HWTEST_F(RingtoneUnitTest, medialib_datashareDelete_test_005, TestSize.Level0)
{
    Uri uri(RINGTONE_URI);

    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);

    auto result = g_dataShareHelper->Delete(uri, predicates);
    EXPECT_EQ(result, E_INVALID_URI);
}

HWTEST_F(RingtoneUnitTest, medialib_datashareOpenFile_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    auto result = g_dataShareHelper->Insert(uri, values);
    EXPECT_EQ((result > 0), true);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA } };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    cout << "query count = " << to_string(results->GetCount()) << endl;
    unique_ptr<RingtoneAsset> ringtoneAsset = results->GetFirstObject();
    if (ringtoneAsset != nullptr && ringtoneAsset->GetPath() != RINGTONE_DEFAULT_STR) {
        string uriStr = RINGTONE_PATH_URI + RINGTONE_SLASH_CHAR + to_string(ringtoneAsset->GetId());
        Uri ofUri(uriStr);
        string mode = RINGTONE_FILEMODE_READWRITE;
        auto fd = g_dataShareHelper->OpenFile(ofUri, mode);
        EXPECT_GE(fd, 0);
        cout << "OpenFile fd = " << fd << endl;
        if (fd >= 0) {
            close(fd);
        }
    }
    DataSharePredicates predicates;
    result = g_dataShareHelper->Delete(uri, predicates);
    EXPECT_EQ((result > 0), true);
}

HWTEST_F(RingtoneUnitTest, medialib_datashareOpenFile_test_002, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    string mode;

    auto fd = g_dataShareHelper->OpenFile(uri, mode);
    EXPECT_EQ(fd, E_ERR);
    if (fd >= 0) {
        close(fd);
    }
}

HWTEST_F(RingtoneUnitTest, medialib_datashareOpenFile_test_003, TestSize.Level0)
{
    const string uriStr = RINGTONE_PATH_URI + RINGTONE_SLASH_CHAR + "1";
    Uri uri(uriStr);
    string mode = RINGTONE_FILEMODE_READWRITE;

    auto fd = g_dataShareHelper->OpenFile(uri, mode);
    EXPECT_EQ(fd, E_ERR);
    if (fd >= 0) {
        close(fd);
    }
}

HWTEST_F(RingtoneUnitTest, medialib_datashareOpenFile_test_004, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    string mode = RINGTONE_FILEMODE_READWRITE;

    auto fd = g_dataShareHelper->OpenFile(uri, mode);
    EXPECT_EQ(fd, E_ERR);
    if (fd >= 0) {
        close(fd);
    }
}

HWTEST_F(RingtoneUnitTest, medialib_datashareOpenFile_test_005, TestSize.Level0)
{
    Uri uri(RINGTONE_URI);
    string mode = RINGTONE_FILEMODE_READWRITE;

    auto fd = g_dataShareHelper->OpenFile(uri, mode);
    EXPECT_EQ(fd, E_ERR);
    if (fd >= 0) {
        close(fd);
    }
}


HWTEST_F(RingtoneUnitTest, medialib_ringtoneRead_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    g_dataShareHelper->Delete(uri, predicates);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    values.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(2));
    auto result = g_dataShareHelper->Insert(uri, values);
    cout << "Insert result=" << result << endl;
    EXPECT_EQ((result > 0), true);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        cout << "query count = " << to_string(results->GetCount()) << endl;
        unique_ptr<RingtoneAsset> ringtoneAsset = results->GetLastObject();
        if (ringtoneAsset == nullptr) {
            exit(1);
        }
        cout << "GetSourceType() = " << to_string(ringtoneAsset->GetSourceType()) << endl;
        result = g_dataShareHelper->Delete(uri, predicates);
        EXPECT_EQ((result > 0), true);
    }
}

HWTEST_F(RingtoneUnitTest, medialib_ringtoneRead_test_002, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    g_dataShareHelper->Delete(uri, predicates);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    values.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(1));
    auto result = g_dataShareHelper->Insert(uri, values);
    cout << "Insert result=" << result << endl;
    EXPECT_EQ((result > 0), true);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        cout << "query count = " << to_string(results->GetCount()) << endl;
        unique_ptr<RingtoneAsset> ringtoneAsset = results->GetLastObject();
        if (ringtoneAsset == nullptr) {
            exit(1);
        }
        cout << "GetSourceType() = " << to_string(ringtoneAsset->GetSourceType()) << endl;
        result = g_dataShareHelper->Delete(uri, predicates);
        EXPECT_EQ((result > 0), true);
    }
}

HWTEST_F(RingtoneUnitTest, medialib_ringtoneRead_test_003, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    g_dataShareHelper->Delete(uri, predicates);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    values.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(2));
    auto result = g_dataShareHelper->Insert(uri, values);
    cout << "Insert result=" << result << endl;
    EXPECT_EQ((result > 0), true);

    DataShareValuesBucket valuesInsert;
    valuesInsert.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(1) + MTP_FORMAT_OGG));
    valuesInsert.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    valuesInsert.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    valuesInsert.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    valuesInsert.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(1));
    result = g_dataShareHelper->Insert(uri, valuesInsert);
    cout << "Insert2 result=" << result << endl;
    EXPECT_EQ((result > 0), true);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, to_string(2));
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        cout << "query count = " << to_string(results->GetCount()) << endl;
        unique_ptr<RingtoneAsset> ringtoneAsset = results->GetLastObject();
        if (ringtoneAsset == nullptr) {
            exit(1);
        }
        cout << "GetSourceType() = " << to_string(ringtoneAsset->GetSourceType()) << endl;
        result = g_dataShareHelper->Delete(uri, predicates);
        EXPECT_EQ((result > 0), true);
    }
}

HWTEST_F(RingtoneUnitTest, medialib_ringtoneRead_test_004, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    g_dataShareHelper->Delete(uri, predicates);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    values.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(2));
    auto result = g_dataShareHelper->Insert(uri, values);
    cout << "Insert result=" << result << endl;
    EXPECT_EQ((result > 0), true);

    DataShareValuesBucket valuesInsert;
    valuesInsert.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(1) + MTP_FORMAT_OGG));
    valuesInsert.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    valuesInsert.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    valuesInsert.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    valuesInsert.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(1));
    result = g_dataShareHelper->Insert(uri, valuesInsert);
    cout << "Insert2 result=" << result << endl;
    EXPECT_EQ((result > 0), true);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, to_string(1));
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        cout << "query count = " << to_string(results->GetCount()) << endl;
        unique_ptr<RingtoneAsset> ringtoneAsset = results->GetLastObject();
        if (ringtoneAsset == nullptr) {
            exit(1);
        }
        cout << "GetSourceType() = " << to_string(ringtoneAsset->GetSourceType()) << endl;
        result = g_dataShareHelper->Delete(uri, predicates);
        EXPECT_EQ((result > 0), true);
    }
}

HWTEST_F(RingtoneUnitTest, medialib_ringtoneRead_test_005, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    g_dataShareHelper->Delete(uri, predicates);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    values.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(2));
    auto result = g_dataShareHelper->Insert(uri, values);
    cout << "Insert result=" << result << endl;
    EXPECT_EQ((result > 0), true);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, to_string(1));
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        cout << "query count = " << to_string(results->GetCount()) << endl;
        result = g_dataShareHelper->Delete(uri, predicates);
        EXPECT_EQ((result > 0), true);
    }
}

HWTEST_F(RingtoneUnitTest, medialib_ringtoneRead_test_006, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    g_dataShareHelper->Delete(uri, predicates);
    DataShareValuesBucket valuesInsert;
    valuesInsert.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + SLASH_STR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(1) + MTP_FORMAT_OGG));
    valuesInsert.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    valuesInsert.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    valuesInsert.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    valuesInsert.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(1));
    auto result = g_dataShareHelper->Insert(uri, valuesInsert);
    cout << "Insert2 result=" << result << endl;
    EXPECT_EQ((result > 0), true);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, to_string(2));
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        cout << "query count = " << to_string(results->GetCount()) << endl;
        result = g_dataShareHelper->Delete(uri, predicates);
        EXPECT_EQ((result > 0), true);
    }
}

HWTEST_F(RingtoneUnitTest, medialib_ringtoneRead_test_007, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    g_dataShareHelper->Delete(uri, predicates);;

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    vector<string> columns = { { RINGTONE_COLUMN_TONE_ID }, { RINGTONE_COLUMN_DATA }, {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        EXPECT_NE(results, nullptr);
        cout << "query count = " << to_string(results->GetCount()) << endl;
    }
}

HWTEST_F(RingtoneUnitTest, medialib_querySystemRingtone_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, to_string(SourceType::SOURCE_TYPE_PRESET));
    vector<string> columns = { {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        EXPECT_NE(results, nullptr);
        cout << "query count = " << to_string(results->GetCount()) << endl;
    }
}

HWTEST_F(RingtoneUnitTest, medialib_queryCustomisedRingtone_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, to_string(SourceType::SOURCE_TYPE_CUSTOMISED));
    vector<string> columns = { {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        EXPECT_NE(results, nullptr);
        cout << "query count = " << to_string(results->GetCount()) << endl;
    }
}

HWTEST_F(RingtoneUnitTest, medialib_queryAllRingtone_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    vector<string> columns = { {RINGTONE_COLUMN_SOURCE_TYPE} };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        EXPECT_NE(results, nullptr);
        cout << "query count = " << to_string(results->GetCount()) << endl;
    }
}

HWTEST_F(RingtoneUnitTest, medialib_deleteRingtone_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataShareValuesBucket values;
    values.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
        TEST_INSERT_RINGTONE_LIBRARY + to_string(0) + MTP_FORMAT_OGG));
    values.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(TEST_RINGTONE_COLUMN_SIZE));
    values.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>(RAINNING) + MTP_FORMAT_OGG);
    values.Put(RINGTONE_COLUMN_TITLE, static_cast<string>(RAINNING));
    values.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(SourceType::SOURCE_TYPE_CUSTOMISED));
    auto result = g_dataShareHelper->Insert(uri, values);
    EXPECT_EQ((result > 0), true);

    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, to_string(SourceType::SOURCE_TYPE_CUSTOMISED));
    vector<string> columns = { { RINGTONE_COLUMN_SOURCE_TYPE } };
    auto resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;
    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        EXPECT_NE(results, nullptr);
        cout << "query count = " << to_string(results->GetCount()) << endl;
    }

    DataSharePredicates predicates;
    predicates.SetWhereClause(RINGTONE_COLUMN_SOURCE_TYPE + " = ? ");
    predicates.SetWhereArgs({ to_string(SourceType::SOURCE_TYPE_CUSTOMISED) });
    result = g_dataShareHelper->Delete(uri, predicates);
    EXPECT_EQ((result > 0), true);

    resultSet = g_dataShareHelper->Query(uri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;
    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
        EXPECT_NE(results, nullptr);
        cout << "query count = " << to_string(results->GetCount()) << endl;
    }
}

HWTEST_F(RingtoneUnitTest, medialib_setVibrateSetting_test_001, TestSize.Level0)
{
    Uri uri(SIMCARD_SETTING_PATH_URI);

    int64_t start = UTCTimeSeconds();
    const int oldMode = 1;
    RingtoneTracer tracer;
    tracer.Start("DataShareInsert->SetVibrateSetting");

    DataShareValuesBucket values;

    values.Put(SIMCARD_SETTING_COLUMN_MODE, RING_TONE_TYPE_NOT);
    values.Put(SIMCARD_SETTING_COLUMN_TONE_FILE, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
            TEST_INSERT_RINGTONE_LIBRARY + MTP_FORMAT_OGG));
    values.Put(SIMCARD_SETTING_COLUMN_VIBRATE_FILE, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
            TEST_INSERT_RINGTONE_LIBRARY + MTP_FORMAT_JSON));
    values.Put(SIMCARD_SETTING_COLUMN_VIBRATE_MODE, oldMode);
    auto result = g_dataShareHelper->Insert(uri, values);
    GTEST_LOG_(INFO)<< "setVibrateSetting -> Insert result=" << result;
    EXPECT_EQ((result > 0), true);

    tracer.Finish();
    int64_t end = UTCTimeSeconds();
    GTEST_LOG_(INFO) << "SetVibrateSetting Cost: " << (double) (end - start) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_updateVibrateSetting_test_001, TestSize.Level0)
{
    Uri uri(SIMCARD_SETTING_PATH_URI);

    int64_t start = UTCTimeSeconds();
    const int newMode = 2;
    RingtoneTracer tracer;
    tracer.Start("DataShareInsert->SetVibrateSetting");

    DataSharePredicates predicates;
    vector<string> selectionArgs = {std::to_string(RING_TONE_TYPE_NOT)};
    const std::string selection = SIMCARD_SETTING_COLUMN_MODE + " = ? ";
    predicates.SetWhereClause(selection);
    predicates.SetWhereArgs(selectionArgs);

    DataShareValuesBucket values;
    values.Put(SIMCARD_SETTING_COLUMN_MODE, RING_TONE_TYPE_NOT);
    values.Put(SIMCARD_SETTING_COLUMN_TONE_FILE, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
            TEST_INSERT_RINGTONE_LIBRARY + MTP_FORMAT_OGG));
    values.Put(SIMCARD_SETTING_COLUMN_VIBRATE_FILE, static_cast<string>(RINGTONE_LIBRARY_PATH + RINGTONE_SLASH_CHAR +
            TEST_INSERT_RINGTONE_LIBRARY + MTP_FORMAT_JSON));
    values.Put(SIMCARD_SETTING_COLUMN_VIBRATE_MODE, newMode);
    auto result = g_dataShareHelper->Update(uri, predicates, values);
    GTEST_LOG_(INFO)<< "updateVibrateSetting -> update result=" << result;
    EXPECT_EQ((result > 0), true);

    tracer.Finish();
    int64_t end = UTCTimeSeconds();
    GTEST_LOG_(INFO) << "updateVibrateSetting Cost: " << (double) (end - start) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_GetVibrateSetting_test_001, TestSize.Level0)
{
    Uri uri(SIMCARD_SETTING_PATH_URI);

    int64_t start = UTCTimeSeconds();
    const int newMode = 2;
    RingtoneTracer tracer;
    tracer.Start("DataShareInsert->GetVibrateSetting");

    DataSharePredicates predicates;
    vector<string> selectionArgs = {std::to_string(RING_TONE_TYPE_NOT)};
    const std::string selection = SIMCARD_SETTING_COLUMN_MODE + " = ? ";
    predicates.SetWhereClause(selection);
    predicates.SetWhereArgs(selectionArgs);
    vector<string> columns {
        SIMCARD_SETTING_COLUMN_MODE,
        SIMCARD_SETTING_COLUMN_TONE_FILE,
        SIMCARD_SETTING_COLUMN_VIBRATE_FILE,
        SIMCARD_SETTING_COLUMN_VIBRATE_MODE
    };

    int errCode = 0;
    DatashareBusinessError businessError;
    auto resultSet = g_dataShareHelper->Query(uri, predicates, columns, &businessError);
    errCode = businessError.GetCode();
    GTEST_LOG_(INFO)<< "GetVibrateSetting -> Query errCode=" << errCode;

    EXPECT_NE(resultSet, nullptr);
    if (resultSet != nullptr) {
        auto results = make_unique<RingtoneFetchResult<SimcardSettingAsset>>(move(resultSet));
        EXPECT_NE(results, nullptr);
        auto simcardSettingAsset = results->GetFirstObject();
        EXPECT_NE(simcardSettingAsset, nullptr);
        if (simcardSettingAsset != nullptr) {
            EXPECT_EQ(simcardSettingAsset->GetVibrateMode(), newMode);
        }
    }

    tracer.Finish();
    int64_t end = UTCTimeSeconds();
    GTEST_LOG_(INFO) << "GetVibrateSetting Cost: " << (double) (end - start) << "ms";
}

HWTEST_F(RingtoneUnitTest, medialib_deleteVibrateSetting_test_001, TestSize.Level0)
{
    Uri uri(SIMCARD_SETTING_PATH_URI);

    DataSharePredicates predicates;
    vector<string> selectionArgs = {std::to_string(RING_TONE_TYPE_NOT)};
    const std::string selection = SIMCARD_SETTING_COLUMN_MODE + " = ? ";
    predicates.SetWhereClause(selection);
    predicates.SetWhereArgs(selectionArgs);
    vector<string> columns {
        SIMCARD_SETTING_COLUMN_MODE,
        SIMCARD_SETTING_COLUMN_TONE_FILE,
        SIMCARD_SETTING_COLUMN_VIBRATE_FILE,
        SIMCARD_SETTING_COLUMN_VIBRATE_MODE
    };

    auto ret = g_dataShareHelper->Delete(uri, predicates);
    GTEST_LOG_(INFO)<< "GetVibrateSetting -> Delete result=" << ret;
    EXPECT_EQ((ret > 0), true);
}

HWTEST_F(RingtoneUnitTest, medialib_silentAccessQuery_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_LIBRARY_PROXY_DATA_URI_SIMCARD_SETTING);

    DataSharePredicates predicates;
    vector<string> selectionArgs = {std::to_string(RING_TONE_TYPE_SIM_CARD_1)};
    const std::string selection = SIMCARD_SETTING_COLUMN_MODE + " = ? ";
    predicates.SetWhereClause(selection);
    predicates.SetWhereArgs(selectionArgs);

    vector<string> columns {
        SIMCARD_SETTING_COLUMN_MODE,
        SIMCARD_SETTING_COLUMN_TONE_FILE,
        SIMCARD_SETTING_COLUMN_VIBRATE_FILE,
        SIMCARD_SETTING_COLUMN_VIBRATE_MODE
    };

    DatashareBusinessError businessError;
    auto resultSet = g_dataShareHelperProxy->Query(uri, predicates, columns, &businessError);
    auto errCode = businessError.GetCode();
    GTEST_LOG_(INFO)<< "g_dataShareHelperProxy->Query(uri errCode=" << errCode;
    EXPECT_EQ(resultSet, nullptr);
}
} // namespace Media
} // namespace OHOS
