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

#include "dualfwk_sound_setting.h"

#include <functional>
#include <string>
#include <unordered_map>

#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_type.h"
#include "ringtone_utils.h"

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

enum DualFwkConfRowType : int32_t {
    DUALFWK_CONFROW_INVALID = -1,
    DUALFWK_CONFROW_BASE,
    DUALFWK_CONFROW_SET,
    DUALFWK_CONFROW_PATH,
    DUALFWK_CONFROW_MAX,
};

struct DualFwkRowTypeInfo {
    SettingItemType item;
    DualFwkConfRowType rowType;
    ToneSettingType settingType;
    int32_t toneType;
};

static const std::unordered_map<std::string, DualFwkRowTypeInfo> g_dualFwkInfoMap = {
    {"notification_sound",      {SETTING_ITEM_NOTIFICATION, DUALFWK_CONFROW_BASE, TONE_SETTING_TYPE_NOTIFICATION,
        NOTIFICATION_TONE_TYPE}},
    {"notification_sound_set",  {SETTING_ITEM_NOTIFICATION, DUALFWK_CONFROW_SET,  TONE_SETTING_TYPE_NOTIFICATION,
        NOTIFICATION_TONE_TYPE}},
    {"notification_sound_path", {SETTING_ITEM_NOTIFICATION, DUALFWK_CONFROW_PATH, TONE_SETTING_TYPE_NOTIFICATION,
        NOTIFICATION_TONE_TYPE}},
    {"alarm_alert",             {SETTING_ITEM_ALARM, DUALFWK_CONFROW_BASE, TONE_SETTING_TYPE_ALARM, ALARM_TONE_TYPE}},
    {"alarm_alert_set",         {SETTING_ITEM_ALARM, DUALFWK_CONFROW_SET,  TONE_SETTING_TYPE_ALARM, ALARM_TONE_TYPE}},
    {"alarm_alert_path",        {SETTING_ITEM_ALARM, DUALFWK_CONFROW_PATH, TONE_SETTING_TYPE_ALARM, ALARM_TONE_TYPE}},
    {"ringtone",                {SETTING_ITEM_RINGTONE, DUALFWK_CONFROW_BASE, TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_1}},
    {"ringtone_set",            {SETTING_ITEM_RINGTONE, DUALFWK_CONFROW_SET,  TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_1}},
    {"ringtone_path",           {SETTING_ITEM_RINGTONE, DUALFWK_CONFROW_PATH, TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_1}},
    {"ringtone2",               {SETTING_ITEM_RINGTONE2, DUALFWK_CONFROW_BASE, TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_2}},
    {"ringtone2_set",           {SETTING_ITEM_RINGTONE2, DUALFWK_CONFROW_SET,  TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_2}},
    {"ringtone2_path",          {SETTING_ITEM_RINGTONE2, DUALFWK_CONFROW_PATH, TONE_SETTING_TYPE_RINGTONE,
        RING_TONE_TYPE_SIM_CARD_2}},
    {"message",                 {SETTING_ITEM_MESSAGE, DUALFWK_CONFROW_BASE, TONE_SETTING_TYPE_SHOT,
        SHOT_TONE_TYPE_SIM_CARD_1}},
    {"message_path",            {SETTING_ITEM_MESSAGE, DUALFWK_CONFROW_PATH, TONE_SETTING_TYPE_SHOT,
        SHOT_TONE_TYPE_SIM_CARD_1}},
    {"messageSub1",             {SETTING_ITEM_MESSAGE2, DUALFWK_CONFROW_BASE, TONE_SETTING_TYPE_SHOT,
        SHOT_TONE_TYPE_SIM_CARD_2}},
    {"messageSub1_path",        {SETTING_ITEM_MESSAGE2, DUALFWK_CONFROW_PATH, TONE_SETTING_TYPE_SHOT,
        SHOT_TONE_TYPE_SIM_CARD_2}},
};

DualFwkSoundSetting::DualFwkSoundSetting()
{}

void DualFwkSoundSetting::SettingsTraval(std::function<void (DualFwkSettingItem &)> func)
{
    for (auto setting : settings_) {
        func(setting);
    }
}

std::vector<std::string> DualFwkSoundSetting::GetFileNames() const
{
    std::vector<std::string> fileNames;
    for (auto setting : settings_) {
        fileNames.push_back(setting.toneSetting.tonePath);
    }
    return fileNames;
}

std::vector<std::string> DualFwkSoundSetting::GetDisplayNames() const
{
    std::vector<std::string> fileNames;
    for (auto setting : settings_) {
        if (setting.isTitle) {
            std::string fileName = RingtoneUtils::ReplaceAll(setting.toneSetting.tonePath + ".ogg", " ", "_");
            fileNames.push_back(fileName);
        } else {
            fileNames.push_back(setting.toneSetting.tonePath);
        }
    }
    return fileNames;
}

