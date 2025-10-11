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

#ifndef RINGTONE_DB_CONST_H
#define RINGTONE_DB_CONST_H

#include <string>

namespace OHOS {
namespace Media {

const std::string RINGTONE_BUNDLE_NAME = "com.ohos.ringtonelibrary.ringtonelibrarydata";

const std::string RINGTONE_URI_SCHEME       = "datashare";
const std::string RINGTONE_URI_AUTHORITY    = "ringtone";
const std::string RINGTONE_URI_PATH         = "ringtone";
const std::string RINGTONE_URI              = RINGTONE_URI_SCHEME + ":///" + RINGTONE_URI_AUTHORITY;
const std::string RINGTONE_PATH_URI         = RINGTONE_URI + "/" + RINGTONE_URI_PATH;

const std::string VIBRATE_URI_PATH          = "vibrate";
const std::string VIBRATE_PATH_URI          = RINGTONE_URI + "/" + VIBRATE_URI_PATH;

const std::string RINGTONE_URI_PROXY_STRING = "?Proxy";

// ringtone user customized tones file location
const std::string RINGTONE_CUSTOMIZED_BASE_PATH          = "/data/storage/el2/base/files";
const std::string RINGTONE_CUSTOMIZED_ALARM_PATH         = RINGTONE_CUSTOMIZED_BASE_PATH + "/Ringtone/alarms";
const std::string RINGTONE_CUSTOMIZED_RINGTONE_PATH      = RINGTONE_CUSTOMIZED_BASE_PATH + "/Ringtone/ringtones";
const std::string RINGTONE_CUSTOMIZED_NOTIFICATIONS_PATH = RINGTONE_CUSTOMIZED_BASE_PATH + "/Ringtone/notifications";
const std::string RINGTONE_CUSTOMIZED_CONTACTS_PATH      = RINGTONE_CUSTOMIZED_BASE_PATH + "/Ringtone/contacts";
const std::string RINGTONE_CUSTOMIZED_APP_NOTIFICATIONS_PATH = RINGTONE_CUSTOMIZED_BASE_PATH +
    "/Ringtone/app_notifications";

const std::string RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH = "/data/storage/el2/base/files/Ringtone";
const std::string RINGTONE_CUSTOMIZED_BASE_RINGTONETMP_PATH = "/data/storage/el2/base/files/RingtoneTmp";

// ringtone preload pathes
const std::string ROOT_TONE_PRELOAD_PATH_NOAH_PATH = "/sys_prod/resource/media/audio";
const std::string ROOT_TONE_PRELOAD_PATH_CHINA_PATH = "/sys_prod/variant/region_comm/china/resource/media/audio";
const std::string ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH = "/sys_prod/variant/region_comm/oversea/resource/media/audio";

// vibrate preload pathes
const std::string ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH = "/sys_prod/resource/media/haptics";
const std::string ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH = "/sys_prod/variant/region_comm/china/resource/media/haptics";
const std::string ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH =
    "/sys_prod/variant/region_comm/oversea/resource/media/haptics";

// ringtone DB file path
const std::string RINGTONE_LIBRARY_DB_PATH = "/data/storage/el2/database";
const std::string RINGTONE_LIBRARY_DB_PATH_EL1 = "/data/storage/el1/database";

// ringtone DB file name
const std::string RINGTONE_LIBRARY_DB_NAME = "ringtone_library.db";

// ringtone db version
const int32_t RINGTONE_RDB_VERSION = 9;
enum {
    VERSION_ADD_DISPLAY_LANGUAGE_COLUMN = 2,
    VERSION_ADD_VIBRATE_TABLE = 3,
    VERSION_UPDATE_MIME_TYPE = 4,
    VERSION_ADD_PRELOAD_CONF_TABLE = 5,
    VERSION_UPDATE_WATCH_MIME_TYPE = 6,
    VERSION_ADD_SCANNER_FLAG = 7,
    VERSION_UPDATE_MEDIA_TYPE_VIDEO = 8,
    VERSION_UPDATE_DATA_URI = 9,
};

// ringtone table name
const std::string RINGTONE_TABLE                                = "ToneFiles";
// ringtone table column name
const std::string RINGTONE_COLUMN_TONE_ID                       = "tone_id";
const std::string RINGTONE_COLUMN_DATA                          = "data";
const std::string RINGTONE_COLUMN_SIZE                          = "size";
const std::string RINGTONE_COLUMN_DISPLAY_NAME                  = "display_name";
const std::string RINGTONE_COLUMN_TITLE                         = "title";
const std::string RINGTONE_COLUMN_MEDIA_TYPE                    = "media_type";
const std::string RINGTONE_COLUMN_TONE_TYPE                     = "tone_type";
const std::string RINGTONE_COLUMN_MIME_TYPE                     = "mime_type";
const std::string RINGTONE_COLUMN_SOURCE_TYPE                   = "source_type";
const std::string RINGTONE_COLUMN_DATE_ADDED                    = "date_added";
const std::string RINGTONE_COLUMN_DATE_MODIFIED                 = "date_modified";
const std::string RINGTONE_COLUMN_DATE_TAKEN                    = "date_taken";
const std::string RINGTONE_COLUMN_DURATION                      = "duration";
const std::string RINGTONE_COLUMN_SHOT_TONE_TYPE                = "shot_tone_type";
const std::string RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE         = "shot_tone_source_type";
const std::string RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE        = "notification_tone_type";
const std::string RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE = "notification_tone_source_type";
const std::string RINGTONE_COLUMN_RING_TONE_TYPE                = "ring_tone_type";
const std::string RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE         = "ring_tone_source_type";
const std::string RINGTONE_COLUMN_ALARM_TONE_TYPE               = "alarm_tone_type";
const std::string RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE        = "alarm_tone_source_type";
const std::string RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE         = "display_language_type";
const std::string RINGTONE_COLUMN_SCANNER_FLAG                  = "scanner_flag";

// simcard setting table name
const std::string SIMCARD_SETTING_TABLE                         = "SimCardSetting";
const std::string SIMCARD_SETTING_URI_PATH                      = SIMCARD_SETTING_TABLE;
const std::string SIMCARD_SETTING_PATH_URI                      = RINGTONE_URI + "/" + SIMCARD_SETTING_URI_PATH;
// simcard setting table column name
const std::string SIMCARD_SETTING_COLUMN_MODE                   = "mode";
const std::string SIMCARD_SETTING_COLUMN_TONE_FILE              = "tone_file";
const std::string SIMCARD_SETTING_COLUMN_RINGTONE_TYPE          = "ringtone_type";
const std::string SIMCARD_SETTING_COLUMN_VIBRATE_FILE           = "vibrate_file";
const std::string SIMCARD_SETTING_COLUMN_VIBRATE_MODE           = "vibrate_mode";
const std::string SIMCARD_SETTING_COLUMN_RING_MODE              = "ring_mode";

// vibrate table name
const std::string VIBRATE_TABLE                                 = "VibrateFiles";
// vibrate table column name
const std::string VIBRATE_COLUMN_VIBRATE_ID                     = "vibrate_id";
const std::string VIBRATE_COLUMN_DATA                           = "data";
const std::string VIBRATE_COLUMN_SIZE                           = "size";
const std::string VIBRATE_COLUMN_DISPLAY_NAME                   = "display_name";
const std::string VIBRATE_COLUMN_TITLE                          = "title";
const std::string VIBRATE_COLUMN_DISPLAY_LANGUAGE               = "display_language";
const std::string VIBRATE_COLUMN_VIBRATE_TYPE                   = "vibrate_type";
const std::string VIBRATE_COLUMN_SOURCE_TYPE                    = "source_type";
const std::string VIBRATE_COLUMN_DATE_ADDED                     = "date_added";
const std::string VIBRATE_COLUMN_DATE_MODIFIED                  = "date_modified";
const std::string VIBRATE_COLUMN_DATE_TAKEN                     = "date_taken";
const std::string VIBRATE_COLUMN_PLAY_MODE                      = "play_mode";
const std::string VIBRATE_COLUMN_SCANNER_FLAG                   = "scanner_flag";

// preload config table name
const std::string PRELOAD_CONFIG_TABLE                          = "PreloadConfig";

// preload config column name
const std::string PRELOAD_CONFIG_COLUMN_RING_TONE_TYPE          = "ring_tone_type";
const std::string PRELOAD_CONFIG_COLUMN_TONE_ID                 = "tone_id";
const std::string PRELOAD_CONFIG_COLUMN_DISPLAY_NAME            = "display_name";

} // namespace Media
} // namespace OHOS

#endif // RINGTONE_DB_CONST_H
