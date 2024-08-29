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
#include "ringtone_language_manager_test.h"

#include "ability_context_impl.h"
#include "rdb_helper.h"
#include "ringtone_errno.h"
#include "ringtone_type.h"
#include "ringtone_rdbstore.h"
#define private public
#include "ringtone_language_manager.h"
#undef private

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {

enum {
    DATA_INDEX = 0,
    DISPLAY_NAME_INDEX,
    TITLE_INDEX,
    SOURCE_TYPE_INDEX,
    DISPLAY_LANGUAGE_TYPE_INDEX,
};

static std::shared_ptr<RingtoneUnistore> g_ringtoneUniStore = nullptr;
const std::vector<std::vector<std::string>> g_ringtoneMetadata = {
    {"/sys_prod/resource/media/audio/alarms/candy.ogg", "candy.ogg", "糖果", "1", "zh-Hans"},
    {"/sys_prod/resource/media/audio/alarms/spring_outing.ogg", "spring_outing.ogg", "春游", "1", "zh-Hans"},
    {"/sys_prod/resource/media/audio/alarms/maze.ogg", "maze.ogg", "maze", "1", "en-Latn-US"},
    {"/sys_prod/resource/media/audio/alarms/didi.ogg", "didi.ogg", "didi", "1", "en-Latn-US"},
    {"/sys_prod/resource/media/audio/alarms/brook.ogg", "brook.ogg", "brook", "1", ""},
    {"/sys_prod/resource/media/audio/alarms/kaleidoscope.ogg", "kaleidoscope.ogg", "kaleidoscope", "2", ""},
};
const std::vector<std::vector<std::string>> g_vibrationMetadata = {
    {"/sys_prod/resource/media/haptics/standard/synchronized/ringtones/candy.json", "candy.json", "糖果", "1",
        "zh-Hans"},
    {"/sys_prod/resource/media/haptics/gentle/synchronized/ringtones/candy.json", "candy.json", "candy", "2", ""},
    {"/sys_prod/resource/media/haptics/standard/synchronized/ringtones/maze.json", "maze.json", "maze", "1",
        "en-Latn-US"},
    {"/sys_prod/resource/media/haptics/gentle/synchronized/ringtones/maze.json", "maze.json", "maze", "2", ""},
    {"/sys_prod/resource/media/haptics/standard/non-synchronized/brook.json", "brook.json", "溪流", "1", "zh-Hans"},
    {"/sys_prod/resource/media/haptics/standard/non-synchronized/didi.json", "didi.json", "didi", "1", "en-Latn-US"},
};
const std::map<std::string, std::map<std::string, std::string>> g_translationResource = {
    {
        "zh-Hans", {
            {"candy", "糖果"},
            {"spring_outing", "春游"},
            {"maze", "迷宫"},
            {"didi", "滴滴"},
            {"brook", "溪流"},
            {"kaleidoscope", "万花筒"},
        }
    },
    {
        "en-Latn-US", {
            {"candy", "candy"},
            {"spring_outing", "spring outing"},
            {"maze", "maze"},
            {"didi", "didi"},
            {"brook", "brook"},
            {"kaleidoscope", "kaleidoscope"},
        }
    }
};

const string CHINESE_ABBREVIATION = "zh-Hans";
const string ENGLISH_ABBREVIATION = "en-Latn-US";
const int32_t RINGTONE_DIFF_AMOUN = 3;
const int32_t VIBRATION_DIFF_AMOUN = 2;

void InitTestData();

void RingtoneLanguageManagerTest::SetUpTestCase() {}

void RingtoneLanguageManagerTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneLanguageManagerTest::SetUp()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    g_ringtoneUniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = g_ringtoneUniStore->Init();
    EXPECT_EQ(ret, E_OK);
    InitTestData();
}

void RingtoneLanguageManagerTest::TearDown(void) {}

