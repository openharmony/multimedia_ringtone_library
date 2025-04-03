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

#include "ringtone_setting_manager_test.h"

#include <memory>

#include "ability_context_impl.h"
#include "ringtone_errno.h"
#include "ringtone_rdbstore.h"
#define private public
#include "ringtone_setting_manager.h"
#undef private
#include "rdb_store.h"
#include "rdb_utils.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
std::shared_ptr<RingtoneSettingManager> g_ringtoneSettingManager;
shared_ptr<RingtoneUnistore> g_uniStore;

void RingtoneSettingManagerTest::SetUpTestCase()
{
}

void RingtoneSettingManagerTest::TearDownTestCase()
{
}

// SetUp:Execute before each test case
void RingtoneSettingManagerTest::SetUp()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    g_uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    auto ret = g_uniStore->Init();
    EXPECT_EQ(ret, E_OK);
    auto rawRdb = g_uniStore->GetRaw();
    g_ringtoneSettingManager = std::make_shared<RingtoneSettingManager>(rawRdb);
    EXPECT_NE(g_ringtoneSettingManager, nullptr);
}

void RingtoneSettingManagerTest::TearDown(void) {}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_UpdateShotSetting_test_001, TestSize.Level0)
{
    std::shared_ptr<RingtoneMetadata> ringtoneMetadata = std::make_shared<RingtoneMetadata>();
    int32_t shotToneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_BOTH);
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1);
    int32_t sourceType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_2);
    ringtoneMetadata->SetShotToneType(shotToneType);
    ringtoneMetadata->SetToneId(0);
    auto ret = g_ringtoneSettingManager->UpdateShotSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    shotToneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_NOT);
    ringtoneMetadata->SetShotToneType(shotToneType);
    ret = g_ringtoneSettingManager->UpdateShotSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    shotToneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1);
    ringtoneMetadata->SetShotToneType(shotToneType);
    ret = g_ringtoneSettingManager->UpdateShotSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_UpdateRingtoneSetting_test_001, TestSize.Level0)
{
    std::shared_ptr<RingtoneMetadata> ringtoneMetadata = std::make_shared<RingtoneMetadata>();
    int32_t ringToneType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_BOTH);
    int32_t toneType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_1);
    int32_t sourceType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_2);
    ringtoneMetadata->SetRingToneType(ringToneType);
    ringtoneMetadata->SetToneId(0);
    auto ret = g_ringtoneSettingManager->UpdateRingtoneSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    ringToneType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_NOT);
    ringtoneMetadata->SetRingToneType(ringToneType);
    ret = g_ringtoneSettingManager->UpdateRingtoneSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    ringToneType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_1);
    ringtoneMetadata->SetRingToneType(ringToneType);
    ret = g_ringtoneSettingManager->UpdateRingtoneSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_UpdateNotificationSetting_test_001, TestSize.Level0)
{
    std::shared_ptr<RingtoneMetadata> ringtoneMetadata = std::make_shared<RingtoneMetadata>();
    int32_t toneType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE);
    int32_t sourceType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE_NOT);
    ringtoneMetadata->SetToneId(0);
    auto ret = g_ringtoneSettingManager->UpdateNotificationSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    toneType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE_MAX);
    sourceType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE_MAX);
    ret = g_ringtoneSettingManager->UpdateNotificationSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_UpdateAlarmSetting_test_001, TestSize.Level0)
{
    std::shared_ptr<RingtoneMetadata> ringtoneMetadata = std::make_shared<RingtoneMetadata>();
    int32_t toneType = static_cast<int32_t>(AlarmToneType::ALARM_TONE_TYPE);
    int32_t sourceType = static_cast<int32_t>(AlarmToneType::ALARM_TONE_TYPE_NOT);
    ringtoneMetadata->SetToneId(0);
    auto ret = g_ringtoneSettingManager->UpdateAlarmSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    g_uniStore->Stop();
    ret = g_ringtoneSettingManager->UpdateAlarmSetting(ringtoneMetadata, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_UpdateSettingsByPath_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    string data = ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    cmd.SetValueBucket(values);
    int64_t rowId = E_HAS_DB_ERROR;
    auto ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_SHOT);
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1);
    int32_t sourceType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_2);
    ret = g_ringtoneSettingManager->UpdateSettingsByPath(data, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_RINGTONE);
    toneType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_1);
    sourceType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_2);
    ret = g_ringtoneSettingManager->UpdateSettingsByPath(data, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_NOTIFICATION);
    toneType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE);
    sourceType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE_NOT);
    ret = g_ringtoneSettingManager->UpdateSettingsByPath(data, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_ALARM);
    toneType = static_cast<int32_t>(AlarmToneType::ALARM_TONE_TYPE);
    sourceType = static_cast<int32_t>(AlarmToneType::ALARM_TONE_TYPE_NOT);
    ret = g_ringtoneSettingManager->UpdateSettingsByPath(data, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_MAX);
    ret = g_ringtoneSettingManager->UpdateSettingsByPath(data, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);

    RingtoneDataCommand cmdDelete(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    DataShare::DataSharePredicates predicates;
    NativeRdb::RdbPredicates rdbPredicate = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, RINGTONE_TABLE);
    cmdDelete.GetAbsRdbPredicates()->SetWhereClause(rdbPredicate.GetWhereClause());
    cmdDelete.GetAbsRdbPredicates()->SetWhereArgs(rdbPredicate.GetWhereArgs());
    int32_t deletedRows = 1;
    ret = g_uniStore->Delete(cmdDelete, deletedRows);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_UpdateSettingsWithToneId_test_001, TestSize.Level0)
{
    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_SHOT);
    int32_t toneId = 0;
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1);
    auto ret = g_ringtoneSettingManager->UpdateSettingsWithToneId(settingType, toneId, toneType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_RINGTONE);
    toneType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_1);
    ret = g_ringtoneSettingManager->UpdateSettingsWithToneId(settingType, toneId, toneType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_NOTIFICATION);
    toneType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE);
    ret = g_ringtoneSettingManager->UpdateSettingsWithToneId(settingType, toneId, toneType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_ALARM);
    toneType = static_cast<int32_t>(AlarmToneType::ALARM_TONE_TYPE);
    g_uniStore->Stop();
    ret = g_ringtoneSettingManager->UpdateSettingsWithToneId(settingType, toneId, toneType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_MAX);
    ret = g_ringtoneSettingManager->UpdateSettingsWithToneId(settingType, toneId, toneType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_CleanupSettingFromRdb_test_001, TestSize.Level0)
{
    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_SHOT);
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1);
    int32_t sourceType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_2);
    auto ret = g_ringtoneSettingManager->CleanupSettingFromRdb(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_BOTH);
    ret = g_ringtoneSettingManager->CleanupSettingFromRdb(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_RINGTONE);
    toneType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_1);
    sourceType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_2);
    ret = g_ringtoneSettingManager->CleanupSettingFromRdb(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_NOTIFICATION);
    toneType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE);
    ret = g_ringtoneSettingManager->CleanupSettingFromRdb(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_ALARM);
    toneType = static_cast<int32_t>(AlarmToneType::ALARM_TONE_TYPE);
    g_uniStore->Stop();
    ret = g_ringtoneSettingManager->CleanupSettingFromRdb(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_MAX);
    ret = g_ringtoneSettingManager->CleanupSettingFromRdb(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_CleanupSetting_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    string data = ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    values.Put(RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET));
    values.Put(RINGTONE_COLUMN_SHOT_TONE_TYPE, static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1));
    cmd.SetValueBucket(values);
    int64_t rowId = E_HAS_DB_ERROR;
    auto ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);

    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_SHOT);
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_BOTH);
    int32_t sourceType = static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET);
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1);
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    NativeRdb::ValuesBucket val;
    val.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    data = ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "rdbStore_Insert_test_002";
    val.PutString(RINGTONE_COLUMN_DATA, data);
    val.PutString(RINGTONE_COLUMN_TITLE, title);
    val.Put(RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET));
    val.Put(RINGTONE_COLUMN_SHOT_TONE_TYPE, static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_BOTH));
    cmd.SetValueBucket(val);
    ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);

    RingtoneDataCommand cmdDelete(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    DataShare::DataSharePredicates predicates;
    NativeRdb::RdbPredicates rdbPredicate = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, RINGTONE_TABLE);
    cmdDelete.GetAbsRdbPredicates()->SetWhereClause(rdbPredicate.GetWhereClause());
    cmdDelete.GetAbsRdbPredicates()->SetWhereArgs(rdbPredicate.GetWhereArgs());
    int32_t deletedRows = 1;
    ret = g_uniStore->Delete(cmdDelete, deletedRows);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_CleanupSetting_test_002, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    string data = ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    values.Put(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET));
    values.Put(RINGTONE_COLUMN_RING_TONE_TYPE, static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_1));
    cmd.SetValueBucket(values);
    int64_t rowId = E_HAS_DB_ERROR;
    auto ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);

    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_RINGTONE);
    int32_t toneType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_BOTH);
    int32_t sourceType = static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET);
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    toneType = static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_1);
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    NativeRdb::ValuesBucket val;
    val.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    data = ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "rdbStore_Insert_test_002";
    val.PutString(RINGTONE_COLUMN_DATA, data);
    val.PutString(RINGTONE_COLUMN_TITLE, title);
    val.Put(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET));
    val.Put(RINGTONE_COLUMN_RING_TONE_TYPE, static_cast<int32_t>(RingToneType::RING_TONE_TYPE_SIM_CARD_BOTH));
    cmd.SetValueBucket(val);
    ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);

    RingtoneDataCommand cmdDelete(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    DataShare::DataSharePredicates predicates;
    NativeRdb::RdbPredicates rdbPredicate = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, RINGTONE_TABLE);
    cmdDelete.GetAbsRdbPredicates()->SetWhereClause(rdbPredicate.GetWhereClause());
    cmdDelete.GetAbsRdbPredicates()->SetWhereArgs(rdbPredicate.GetWhereArgs());
    int32_t deletedRows = 1;
    ret = g_uniStore->Delete(cmdDelete, deletedRows);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_CleanupSetting_test_003, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    NativeRdb::ValuesBucket values;
    const string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    const string data = ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    values.Put(RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET));
    values.Put(RINGTONE_COLUMN_ALARM_TONE_TYPE, static_cast<int32_t>(AlarmToneType::ALARM_TONE_TYPE));
    cmd.SetValueBucket(values);
    int64_t rowId = E_HAS_DB_ERROR;
    auto ret = g_uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);

    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_ALARM);
    int32_t toneType = static_cast<int32_t>(AlarmToneType::ALARM_TONE_TYPE);
    int32_t sourceType = static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET);
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_NOTIFICATION);
    toneType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE);
    sourceType = static_cast<int32_t>(NotificationToneType::NOTIFICATION_TONE_TYPE);
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_MAX);
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    g_ringtoneSettingManager->ringtoneRdb_ = nullptr;
    ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_DB_FAIL);

    RingtoneDataCommand cmdDelete(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    DataShare::DataSharePredicates predicates;
    NativeRdb::RdbPredicates rdbPredicate = RdbDataShareAdapter::RdbUtils::ToPredicates(predicates, RINGTONE_TABLE);
    cmdDelete.GetAbsRdbPredicates()->SetWhereClause(rdbPredicate.GetWhereClause());
    cmdDelete.GetAbsRdbPredicates()->SetWhereArgs(rdbPredicate.GetWhereArgs());
    int32_t deletedRows = 1;
    ret = g_uniStore->Delete(cmdDelete, deletedRows);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_CommitSetting_test_001, TestSize.Level0)
{
    std::shared_ptr<RingtoneMetadata> ringtoneMetadata = std::make_shared<RingtoneMetadata>();
    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_ALARM);
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_BOTH);
    int32_t sourceType = static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET);
    int32_t toneId = 0;
    string tonePath = "settingMetadata_CommitSetting_test_001";
    auto ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_SHOT);
    ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    g_ringtoneSettingManager->FlushSettings();
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_RINGTONE);
    ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_NOT);
    ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_SHOT);
    toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1);
    ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_2);
    ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_RINGTONE);
    toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1);
    ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_FAIL);
    g_ringtoneSettingManager->FlushSettings();
    toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_1);
    ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
    toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_2);
    ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneSettingManagerTest, settingMetadata_CommitSettingCompare_test_001, TestSize.Level0)
{
    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_ALARM);
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_BOTH);
    int32_t sourceType = static_cast<int32_t>(SourceType::SOURCE_TYPE_MAX);
    auto ret = g_ringtoneSettingManager->CommitSettingCompare(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    sourceType = static_cast<int32_t>(SourceType::SOURCE_TYPE_CUSTOMISED);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_INVALID);
    ret = g_ringtoneSettingManager->CommitSettingCompare(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_MAX);
    ret = g_ringtoneSettingManager->CommitSettingCompare(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
    settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_NOTIFICATION);
    toneType = NOTIFICATION_TONE_TYPE;
    ret = g_ringtoneSettingManager->CommitSettingCompare(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);
}

