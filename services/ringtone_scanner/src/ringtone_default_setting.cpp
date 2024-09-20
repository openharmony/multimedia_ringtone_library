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
#define MLOG_TAG "RingtoneDefaultSetting"

#include "ringtone_default_setting.h"

#include "parameter.h"
#include "rdb_errno.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_type.h"
#include "ringtone_scanner_db.h"
#include "ringtone_setting_manager.h"
#include "rdb_helper.h"
#include "result_set.h"
#include "ringtone_utils.h"

namespace OHOS {
namespace Media {
using namespace std;
static const int32_t SYSPARA_SIZE = 128;
std::unique_ptr<RingtoneDefaultSetting> RingtoneDefaultSetting::GetObj(std::shared_ptr<NativeRdb::RdbStore> &rdb)
{
    return make_unique<RingtoneDefaultSetting>(rdb);
}

RingtoneDefaultSetting::RingtoneDefaultSetting(shared_ptr<NativeRdb::RdbStore> &rdb)
{
    settingMgr_ = make_unique<RingtoneSettingManager>(rdb);
}

#ifndef OHOS_LOCAL_DEBUG_DISABLE
// liuxk just for debug
static const std::string LOCAL_DIR = "/data/storage/el2/base/preload_data";
#endif
string RingtoneDefaultSetting::GetTonePathByDisplayName(const string &name)
{
    string pathStr = {};
    string querySql = "SELECT data FROM ToneFiles WHERE display_name = "s + "\"" + name + "\"";
    settingMgr_->TravelQueryResultSet(querySql, [&](shared_ptr<RingtoneMetadata> &meta) -> bool {
        pathStr = meta->GetData();
        if (pathStr.empty()) {
            pathStr = {};
            return false;
        }
        if (pathStr.find(ROOT_TONE_PRELOAD_PATH_NOAH_PATH) == 0 ||
            pathStr.find(ROOT_TONE_PRELOAD_PATH_CHINA_PATH) == 0 ||
            #ifndef OHOS_LOCAL_DEBUG_DISABLE
            pathStr.find(LOCAL_DIR) == 0 ||
#endif
            pathStr.find(ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH) == 0) {
#ifndef OHOS_LOCAL_DEBUG_DISABLE
            RINGTONE_ERR_LOG("liuxk, find path=%{public}s", pathStr.c_str());
#endif
            return true;
        }
        pathStr = {};
        return false;
    });

    return pathStr;
}

void RingtoneDefaultSetting::ShotToneDefaultSettings()
{
    char paramValue1[SYSPARA_SIZE] = {0};
    char paramValue2[SYSPARA_SIZE] = {0};

    GetParameter(PARAM_RINGTONE_SETTING_SHOT, "", paramValue1, SYSPARA_SIZE);
    GetParameter(PARAM_RINGTONE_SETTING_SHOT2, "", paramValue2, SYSPARA_SIZE);

    if (strcmp(paramValue1, "")) {
        string tonePath = {};
        string strVal = {paramValue1};
        tonePath = GetTonePathByDisplayName(strVal);
        if (!tonePath.empty()) {
            settingMgr_->CommitSetting(TONE_ID_DEFAULT, tonePath, TONE_SETTING_TYPE_SHOT, SHOT_TONE_TYPE_SIM_CARD_1,
                SOURCE_TYPE_PRESET);
        }
    }

    if (strcmp(paramValue2, "")) {
        string tonePath = {};
        string strVal = {paramValue2};
        tonePath = GetTonePathByDisplayName(strVal);
        if (!tonePath.empty()) {
            settingMgr_->CommitSetting(TONE_ID_DEFAULT, tonePath, TONE_SETTING_TYPE_SHOT, SHOT_TONE_TYPE_SIM_CARD_2,
                SOURCE_TYPE_PRESET);
        }
    }
}

void RingtoneDefaultSetting::NotificationToneDefaultSettings()
{
    char paramValue[SYSPARA_SIZE] = {0};
    GetParameter(PARAM_RINGTONE_SETTING_NOTIFICATIONTONE, "", paramValue, SYSPARA_SIZE);

    if (strcmp(paramValue, "")) {
        string tonePath = {};
        string strVal = {paramValue};
        tonePath = GetTonePathByDisplayName(strVal);
        if (!tonePath.empty()) {
            settingMgr_->CommitSetting(TONE_ID_DEFAULT, tonePath, TONE_SETTING_TYPE_NOTIFICATION,
                NOTIFICATION_TONE_TYPE, SOURCE_TYPE_PRESET);
        }
    }
}

void RingtoneDefaultSetting::RingToneDefaultSettings()
{
    char paramValue1[SYSPARA_SIZE] = {0};
    char paramValue2[SYSPARA_SIZE] = {0};

    GetParameter(PARAM_RINGTONE_SETTING_RINGTONE, "", paramValue1, SYSPARA_SIZE);
    GetParameter(PARAM_RINGTONE_SETTING_RINGTONE2, "", paramValue2, SYSPARA_SIZE);

    if (strcmp(paramValue1, "")) {
        string tonePath = {};
        string strVal = {paramValue1};
        tonePath = GetTonePathByDisplayName(strVal);
        if (!tonePath.empty()) {
            settingMgr_->CommitSetting(TONE_ID_DEFAULT, tonePath, TONE_SETTING_TYPE_RINGTONE, RING_TONE_TYPE_SIM_CARD_1,
                SOURCE_TYPE_PRESET);
        }
    }

    if (strcmp(paramValue2, "")) {
        string tonePath = {};
        string strVal = {paramValue2};
        tonePath = GetTonePathByDisplayName(strVal);
        if (!tonePath.empty()) {
            settingMgr_->CommitSetting(TONE_ID_DEFAULT, tonePath, TONE_SETTING_TYPE_RINGTONE, RING_TONE_TYPE_SIM_CARD_2,
                SOURCE_TYPE_PRESET);
        }
    }
}

void RingtoneDefaultSetting::AlarmToneDefaultSettings()
{
    char paramValue[SYSPARA_SIZE] = {0};
    GetParameter(PARAM_RINGTONE_SETTING_ALARM, "", paramValue, SYSPARA_SIZE);

    if (strcmp(paramValue, "")) {
        string tonePath = {};
        string strVal = {paramValue};
        tonePath = GetTonePathByDisplayName(strVal);
        if (!tonePath.empty()) {
            settingMgr_->CommitSetting(TONE_ID_DEFAULT, tonePath, TONE_SETTING_TYPE_ALARM, ALARM_TONE_TYPE,
                SOURCE_TYPE_PRESET);
        }
    }
}

void RingtoneDefaultSetting::Update()
{
    ShotToneDefaultSettings();
    NotificationToneDefaultSettings();
    RingToneDefaultSettings();
    AlarmToneDefaultSettings();

    if (settingMgr_ != nullptr) {
        settingMgr_->FlushSettings();
    } else {
        RINGTONE_ERR_LOG("ringtone setting mgr is nullptr");
    }
    UpdateDefaultSystemTone();
}

void RingtoneDefaultSetting::UpdateDefaultSystemTone()
{
    RINGTONE_INFO_LOG("setting system tone begin");
    auto infos = RingtoneUtils::GetDefaultSystemtoneInfo();
    for (auto info : infos) {
        const string querySql = "SELECT tone_id FROM ToneFiles WHERE display_name = "s + "\"" + info.second + "\"";
        int32_t tone_id = 0;
        settingMgr_->TravelQueryResultSet(querySql, [&](shared_ptr<RingtoneMetadata> &meta) -> bool {
            tone_id = meta->GetToneId();
            return true;
        });

        NativeRdb::ValuesBucket values;
        values.PutString(PRELOAD_CONFIG_COLUMN_DISPLAY_NAME, info.second);
        values.PutInt(PRELOAD_CONFIG_COLUMN_TONE_ID, tone_id);
        NativeRdb::AbsRdbPredicates absRdbPredicates(PRELOAD_CONFIG_TABLE);
        absRdbPredicates.EqualTo(PRELOAD_CONFIG_COLUMN_RING_TONE_TYPE, std::to_string(info.first));
        int32_t changedRows = 0;
        int32_t result = settingMgr_->Update(changedRows, values, absRdbPredicates);
        if (result != E_OK || changedRows <= 0) {
            RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, changedRows);
        }
    }
}

} // namespace Media
} // namespace OHOS
