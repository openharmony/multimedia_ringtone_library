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

#include "ringtone_default_settings_test.h"

#include<memory>

#include "ringtone_errno.h"
#define private public
#include "ringtone_default_setting.h"
#undef private
#include "ability_context_impl.h"
#include "rdb_store.h"
#include "rdb_utils.h"
#include "ringtone_rdbstore.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
void RingtoneDefaultSettingsTest::SetUpTestCase()
{
}

void RingtoneDefaultSettingsTest::TearDownTestCase()
{
}

// SetUp:Execute before each test case
void RingtoneDefaultSettingsTest::SetUp() {}

void RingtoneDefaultSettingsTest::TearDown(void) {}

HWTEST_F(RingtoneDefaultSettingsTest, settings_ShotToneDefaultSettings_001, TestSize.Level0)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);
    auto rawRdb = uniStore->GetRaw();
    std::unique_ptr<RingtoneDefaultSetting> defaultSetting = RingtoneDefaultSetting::GetObj(rawRdb);
    system("param set persist.ringtone.setting.shot2 Betelgeuse.ogg");
    system("param set persist.ringtone.setting.shot Bellatrix.ogg");
    defaultSetting->ShotToneDefaultSettings();
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    const string data = ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    values.Put(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, static_cast<int>(1));
    cmd.SetValueBucket(values);
    uniStore->Init();
    int64_t rowId = E_HAS_DB_ERROR;
    ret = uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
    defaultSetting->ShotToneDefaultSettings();
    system("param set persist.ringtone.setting.shot2 \"\"");
    system("param set persist.ringtone.setting.shot \"\"");
    defaultSetting->ShotToneDefaultSettings();
}

HWTEST_F(RingtoneDefaultSettingsTest, settings_NotificationToneDefaultSettings_001, TestSize.Level0)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);
    auto rawRdb = uniStore->GetRaw();
    std::unique_ptr<RingtoneDefaultSetting> defaultSetting = RingtoneDefaultSetting::GetObj(rawRdb);
    system("param set persist.ringtone.setting.notification Bellatrix.ogg");
    defaultSetting->NotificationToneDefaultSettings();
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    const string data = ROOT_TONE_PRELOAD_PATH_CHINA_PATH + "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    values.Put(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, static_cast<int>(1));
    cmd.SetValueBucket(values);
    uniStore->Init();
    int64_t rowId = E_HAS_DB_ERROR;
    ret = uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
    defaultSetting->NotificationToneDefaultSettings();
    system("param set persist.ringtone.setting.notification \"\"");
    defaultSetting->NotificationToneDefaultSettings();
}

HWTEST_F(RingtoneDefaultSettingsTest, settings_RingToneDefaultSettings_001, TestSize.Level0)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);
    auto rawRdb = uniStore->GetRaw();
    std::unique_ptr<RingtoneDefaultSetting> defaultSetting = RingtoneDefaultSetting::GetObj(rawRdb);
    system("param set persist.ringtone.setting.ringtone2 test_name");
    system("param set persist.ringtone.setting.ringtone test_name");
    defaultSetting->RingToneDefaultSettings();
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    const string data = ROOT_TONE_PRELOAD_PATH_NOAH_PATH + "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    values.Put(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, static_cast<int>(1));
    cmd.SetValueBucket(values);
    uniStore->Init();
    int64_t rowId = E_HAS_DB_ERROR;
    ret = uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
    defaultSetting->RingToneDefaultSettings();
    system("param set persist.ringtone.setting.ringtone2 \"\"");
    system("param set persist.ringtone.setting.ringtone \"\"");
    defaultSetting->RingToneDefaultSettings();
}

HWTEST_F(RingtoneDefaultSettingsTest, settings_GetTonePathByDisplayName_001, TestSize.Level0)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    const string data = "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    values.Put(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, static_cast<int>(1));
    cmd.SetValueBucket(values);
    uniStore->Init();
    int64_t rowId = E_HAS_DB_ERROR;
    ret = uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
    auto rawRdb = uniStore->GetRaw();
    std::unique_ptr<RingtoneDefaultSetting> defaultSetting = RingtoneDefaultSetting::GetObj(rawRdb);
    defaultSetting->GetTonePathByDisplayName(name);
}
} // namespace Media
} // namespace OHOS