/*
 * Feature: Service
 * Function: Test RingtoneSettingManager with CommitSettingCompare
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test CommitSettingCompare for abnormal branches
 */
HWTEST_F(RingtoneSettingManagerTest, settingMetadata_CommitSettingCompare_test_002, TestSize.Level0)
{
    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_SHOT);
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_NOT);
    int32_t sourceType = static_cast<int32_t>(SourceType::SOURCE_TYPE_CUSTOMISED);
    auto ret = g_ringtoneSettingManager->CommitSettingCompare(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);

    toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_MAX);
    ret = g_ringtoneSettingManager->CommitSettingCompare(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_INVALID_ARGUMENTS);
}

/*
 * Feature: Service
 * Function: Test RingtoneSettingManager with CommitSetting
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test CommitSetting when setting is existing
 */
HWTEST_F(RingtoneSettingManagerTest, settingMetadata_CommitSetting_test_002, TestSize.Level0)
{
    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_SHOT);
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_BOTH);
    int32_t sourceType = static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET);
    int32_t toneId = 0;
    string tonePath = "settingMetadata_CommitSetting_test_002";
    auto ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_OK);

    ret = g_ringtoneSettingManager->CommitSetting(toneId, tonePath, settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_FAIL);
}

/*
 * Feature: Service
 * Function: Test RingtoneSettingManager with FlushSettings
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test FlushSettings when ringtone rdb_ is nullptr
 */