void InitTestData()
{
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand ringtoneCmdDel(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    int32_t rowId = 0;
    int32_t ret = g_ringtoneUniStore->Delete(ringtoneCmdDel, rowId);
    EXPECT_EQ(ret, E_OK);

    for (auto &item : g_ringtoneMetadata) {
        RingtoneDataCommand cmdInsert(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
        NativeRdb::ValuesBucket values;
        values.Put(RINGTONE_COLUMN_DATA, item[DATA_INDEX]);
        values.Put(RINGTONE_COLUMN_DISPLAY_NAME, item[DISPLAY_NAME_INDEX]);
        values.Put(RINGTONE_COLUMN_TITLE, item[TITLE_INDEX]);
        values.Put(RINGTONE_COLUMN_SOURCE_TYPE, stoi(item[SOURCE_TYPE_INDEX]));
        if (!item[DISPLAY_LANGUAGE_TYPE_INDEX].empty()) {
            values.Put(RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE, item[DISPLAY_LANGUAGE_TYPE_INDEX]);
        }
        cmdInsert.SetValueBucket(values);
        int64_t rowId = E_HAS_DB_ERROR;
        int32_t ret = g_ringtoneUniStore->Insert(cmdInsert, rowId);
        EXPECT_EQ(ret, E_OK);
    }

    RingtoneDataCommand vibrationCmdDel(uri, VIBRATE_TABLE, RingtoneOperationType::DELETE);
    rowId = 0;
    ret = g_ringtoneUniStore->Delete(vibrationCmdDel, rowId);
    EXPECT_EQ(ret, E_OK);

    for (auto &item : g_vibrationMetadata) {
        RingtoneDataCommand cmdInsert(uri, VIBRATE_TABLE, RingtoneOperationType::INSERT);
        NativeRdb::ValuesBucket values;
        values.Put(VIBRATE_COLUMN_DATA, item[DATA_INDEX]);
        values.Put(VIBRATE_COLUMN_DISPLAY_NAME, item[DISPLAY_NAME_INDEX]);
        values.Put(VIBRATE_COLUMN_TITLE, item[TITLE_INDEX]);
        values.Put(VIBRATE_COLUMN_VIBRATE_TYPE, stoi(item[SOURCE_TYPE_INDEX]));
        if (!item[DISPLAY_LANGUAGE_TYPE_INDEX].empty()) {
            values.Put(VIBRATE_COLUMN_DISPLAY_LANGUAGE, item[DISPLAY_LANGUAGE_TYPE_INDEX]);
        }
        cmdInsert.SetValueBucket(values);
        int64_t rowId = E_HAS_DB_ERROR;
        int32_t ret = g_ringtoneUniStore->Insert(cmdInsert, rowId);
        EXPECT_EQ(ret, E_OK);
    }
}

HWTEST_F(RingtoneLanguageManagerTest, languageManager_CheckLanguageTypeByRingtone_test_001, TestSize.Level0)
{
    auto langMgr = RingtoneLanguageManager::GetInstance();
    langMgr->systemLanguage_ = CHINESE_ABBREVIATION;

    int rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int ret = langMgr->CheckLanguageTypeByRingtone(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, RINGTONE_DIFF_AMOUN);
}

HWTEST_F(RingtoneLanguageManagerTest, languageManager_CheckLanguageTypeByRingtone_test_002, TestSize.Level0)
{
    auto langMgr = RingtoneLanguageManager::GetInstance();
    langMgr->systemLanguage_ = ENGLISH_ABBREVIATION;

    int rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int ret = langMgr->CheckLanguageTypeByRingtone(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, RINGTONE_DIFF_AMOUN);
}

HWTEST_F(RingtoneLanguageManagerTest, languageManager_ChangeLanguageDataToRingtone_test_001, TestSize.Level0)
{
    auto langMgr = RingtoneLanguageManager::GetInstance();
    langMgr->systemLanguage_ = ENGLISH_ABBREVIATION;

    int rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int32_t ret = langMgr->CheckLanguageTypeByRingtone(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, RINGTONE_DIFF_AMOUN);

    langMgr->ringtoneTranslate_ = g_translationResource;
    langMgr->ChangeLanguageDataToRingtone(rowCount, resultSet);

    ret = langMgr->CheckLanguageTypeByRingtone(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, 0);
}

HWTEST_F(RingtoneLanguageManagerTest, languageManager_ChangeLanguageDataToRingtone_test_002, TestSize.Level0)
{
    auto langMgr = RingtoneLanguageManager::GetInstance();
    langMgr->systemLanguage_ = CHINESE_ABBREVIATION;

    int rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int32_t ret = langMgr->CheckLanguageTypeByRingtone(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, RINGTONE_DIFF_AMOUN);

    langMgr->ringtoneTranslate_ = g_translationResource;
    langMgr->ChangeLanguageDataToRingtone(rowCount, resultSet);

    ret = langMgr->CheckLanguageTypeByRingtone(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, 0);
}

HWTEST_F(RingtoneLanguageManagerTest, languageManager_CheckLanguageTypeByVibration_test_001, TestSize.Level0)
{
    auto langMgr = RingtoneLanguageManager::GetInstance();
    langMgr->systemLanguage_ = CHINESE_ABBREVIATION;

    int rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int ret = langMgr->CheckLanguageTypeByVibration(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, VIBRATION_DIFF_AMOUN);
}

HWTEST_F(RingtoneLanguageManagerTest, languageManager_CheckLanguageTypeByVibration_test_002, TestSize.Level0)
{
    auto langMgr = RingtoneLanguageManager::GetInstance();
    langMgr->systemLanguage_ = ENGLISH_ABBREVIATION;

    int rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int ret = langMgr->CheckLanguageTypeByVibration(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, VIBRATION_DIFF_AMOUN);
}

HWTEST_F(RingtoneLanguageManagerTest, languageManager_ChangeLanguageDataToVibration_test_001, TestSize.Level0)
{
    auto langMgr = RingtoneLanguageManager::GetInstance();
    langMgr->systemLanguage_ = CHINESE_ABBREVIATION;

    int rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int32_t ret = langMgr->CheckLanguageTypeByVibration(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, VIBRATION_DIFF_AMOUN);

    langMgr->vibrationTranslate_ = g_translationResource;
    langMgr->ChangeLanguageDataToVibration(rowCount, resultSet);

    ret = langMgr->CheckLanguageTypeByVibration(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, 0);
}

HWTEST_F(RingtoneLanguageManagerTest, languageManager_ChangeLanguageDataToVibration_test_002, TestSize.Level0)
{
    auto langMgr = RingtoneLanguageManager::GetInstance();
    langMgr->systemLanguage_ = ENGLISH_ABBREVIATION;

    int rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    int32_t ret = langMgr->CheckLanguageTypeByVibration(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, VIBRATION_DIFF_AMOUN);

    langMgr->vibrationTranslate_ = g_translationResource;
    langMgr->ChangeLanguageDataToVibration(rowCount, resultSet);

    ret = langMgr->CheckLanguageTypeByVibration(rowCount, resultSet);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(rowCount, 0);
}
} // namespace Media
} // namespace OHOS