std::vector<DualFwkSettingItem> DualFwkSoundSetting::GetSettings() const
{
    std::vector<DualFwkSettingItem> settings;
    for (auto setting : settings_) {
        settings.push_back(setting);
    }
    return settings;
}

int32_t DualFwkSoundSetting::ProcessConfRow(std::unique_ptr<DualFwkConfRow> &conf)
{
    if (conf == nullptr) {
        RINGTONE_ERR_LOG("invalid conf");
        return E_ERR;
    }
    int32_t ret = E_SUCCESS;

    auto found = g_dualFwkInfoMap.find(conf->name);
    if (found != g_dualFwkInfoMap.end()) {
        if (found->second.rowType == DUALFWK_CONFROW_BASE) {
            settings_[found->second.item].toneSetting.settingType = found->second.settingType;
            settings_[found->second.item].toneSetting.toneType = found->second.toneType;
            settings_[found->second.item].defaultSysSet = (conf->defaultSysSet == "true" ? true : false);
        } else if (found->second.rowType == DUALFWK_CONFROW_SET) {
            try {
                settings_[found->second.item].setFlag = (std::stoi(conf->value) > 0 ? true : false);
            } catch (const std::invalid_argument& e) {
                settings_[found->second.item].setFlag = false;
                RINGTONE_INFO_LOG("invalid argument: %s", e.what());
            } catch (const std::out_of_range& e) {
                settings_[found->second.item].setFlag = false;
                RINGTONE_INFO_LOG("out of range: %s", e.what());
            }
        } else if (found->second.rowType == DUALFWK_CONFROW_PATH) {
            settings_[found->second.item].toneSetting.tonePath = RingtoneFileUtils::GetFileNameFromPath(conf->value);
        } else {
            ret = E_ERR;
        }
    } else {
        ret = E_ERR;
    }

    return ret;
}


void DualFwkSoundSetting::ProcessConf(const DualFwkConf &conf)
{
    bool noRingtone = conf.ringtone == DUALFWK_CONF_VALUE_NULL;
    bool noRingtone2 = conf.ringtone2 == DUALFWK_CONF_VALUE_NULL;
    bool noMessage = conf.message == DUALFWK_CONF_VALUE_NULL;
    bool noMessage2 = conf.messageSub1 == DUALFWK_CONF_VALUE_NULL;
    bool noNotification = conf.notificationSound == DUALFWK_CONF_VALUE_NULL;
    bool noAlarm = conf.alarmAlert == DUALFWK_CONF_VALUE_NULL;

    int32_t toneType = noRingtone ? RING_TONE_TYPE_NOT : RING_TONE_TYPE_SIM_CARD_1;
    int32_t sourceType = noRingtone ? SOURCE_TYPE_INVALID : SOURCE_TYPE_NOT_SET;
    AddSetting({SIMCARD_MODE_1, TONE_SETTING_TYPE_RINGTONE, toneType, conf.ringtonePath, sourceType},
        {SIMCARD_MODE_1, TONE_SETTING_TYPE_RINGTONE, TONE_TYPE_RINGTONE, VIBRATE_TYPE_SRINGTONE,
            ExtractVibrateMode(conf.vibrateConf.ringtone), ExtractFileName(conf.vibrateConf.ringtone)});

    toneType = noRingtone2 ? RING_TONE_TYPE_NOT : RING_TONE_TYPE_SIM_CARD_2;
    sourceType = noRingtone2 ? SOURCE_TYPE_INVALID : SOURCE_TYPE_NOT_SET;
    AddSetting({SIMCARD_MODE_2, TONE_SETTING_TYPE_RINGTONE, toneType, conf.ringtone2Path, sourceType},
        {SIMCARD_MODE_2, TONE_SETTING_TYPE_RINGTONE, TONE_TYPE_RINGTONE, VIBRATE_TYPE_SRINGTONE,
            ExtractVibrateMode(conf.vibrateConf.ringtone2), ExtractFileName(conf.vibrateConf.ringtone2)});

    toneType = noMessage ? SHOT_TONE_TYPE_NOT : SHOT_TONE_TYPE_SIM_CARD_1;
    sourceType = noMessage ? SOURCE_TYPE_INVALID : SOURCE_TYPE_NOT_SET;
    AddSetting({SIMCARD_MODE_1, TONE_SETTING_TYPE_SHOT, toneType, conf.messagePath, sourceType},
        {SIMCARD_MODE_1, TONE_SETTING_TYPE_SHOT, TONE_TYPE_NOTIFICATION, VIBRATE_TYPE_SNOTIFICATION,
            ExtractVibrateMode(conf.vibrateConf.message), ExtractFileName(conf.vibrateConf.message)});

    toneType = noMessage2 ? SHOT_TONE_TYPE_NOT : SHOT_TONE_TYPE_SIM_CARD_2;
    sourceType = noMessage2 ? SOURCE_TYPE_INVALID : SOURCE_TYPE_NOT_SET;
    AddSetting({SIMCARD_MODE_2, TONE_SETTING_TYPE_SHOT, toneType, conf.messageSub1, sourceType},
        {SIMCARD_MODE_2, TONE_SETTING_TYPE_SHOT, TONE_TYPE_NOTIFICATION, VIBRATE_TYPE_SNOTIFICATION,
            ExtractVibrateMode(conf.vibrateConf.message2), ExtractFileName(conf.vibrateConf.message2)});

    toneType = noNotification ? NOTIFICATION_TONE_TYPE_NOT : NOTIFICATION_TONE_TYPE;
    sourceType = noNotification ? SOURCE_TYPE_INVALID : SOURCE_TYPE_NOT_SET;
    AddSetting({SIMCARD_MODE_1, TONE_SETTING_TYPE_NOTIFICATION, toneType, conf.notificationSoundPath, sourceType},
        {SIMCARD_MODE_1, TONE_SETTING_TYPE_NOTIFICATION, TONE_TYPE_NOTIFICATION, VIBRATE_TYPE_SNOTIFICATION,
            ExtractVibrateMode(conf.vibrateConf.notification), ExtractFileName(conf.vibrateConf.notification)});

    toneType = noAlarm ? ALARM_TONE_TYPE_NOT : ALARM_TONE_TYPE;
    sourceType = noAlarm ? SOURCE_TYPE_INVALID : SOURCE_TYPE_NOT_SET;
    AddSetting({SIMCARD_MODE_1, TONE_SETTING_TYPE_ALARM, toneType, conf.alarmAlertPath, sourceType},
        {SIMCARD_MODE_1, TONE_SETTING_TYPE_ALARM, TONE_TYPE_ALARM, VIBRATE_TYPE_SALARM,
            ExtractVibrateMode(conf.vibrateConf.alarm), ExtractFileName(conf.vibrateConf.alarm)});
}

