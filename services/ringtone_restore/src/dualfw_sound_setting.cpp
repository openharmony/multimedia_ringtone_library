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

#include "dualfw_sound_setting.h"

#include <functional>
#include <string>
#include <unordered_map>

#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
enum SettingItemType : int32_t {
    SETTING_ITEM_INVALID = -1,
    SETTING_ITEM_RINGTONE,
    SETTING_ITEM_RINGTONE2,
    SETTING_ITEM_NOTIFICATION,
    SETTING_ITEM_ALARM,
    SETTING_ITEM_MESSAGE,
    SETTING_ITEM_MESSAGE2,
    SETTING_ITEM_MAX,
};

enum DualfwConfRowType : int32_t {
    DUALFW_CONFROW_INVALID = -1,
    DUALFW_CONFROW_BASE,
    DUALFW_CONFROW_SET,
    DUALFW_CONFROW_PATH,
    DUALFW_CONFROW_MAX,
};

struct DualfwRowTypeInfo {
    SettingItemType item;
    DualfwConfRowType rowType;
    int32_t settingType;
    int32_t toneType;
};

static const std::unordered_map<std::string, DualfwRowTypeInfo> g_dualfwInfoMap = {
    {"notification_sound",      {SETTING_ITEM_NOTIFICATION, DUALFW_CONFROW_BASE, TONE_SETTING_TYPE_NOTIFICATION,
        NOTIFICATION_TONE_TYPE}},
    {"notification_sound_set",  {SETTING_ITEM_NOTIFICATION, DUALFW_CONFROW_SET,  TONE_SETTING_TYPE_NOTIFICATION,
        NOTIFICATION_TONE_TYPE}},
    {"notification_sound_path", {SETTING_ITEM_NOTIFICATION, DUALFW_CONFROW_PATH, TONE_SETTING_TYPE_NOTIFICATION,
        NOTIFICATION_TONE_TYPE}},
    {"alarm_alert",             {SETTING_ITEM_ALARM, DUALFW_CONFROW_BASE, TONE_SETTING_TYPE_ALARM, ALARM_TONE_TYPE}},
    {"alarm_alert_set",         {SETTING_ITEM_ALARM, DUALFW_CONFROW_SET,  TONE_SETTING_TYPE_ALARM, ALARM_TONE_TYPE}},
    {"alarm_alert_path",        {SETTING_ITEM_ALARM, DUALFW_CONFROW_PATH, TONE_SETTING_TYPE_ALARM, ALARM_TONE_TYPE}},
    {"ringtone",                {SETTING_ITEM_RINGTONE, DUALFW_CONFROW_BASE, TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_1}},
    {"ringtone_set",            {SETTING_ITEM_RINGTONE, DUALFW_CONFROW_SET,  TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_1}},
    {"ringtone_path",           {SETTING_ITEM_RINGTONE, DUALFW_CONFROW_PATH, TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_1}},
    {"ringtone2",               {SETTING_ITEM_RINGTONE2, DUALFW_CONFROW_BASE, TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_2}},
    {"ringtone2_set",           {SETTING_ITEM_RINGTONE2, DUALFW_CONFROW_SET,  TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_2}},
    {"ringtone2_path",          {SETTING_ITEM_RINGTONE2, DUALFW_CONFROW_PATH, TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_2}},
    {"message",                 {SETTING_ITEM_MESSAGE, DUALFW_CONFROW_BASE, TONE_SETTING_TYPE_SHOT,
        SHOT_TONE_TYPE_SIM_CARD_1}},
    {"message_path",            {SETTING_ITEM_MESSAGE, DUALFW_CONFROW_PATH, TONE_SETTING_TYPE_SHOT,
        SHOT_TONE_TYPE_SIM_CARD_1}},
    {"messageSub1",             {SETTING_ITEM_MESSAGE2, DUALFW_CONFROW_BASE, TONE_SETTING_TYPE_SHOT,
        SHOT_TONE_TYPE_SIM_CARD_2}},
    {"messageSub1_path",        {SETTING_ITEM_MESSAGE2, DUALFW_CONFROW_PATH, TONE_SETTING_TYPE_SHOT,
        SHOT_TONE_TYPE_SIM_CARD_2}},
};

DualfwSoundSetting::DualfwSoundSetting()
{}

void DualfwSoundSetting::SettingsTraval(std::function<void (DualfwSettingItem &)> func)
{
    for (auto setting : settings_) {
        func(setting.second);
    }
}

int32_t DualfwSoundSetting::ProcessConfRow(std::unique_ptr<DualFwConfRow> &conf)
{
    int32_t ret = E_SUCCESS;

    auto found = g_dualfwInfoMap.find(conf->name);
    if (found != g_dualfwInfoMap.end()) {
        if (found->second.rowType == DUALFW_CONFROW_BASE) {
            settings_[found->second.item].settingType = found->second.settingType;
            settings_[found->second.item].toneType = found->second.toneType;
            settings_[found->second.item].defaultSysSet = (conf->defaultSysSet == "true" ? true : false);
        } else if (found->second.rowType == DUALFW_CONFROW_SET) {
            try {
                settings_[found->second.item].setFlag = (std::stoi(conf->value) > 0 ? true : false);
            } catch (const std::invalid_argument& e) {
                settings_[found->second.item].setFlag = false;
                RINGTONE_INFO_LOG("invalid argument: %s", e.what());
            } catch (const std::out_of_range& e) {
                settings_[found->second.item].setFlag = false;
                RINGTONE_INFO_LOG("out of range: %s", e.what());
            }
        } else if (found->second.rowType == DUALFW_CONFROW_PATH) {
            settings_[found->second.item].toneFileName = RingtoneFileUtils::GetFileNameFromPath(conf->value);
        } else {
            ret = E_ERR;
        }
    } else {
        ret = E_ERR;
    }

    return ret;
}
} // namespace Media
} // namespace OHOS
