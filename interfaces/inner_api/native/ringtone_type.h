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
#include "ringtone_db_const.h"

namespace OHOS {
namespace Media {

enum ToneType : int32_t {
    TONE_TYPE_INVALID = -1,
    TONE_TYPE_ALARM,
    TONE_TYPE_RINGTONE,
    TONE_TYPE_NOTIFICATION,
    TONE_TYPE_SHOT,
    TONE_TYPE_CONTACTS,
    TONE_TYPE_APP_NOTIFICATION,
    TONE_TYPE_MAX,
};

enum SourceType : int32_t {
    SOURCE_TYPE_INVALID = -1,
    SOURCE_TYPE_NOT_SET = 0,  // not set
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
    DEFAULT_RING_TYPE_SIM_CARD_2 = 2,
    DEFAULT_SHOT_TYPE_SIM_CARD_1 = 3,
    DEFAULT_SHOT_TYPE_SIM_CARD_2 = 4,
    DEFAULT_NOTIFICATION_TYPE = 5,
    DEFAULT_ALARM_TYPE = 6,
    DEFAULT_RING_TYPE_ESIM_CARD_1 = 7,    // eSIM1来电 (新增)
    DEFAULT_SHOT_TYPE_ESIM_CARD_1 = 8,    // eSIM1短信 (新增)
    DEFAULT_RING_TYPE_ESIM_CARD_2 = 9,    // eSIM2来电 (新增)
    DEFAULT_SHOT_TYPE_ESIM_CARD_2 = 10,   // eSIM2短信 (新增)
};

enum SimcardMode : int32_t {
    SIMCARD_MODE_INVALID = 0,
    SIMCARD_MODE_1 = 1,       // 卡1 (SIM卡)
    SIMCARD_MODE_2 = 2,       // 卡2 (SIM卡)
    SIMCARD_MODE_BOTH = 3,  // 非卡相关(通知、闹钟)
    SIMCARD_MODE_ESIM_1 = 4,  // eSIM1 (新增)
    SIMCARD_MODE_ESIM_2 = 5,  // eSIM2 (新增)
};

enum RingMockHapticAudioType {
    RING_MOCK_HAPTIC_AUDIO_TYPE_INVALID = 0,
    RING_MOCK_HAPTIC_AUDIO_TYPE_CLASSIC_STANDARD = 1,
    RING_MOCK_HAPTIC_AUDIO_TYPE_CLASSIC_GENTLE = 2,
    RING_MOCK_HAPTIC_AUDIO_TYPE_ALARM_STANDARD = 3,
    RING_MOCK_HAPTIC_AUDIO_TYPE_RINGTONE_STANDARD = 4,
    RING_MOCK_HAPTIC_AUDIO_TYPE_NOTIFICATION_STANDARD = 5,
    RING_MOCK_HAPTIC_AUDIO_TYPE_ALARM_GENTLE = 6,
    RING_MOCK_HAPTIC_AUDIO_TYPE_RINGTONE_GENTLE = 7,
    RING_MOCK_HAPTIC_AUDIO_TYPE_NOTIFICATION_GENTLE = 8,
};

enum RingMockHapticAudioPlayMode {
    RING_MOCK_HAPTIC_AUDIO_PLAYMODE_INVALID = 0,
    RING_MOCK_HAPTIC_AUDIO_PLAYMODE_SYNC = 1,
    RING_MOCK_HAPTIC_AUDIO_PLAYMODE_CLASSIC = 2,
};

// 声音模式枚举 - 用于区分响铃模式/振动模式/静音模式
enum SoundMode : int32_t {
    SOUND_MODE_RING = 0,    // 响铃模式
    SOUND_MODE_VIBRATE = 1, // 振动模式
    SOUND_MODE_SILENT = 2,  // 静音模式（等同于响铃模式）
};

// 声音模式系数 - ringtone_type编码规则: soundMode * 100 + toneType
// 示例: 100 = 振动模式(1) * 100 + 闹钟(0)
//       101 = 振动模式(1) * 100 + 短信(1)
//       102 = 振动模式(1) * 100 + 来电(2)
//       103 = 振动模式(1) * 100 + 通知(3)
constexpr int32_t RINGTONE_TYPE_SOUND_MODE_FACTOR = 100;

// ringtone_type编解码函数
constexpr int32_t CalcRingtoneTypeByMode(int32_t soundMode, int32_t toneType)
{
    return soundMode * RINGTONE_TYPE_SOUND_MODE_FACTOR + toneType;
}

// 预定义的ringtone_type常量（方便外部调用）
// 响铃模式: 0=闹钟, 1=短信, 2=来电, 3=通知
constexpr int32_t RINGTONE_TYPE_RING_ALARM = CalcRingtoneTypeByMode(SOUND_MODE_RING, 0);           // 0
constexpr int32_t RINGTONE_TYPE_RING_SMS = CalcRingtoneTypeByMode(SOUND_MODE_RING, 1);           // 1
constexpr int32_t RINGTONE_TYPE_RING_RINGTONE = CalcRingtoneTypeByMode(SOUND_MODE_RING, 2);     // 2
constexpr int32_t RINGTONE_TYPE_RING_NOTIFICATION = CalcRingtoneTypeByMode(SOUND_MODE_RING, 3); // 3
// 振动模式: 100=振动-闹钟, 101=振动-短信, 102=振动-来电, 103=振动-通知
constexpr int32_t RINGTONE_TYPE_VIBRATE_ALARM = CalcRingtoneTypeByMode(SOUND_MODE_VIBRATE, 0);    // 100
constexpr int32_t RINGTONE_TYPE_VIBRATE_SMS = CalcRingtoneTypeByMode(SOUND_MODE_VIBRATE, 1);    // 101
constexpr int32_t RINGTONE_TYPE_VIBRATE_RINGTONE = CalcRingtoneTypeByMode(SOUND_MODE_VIBRATE, 2); // 102
constexpr int32_t RINGTONE_TYPE_VIBRATE_NOTIFICATION = CalcRingtoneTypeByMode(SOUND_MODE_VIBRATE, 3); // 103

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
// eSIM CCM configuration items (新增)
static const char PARAM_RINGTONE_ESIM_CARD_0[] = "const.multimedia.ringtone_esim_card_0";
static const char PARAM_RINGTONE_ESIM_CARD_1[] = "const.multimedia.ringtone_esim_card_1";
static const char PARAM_SYSTEM_TONE_ESIM_CARD_0[] = "const.multimedia.system_tone_esim_card_0";
static const char PARAM_SYSTEM_TONE_ESIM_CARD_1[] = "const.multimedia.system_tone_esim_card_1";
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

// SIM卡Bitmap掩码定义 (用于shot_tone_type和ring_tone_type字段)
constexpr int32_t SIM_CARD_1_MASK = 0x01;    // bit0: 卡1
constexpr int32_t SIM_CARD_2_MASK = 0x02;    // bit1: 卡2
constexpr int32_t ESIM_CARD_1_MASK = 0x04;   // bit2: eSIM1
constexpr int32_t ESIM_CARD_2_MASK = 0x08;   // bit3: eSIM2
constexpr int32_t ALL_CARD_MASK = 0x0F;      // 所有卡

// SIM卡数量定义
constexpr int32_t MAX_SIM_CARD_COUNT = 2;
constexpr int32_t MAX_ESIM_CARD_COUNT = 2;
constexpr int32_t MAX_TOTAL_CARD_COUNT = 4;

// eSIM铃声类型常量 (用于ToneFiles表shot_tone_type/ring_tone_type)
constexpr int32_t SHOT_TONE_TYPE_ESIM_CARD_1 = ESIM_CARD_1_MASK;  // 4
constexpr int32_t SHOT_TONE_TYPE_ESIM_CARD_2 = ESIM_CARD_2_MASK;  // 8
constexpr int32_t RING_TONE_TYPE_ESIM_CARD_1 = ESIM_CARD_1_MASK;  // 4
constexpr int32_t RING_TONE_TYPE_ESIM_CARD_2 = ESIM_CARD_2_MASK;  // 8

inline bool IsSimCard1Set(int32_t toneType)
{
    return (toneType & SIM_CARD_1_MASK) != 0;
}

inline bool IsSimCard2Set(int32_t toneType)
{
    return (toneType & SIM_CARD_2_MASK) != 0;
}

inline bool IsESimCard1Set(int32_t toneType)
{
    return (toneType & ESIM_CARD_1_MASK) != 0;
}

inline bool IsESimCard2Set(int32_t toneType)
{
    return (toneType & ESIM_CARD_2_MASK) != 0;
}

inline int32_t GetSimCardCount(int32_t toneType)
{
    int32_t count = 0;
    if (IsSimCard1Set(toneType)) count++;
    if (IsSimCard2Set(toneType)) count++;
    if (IsESimCard1Set(toneType)) count++;
    if (IsESimCard2Set(toneType)) count++;
    return count;
}

inline bool HasAnyCardSet(int32_t toneType)
{
    return (toneType & ALL_CARD_MASK) != 0;
}

inline int32_t ClearCardMask(int32_t toneType, int32_t cardMask)
{
    return toneType & ~cardMask;
}

inline int32_t SetCardMask(int32_t toneType, int32_t cardMask)
{
    return toneType | cardMask;
}

inline int32_t GetCardMaskFromSimcardMode(SimcardMode mode)
{
    switch (mode) {
        case SIMCARD_MODE_1: return SIM_CARD_1_MASK;
        case SIMCARD_MODE_2: return SIM_CARD_2_MASK;
        case SIMCARD_MODE_ESIM_1: return ESIM_CARD_1_MASK;
        case SIMCARD_MODE_ESIM_2: return ESIM_CARD_2_MASK;
        default: return 0;
    }
}

inline SimcardMode GetSimcardModeFromCardMask(int32_t cardMask)
{
    if (cardMask & SIM_CARD_1_MASK) return SIMCARD_MODE_1;
    if (cardMask & SIM_CARD_2_MASK) return SIMCARD_MODE_2;
    if (cardMask & ESIM_CARD_1_MASK) return SIMCARD_MODE_ESIM_1;
    if (cardMask & ESIM_CARD_2_MASK) return SIMCARD_MODE_ESIM_2;
    return SIMCARD_MODE_INVALID;
}
} // namespace Media
} // namespace OHOS

#endif  // RINGTONE_TYPE_H
