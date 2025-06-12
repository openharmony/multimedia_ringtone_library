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

#ifndef RINGTONE_TYPE_H
#define RINGTONE_TYPE_H

#include "ringtone_errno.h"

namespace OHOS {
namespace Media {

enum ToneType : int32_t {
    TONE_TYPE_INVALID = -1,
    TONE_TYPE_ALARM,
    TONE_TYPE_RINGTONE,
    TONE_TYPE_NOTIFICATION,
    TONE_TYPE_SHOT,
    TONE_TYPE_CONTACTS,
    TONE_TYPE_MAX,
};

enum SourceType : int32_t {
    SOURCE_TYPE_INVALID = -1,
    SOURCE_TYPE_PRESET = 1,  // system preset
    SOURCE_TYPE_CUSTOMISED,  // user customised
    SOURCE_TYPE_MAX,
};

enum RingtoneResultSetDataType : int32_t {
    DATA_TYPE_NULL = 0,
    DATA_TYPE_STRING,
    DATA_TYPE_INT32,
    DATA_TYPE_INT64,
    DATA_TYPE_DOUBLE
};

enum RingtoneMediaType : int32_t {
    RINGTONE_MEDIA_TYPE_INVALID = -1,
    RINGTONE_MEDIA_TYPE_AUDIO = 2,
    RINGTONE_MEDIA_TYPE_VIDEO = 3,
    RINGTONE_MEDIA_TYPE_MAX
};

enum ToneSettingType : int32_t {
    TONE_SETTING_TYPE_INVALID = -1,
    TONE_SETTING_TYPE_ALARM = 0,
    TONE_SETTING_TYPE_SHOT,
    TONE_SETTING_TYPE_RINGTONE,
    TONE_SETTING_TYPE_NOTIFICATION,
    TONE_SETTING_TYPE_MAX
};

enum ShotToneType : int32_t {
    SHOT_TONE_TYPE_NOT = 0, // not set
    SHOT_TONE_TYPE_SIM_CARD_1, // SimCard 1
    SHOT_TONE_TYPE_SIM_CARD_2, // SimCard 2
    SHOT_TONE_TYPE_SIM_CARD_BOTH, // both 1 and 2
    SHOT_TONE_TYPE_MAX
};

enum NotificationToneType : int32_t {
    NOTIFICATION_TONE_TYPE_NOT = 0, // not set
    NOTIFICATION_TONE_TYPE, // notification tone
    NOTIFICATION_TONE_TYPE_MAX
};

enum RingToneType : int32_t {
    RING_TONE_TYPE_NOT = 0, // not set
    RING_TONE_TYPE_SIM_CARD_1, // SimCard 1
    RING_TONE_TYPE_SIM_CARD_2, // SimCard 2
    RING_TONE_TYPE_SIM_CARD_BOTH, // both 1 and 2
    RING_TONE_TYPE_MAX
};

enum AlarmToneType : int32_t {
    ALARM_TONE_TYPE_NOT = 0, // not set
    ALARM_TONE_TYPE, // alarm tone
    ALARM_TONE_TYPE_MAX
};

enum DefaultSystemToneType : int32_t {
    DEFAULT_RING_TYPE_SIM_CARD_1 = 1,
    DEFAULT_RING_TYPE_SIM_CARD_2,
    DEFAULT_SHOT_TYPE_SIM_CARD_1,
    DEFAULT_SHOT_TYPE_SIM_CARD_2,
    DEFAULT_NOTIFICATION_TYPE,
    DEFAULT_ALARM_TYPE,
};

inline int GetAppSandboxPathFromUri(std::string &uri)
{
    const std::string prefixPart = "/data/app/el2/";
    const std::string targetPrefix = "/data/storage/el2/base/files/Ringtone";
    if (uri.compare(0, prefixPart.size(), prefixPart) ==  0) {
        uri = targetPrefix;
        return E_OK;
    } else {
        return E_INVALID_URI;
    }
}

// ringtone system default setting
static const char PARAM_RINGTONE_SETTING_SHOT[] = "const.multimedia.system_tone_sim_card_0";
static const char PARAM_RINGTONE_SETTING_SHOT2[] = "const.multimedia.system_tone_sim_card_1";
static const char PARAM_RINGTONE_SETTING_NOTIFICATIONTONE[] = "const.multimedia.notification_tone";
static const char PARAM_RINGTONE_SETTING_RINGTONE[] = "const.multimedia.ringtone_sim_card_0";
static const char PARAM_RINGTONE_SETTING_RINGTONE2[] = "const.multimedia.ringtone_sim_card_1";
static const char PARAM_RINGTONE_SETTING_ALARM[] = "const.multimedia.alarm_tone";
// default value
const int32_t TONE_ID_DEFAULT = -1;
const std::string DATA_DEFAULT = {};
const int64_t SIZE_DEFAULT = 0;
const std::string DISPLAY_NAME_DEFAULT = {};
const std::string TITLE_DEFAULT = {};
const int32_t METADATA_MEDIA_TYPE_DEFAULT = RINGTONE_MEDIA_TYPE_AUDIO;
const int32_t TONE_TYPE_DEFAULT = TONE_TYPE_INVALID;
const std::string MIME_TYPE_DEFAULT = {};
const int32_t SOURCE_TYPE_DEFAULT = SOURCE_TYPE_INVALID;
const int64_t DATE_ADDED_DEFAULT = 0;
const int64_t DATE_MODIFIED_DEFAULT = 0;
const int64_t DATE_TAKEN_DEFAULT = 0;
const int32_t DURATION_DEFAULT = 0;
const int32_t SHOT_TONE_TYPE_DEFAULT = SHOT_TONE_TYPE_NOT;
const int32_t SHOT_TONE_SOURCE_TYPE_DEFAULT = SOURCE_TYPE_INVALID;
const int32_t NOTIFICATION_TONE_TYPE_DEFAULT = NOTIFICATION_TONE_TYPE_NOT;
const int32_t NOTIFICATION_TONE_SOURCE_TYPE_DEFAULT = SOURCE_TYPE_INVALID;
const int32_t RING_TONE_TYPE_DEFAULT = RING_TONE_TYPE_NOT;
const int32_t RING_TONE_SOURCE_TYPE_DEFAULT = SOURCE_TYPE_INVALID;
const int32_t ALARM_TONE_TYPE_DEFAULT = ALARM_TONE_TYPE_NOT;
const int32_t ALARM_TONE_SOURCE_TYPE_DEFAULT = SOURCE_TYPE_INVALID;
const int32_t TONE_SETTING_TYPE_DEFAULT = TONE_SETTING_TYPE_INVALID;
const int32_t SCANNER_FLAG_DEFAULT = 0;

const char RINGTONE_SLASH_CHAR = '/';
constexpr int32_t RINGTONE_DEFAULT_INT32 = 0;
constexpr int64_t RINGTONE_DEFAULT_INT64 = 0;
const std::string RINGTONE_DEFAULT_STR = "";

constexpr int32_t RINGTONE_VIDEO_MAX_COUNT = 40;

/** Supported ringtone types */
const std::string RINGTONE_CONTAINER_TYPE_3GA   = "3ga";
const std::string RINGTONE_CONTAINER_TYPE_AC3   = "ac3";
const std::string RINGTONE_CONTAINER_TYPE_A52   = "a52";
const std::string RINGTONE_CONTAINER_TYPE_AMR   = "amr";
const std::string RINGTONE_CONTAINER_TYPE_IMY   = "imy";
const std::string RINGTONE_CONTAINER_TYPE_RTTTL = "rtttl";
const std::string RINGTONE_CONTAINER_TYPE_XMF   = "xmf";
const std::string RINGTONE_CONTAINER_TYPE_RTX   = "rtx";
const std::string RINGTONE_CONTAINER_TYPE_MXMF  = "mxmf";
const std::string RINGTONE_CONTAINER_TYPE_M4A   = "m4a";
const std::string RINGTONE_CONTAINER_TYPE_M4B   = "m4b";
const std::string RINGTONE_CONTAINER_TYPE_M4P   = "m4p";
const std::string RINGTONE_CONTAINER_TYPE_F4A   = "f4a";
const std::string RINGTONE_CONTAINER_TYPE_F4B   = "f4b";
const std::string RINGTONE_CONTAINER_TYPE_F4P   = "f4p";
const std::string RINGTONE_CONTAINER_TYPE_M3U   = "m3u";
const std::string RINGTONE_CONTAINER_TYPE_SMF   = "smf";
const std::string RINGTONE_CONTAINER_TYPE_MKA   = "mka";
const std::string RINGTONE_CONTAINER_TYPE_RA    = "ra";
const std::string RINGTONE_CONTAINER_TYPE_MP3   = "mp3";
const std::string RINGTONE_CONTAINER_TYPE_AAC   = "aac";
const std::string RINGTONE_CONTAINER_TYPE_ADTS  = "adts";
const std::string RINGTONE_CONTAINER_TYPE_ADT   = "adt";
const std::string RINGTONE_CONTAINER_TYPE_SND   = "snd";
const std::string RINGTONE_CONTAINER_TYPE_FLAC  = "flac";
const std::string RINGTONE_CONTAINER_TYPE_MP2   = "mp2";
const std::string RINGTONE_CONTAINER_TYPE_MP1   = "mp1";
const std::string RINGTONE_CONTAINER_TYPE_MPA   = "mpa";
const std::string RINGTONE_CONTAINER_TYPE_M4R   = "m4r";
const std::string RINGTONE_CONTAINER_TYPE_WAV   = "wav";
const std::string RINGTONE_CONTAINER_TYPE_OGG   = "ogg";
const std::string RINGTONE_CONTAINER_TYPE_VIDEO_MP4   = "mp4";
} // namespace Media
} // namespace OHOS

#endif  // RINGTONE_TYPE_H
