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

#ifndef RINGTONE_RESTORE_CONST_H
#define RINGTONE_RESTORE_CONST_H

#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "ringtone_db_const.h"
#include "ringtone_type.h"
#include "ringtone_metadata.h"
#include "vibrate_type.h"

namespace OHOS {
namespace Media {

const std::string RINGTONE_BACKUP_SUFFIX_DIR = "/data/storage/el2/base/files/Ringtone";
const std::string RINGTONE_RESTORE_DIR = "/data/storage/el2/base/files/Ringtone";
static const std::string DUALFWK_CONF_VALUE_NULL = "null";
static const std::string DUALFWK_CONF_VIBRATE_MODE_STD = "standard_vibrator";
static const std::string DUALFWK_CONF_VIBRATE_MODE_NULL = "null_vibrator";
static const std::string DUALFWK_CONF_VIBRATE_MODE_FOLLOW = "follow_system_ringtone";
static const std::string VIBRATE_FILE_NAME_STD = "Classic.json";
static const std::string VIBRATE_FILE_SUFFIX = ".json";

enum RestoreSceneType : int32_t {
    RESTORE_SCENE_TYPE_INVILID = -1,
    RESTORE_SCENE_TYPE_DUAL_UPGRADE,
    RESTORE_SCENE_TYPE_SINGLE_CLONE,
    RESTORE_SCENE_TYPE_DUAL_CLONE,
    RESTORE_SCENE_TYPE_MAX,
};

struct DualFwkVibrateConf {
    std::string ringtone;
    std::string ringtone2;
    std::string message;
    std::string message2;
    std::string notification;
    std::string alarm;
};

struct ToneFileInfo {
    SimcardMode simcard{SIMCARD_MODE_INVALID};
    ToneSettingType settingType{TONE_SETTING_TYPE_INVALID};
    int32_t toneType{TONE_TYPE_INVALID};
    std::string tonePath{};
    int32_t toneSourceType{SOURCE_TYPE_NOT_SET};
};

struct VibrateFileInfo {
    SimcardMode simcard {SIMCARD_MODE_INVALID};
    ToneSettingType settingType{TONE_SETTING_TYPE_INVALID};
    ToneType toneType{TONE_TYPE_INVALID};
    VibrateType vibrateType{VIBRATE_TYPE_INVALID};
    VibratePlayMode vibrateMode{VIBRATE_PLAYMODE_INVALID};
    std::string displayName;
};

struct FileInfo {
    int32_t toneId {0};
    std::string data;
    int64_t size {0};
    std::string displayName {};
    std::string title {};
    int32_t mediaType {0};
    int32_t toneType {0};
    std::string mimeType {};
    int32_t sourceType {0};
    int64_t dateAdded {0};
    int64_t dateModified {0};
    int64_t dateTaken {0};
    int32_t duration {0};
    int32_t shotToneType {0};
    int32_t shotToneSourceType {0};
    int32_t notificationToneType {0};
    int32_t notificationToneSourceType {0};
    int32_t ringToneType {0};
    int32_t ringToneSourceType {0};
    int32_t alarmToneType {0};
    int32_t alarmToneSourceType {0};
    std::string restorePath {};
    int32_t scannerFlag {0};
    bool doInsert {true};
    SimcardMode simcard {SIMCARD_MODE_INVALID};
    VibrateFileInfo vibrateInfo{};

    FileInfo() = default;
    FileInfo(const RingtoneMetadata &meta): toneId(meta.GetToneId()),
        data(meta.GetData()),
        size(meta.GetSize()),
        displayName(meta.GetDisplayName()),
        title(meta.GetTitle()),
        mediaType(meta.GetMediaType()),
        toneType(meta.GetToneType()),
        mimeType(meta.GetMimeType()),
        sourceType(meta.GetSourceType()),
        dateAdded(meta.GetDateAdded()),
        dateModified(meta.GetDateModified()),
        dateTaken(meta.GetDateTaken()),
        duration(meta.GetDuration()),
        shotToneType(meta.GetShotToneType()),
        shotToneSourceType(meta.GetShotToneSourceType()),
        notificationToneType(meta.GetNotificationToneType()),
        notificationToneSourceType(meta.GetNotificationToneSourceType()),
        ringToneType(meta.GetRingToneType()),
        ringToneSourceType(meta.GetRingToneSourceType()),
        alarmToneType(meta.GetAlarmToneType()),
        alarmToneSourceType(meta.GetAlarmToneSourceType()),
        scannerFlag(meta.GetScannerFlag()) {}

    std::string toString() const
    {
        return data + "|" + displayName + "|" + title + "|size=" + std::to_string(size) +
            "|sourceType=" + std::to_string(sourceType) +
            "|shotToneType=" + std::to_string(shotToneType) +
            "|shotToneSourceType=" + std::to_string(shotToneSourceType) +
            "|notificationToneType=" + std::to_string(notificationToneType) +
            "|notificationToneSourceType=" + std::to_string(notificationToneSourceType) +
            "|ringToneType=" + std::to_string(ringToneType) +
            "|ringToneSourceType=" + std::to_string(ringToneSourceType) +
            "|doInsert=" + std::to_string(doInsert) +
            "|restorePath=" + restorePath + "|simcard=" + std::to_string(simcard);
    }
};
} // namespace Media
} // namespace OHOS

#endif  // RINGTONE_RESTORE_CONST_H