HWTEST_F(RingtoneSettingManagerTest, settingMetadata_FlushSettings_test_001, TestSize.Level0)
{
    int32_t settingType = static_cast<int32_t>(ToneSettingType::TONE_SETTING_TYPE_SHOT);
    int32_t toneType = static_cast<int32_t>(ShotToneType::SHOT_TONE_TYPE_SIM_CARD_BOTH);
    int32_t sourceType = static_cast<int32_t>(SourceType::SOURCE_TYPE_PRESET);
    g_ringtoneSettingManager->ringtoneRdb_ = nullptr;
    auto ret = g_ringtoneSettingManager->CleanupSetting(settingType, toneType, sourceType);
    EXPECT_EQ(ret, E_DB_FAIL);
    g_ringtoneSettingManager->FlushSettings();
}

/*
 * Feature: Service
 * Function: Test RingtoneSettingManager with ExtractMetaFromColumn
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test ExtractMetaFromColumn when column name is invalid
 */
HWTEST_F(RingtoneSettingManagerTest, settingMetadata_ExtractMetaFromColumn_test_001, TestSize.Level0)
{
    std::unique_ptr<RingtoneMetadata> ringtoneMetadata = std::make_unique<RingtoneMetadata>();
    std::shared_ptr<NativeRdb::ResultSet> results;
    std::string columnName = "test";
    g_ringtoneSettingManager->ExtractMetaFromColumn(results, ringtoneMetadata, columnName);
    EXPECT_TRUE(ringtoneMetadata->memberFuncMap_.find(columnName) == ringtoneMetadata->memberFuncMap_.end());
}

/*
 * Feature: Service
 * Function: Test RingtoneSettingManager with Update
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test Update when ringtone rdb_ is nullptr
 */
HWTEST_F(RingtoneSettingManagerTest, settingMetadata_Update_test_001, TestSize.Level0)
{
    int changedRows = 0;
    NativeRdb::ValuesBucket values;
    const string name = "test_name";
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    NativeRdb::AbsRdbPredicates predicates(RINGTONE_TABLE);
    g_ringtoneSettingManager->ringtoneRdb_ = nullptr;
    int32_t ret = g_ringtoneSettingManager->Update(changedRows, values, predicates);
    EXPECT_EQ(ret, E_DB_FAIL);
}
} // namespace Media
} // namespace OHOS