VibratePlayMode DualFwkSoundSetting::ExtractVibrateMode(const std::string &input)
{
    VibratePlayMode playMode = VIBRATE_PLAYMODE_INVALID;
    do {
        auto pos = input.find(DUALFWK_CONF_VIBRATE_MODE_STD);
        if (pos != std::string::npos) {
            playMode = VIBRATE_PLAYMODE_CLASSIC;
            break;
        }
        pos = input.find(DUALFWK_CONF_VIBRATE_MODE_NULL);
        if (pos != std::string::npos) {
            playMode = VIBRATE_PLAYMODE_NONE;
            break;
        }
        pos = input.find(DUALFWK_CONF_VIBRATE_MODE_FOLLOW);
        if (pos != std::string::npos) {
            playMode = VIBRATE_PLAYMODE_SYNC;
        }
    } while (false);
    return playMode;
}

std::string DualFwkSoundSetting::ExtractFileName(const std::string &input)
{
    std::string ret{};
    if (input == DUALFWK_CONF_VIBRATE_MODE_NULL ||
        input == DUALFWK_CONF_VIBRATE_MODE_FOLLOW) {
        return ret;
    }

    auto pos = input.find(DUALFWK_CONF_VIBRATE_MODE_STD);
    if (pos == std::string::npos) {
        return ret;
    }

    pos = input.find('_', pos + DUALFWK_CONF_VIBRATE_MODE_STD.length());
    if (pos == std::string::npos) {
        return VIBRATE_FILE_NAME_STD;
    }
    ret = input.substr(pos + 1);
    if (!ret.empty()) {
        ret += VIBRATE_FILE_SUFFIX;
    }
    return ret;
}

void DualFwkSoundSetting::AddSetting(const ToneFileInfo &toneConf, const VibrateFileInfo &vibrateConf)
{
    ToneFileInfo newToneConf = toneConf;
    bool isTitle = false;
    if (!toneConf.tonePath.empty()) {
        newToneConf.tonePath = RingtoneFileUtils::GetFileNameFromPathOrUri(toneConf.tonePath, isTitle);
    }
    settings_.push_back({newToneConf, false, true, isTitle, vibrateConf});
}

} // namespace Media
} // namespace OHOS
