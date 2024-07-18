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

#include "dualfw_conf_loader.h"

#include <vector>

#include "ringtone_errno.h"

namespace OHOS {
namespace Media {

static const std::string SETTINGS_DATA_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_100";
static const std::string SETTINGS_DATA_FIELD_KEY = "KEYWORD";
static const std::string SETTINGS_DATA_FIELD_VAL = "VALUE";
static std::vector<std::string> COLUMNS = {SETTINGS_DATA_FIELD_VAL};
static Uri URI = Uri(SETTINGS_DATA_URI);

DualfwConfLoader::DualfwConfLoader() {}

int32_t DualfwConfLoader::Init()
{
    DataShare::CreateOptions options;
    RINGTONE_INFO_LOG("Getting data share helper with SETTINGS_DATA_URI = %{public}s", SETTINGS_DATA_URI.c_str());
    dataShareHelper_ = DataShare::DataShareHelper::Creator(SETTINGS_DATA_URI, options);
    if (dataShareHelper_ == nullptr) {
        RINGTONE_INFO_LOG("dataShareHelper_ is null, failed to get data share helper");
        return E_ERR;
    }
    RINGTONE_INFO_LOG("Successfully initialized.");
    return E_OK;
}

int32_t DualfwConfLoader::Load(DualFwConf& conf, const RestoreSceneType& type)
{
    if (dataShareHelper_ == nullptr) {
        RINGTONE_ERR_LOG("DualfwConfLoader is not initialized successfully");
        return E_ERR;
    }
    if (type == RestoreSceneType::RESTORE_SCENE_TYPE_DUAL_CLONE) {
        RINGTONE_INFO_LOG("Load configurations for RestoreSceneType::RESTORE_SCENE_TYPE_DUAL_CLONE");
        conf.notificationSoundPath = GetConf("notification_sound");
        conf.ringtonePath = GetConf("ringtone_path");
        conf.ringtone2Path = GetConf("ringtone2_path");
        conf.alarmAlertPath = GetConf("alarm_alert");
        conf.messagePath = GetConf("message_path");
        conf.messageSub1 = GetConf("message_sub1");
    } else {
        RINGTONE_INFO_LOG("Load configurations for RestoreSceneType::RESTORE_SCENE_TYPE_DUAL_UPGRADE");
        conf.notificationSoundPath = GetConf("notification_sound_path");
        conf.ringtonePath = GetConf("ringtone_path");
        conf.ringtone2Path = GetConf("ringtone2_path");
        conf.alarmAlertPath = GetConf("alarm_alert_path");
        conf.messagePath = GetConf("message_path");
        conf.messageSub1 = GetConf("message_sub1");
    }
    
    return E_OK;
}

void DualfwConfLoader::ShowConf(const DualFwConf& conf)
{
    RINGTONE_INFO_LOG("===================================================");
    RINGTONE_INFO_LOG("conf.notificationSoundPath  = %{public}s", conf.notificationSoundPath.c_str());
    RINGTONE_INFO_LOG("conf.ringtonePath = %{public}s", conf.ringtonePath.c_str());
    RINGTONE_INFO_LOG("conf.ringtone2Path = %{public}s", conf.ringtone2Path.c_str());
    RINGTONE_INFO_LOG("conf.alarmAlertPath = %{public}s", conf.alarmAlertPath.c_str());
    RINGTONE_INFO_LOG("conf.messagePath = %{public}s", conf.messagePath.c_str());
    RINGTONE_INFO_LOG("conf.messageSub1 = %{public}s", conf.messageSub1.c_str());
    RINGTONE_INFO_LOG("===================================================");
}

std::string DualfwConfLoader::GetConf(const std::string& key)
{
    DataShare::DataSharePredicates dataSharePredicates;
    dataSharePredicates.EqualTo(SETTINGS_DATA_FIELD_KEY, key);
    auto resultSet = dataShareHelper_->Query(URI, dataSharePredicates, COLUMNS);
    if (resultSet == nullptr) {
        RINGTONE_INFO_LOG("resultSet is null, failed to get value of key = %{public}s", key.c_str());
        return "";
    }

    int32_t numRows = 0;
    resultSet->GetRowCount(numRows);
    RINGTONE_INFO_LOG("%{public}d records found with key = %{public}s", numRows, key.c_str());
    if (numRows == 0) {
        RINGTONE_INFO_LOG("no record at key = %{public}s, returning an empty string.", key.c_str());
        return "";
    }
    int32_t columnIndex = 0;
    int32_t rowNumber = 0;
    resultSet->GoToRow(rowNumber);
    std::string valueResult = "";
    int32_t ret = resultSet->GetString(columnIndex, valueResult);
    if (ret != 0) {
        RINGTONE_INFO_LOG("failed to get string, returning an empty string.");
    } else {
        RINGTONE_INFO_LOG("valueResult = %{public}s", valueResult.c_str());
    }
    return valueResult;
}
} // namespace Media
} // namespace OHOS