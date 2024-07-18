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

// ringtone user customized tones file location
const std::string RINGTONE_CUSTOMIZED_BASE_PATH          = "/storage/media/local/files";
const std::string RINGTONE_CUSTOMIZED_ALARM_PATH         = RINGTONE_CUSTOMIZED_BASE_PATH + "/Ringtone/alarms";
const std::string RINGTONE_CUSTOMIZED_RINGTONE_PATH      = RINGTONE_CUSTOMIZED_BASE_PATH + "/Ringtone/ringtones";
const std::string RINGTONE_CUSTOMIZED_NOTIFICATIONS_PATH = RINGTONE_CUSTOMIZED_BASE_PATH + "/Ringtone/notifications";

// ringtone preload pathes
const std::string ROOT_TONE_PRELOAD_PATH_NOAH_PATH = "/sys_prod/resource/media/audio";
const std::string ROOT_TONE_PRELOAD_PATH_CHINA_PATH = "/sys_prod/variant/region_comm/china/resource/media/audio";
const std::string ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH = "/sys_prod/variant/region_comm/oversea/resource/media/audio";

// ringtone DB file path
const std::string RINGTONE_LIBRARY_DB_PATH = "/data/storage/el2/database";

// ringtone DB file name
const std::string RINGTONE_LIBRARY_DB_NAME = "ringtone_library.db";

// ringtone db version
const int32_t RINGTONE_RDB_VERSION = 1;

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
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_DB_CONST_H
