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

#ifndef VIBRATE_TYPE_H
#define VIBRATE_TYPE_H

namespace OHOS {
namespace Media {
// vibrate system default setting
enum VibrateType : int32_t {
    VIBRATE_TYPE_INVALID = -1,
    VIBRATE_TYPE_STANDARD = 1,
    VIBRATE_TYPE_GENTLE,
    VIBRATE_TYPE_SALARM,
    VIBRATE_TYPE_SRINGTONE,
    VIBRATE_TYPE_SNOTIFICATION,
    VIBRATE_TYPE_GALARM,
    VIBRATE_TYPE_GRINGTONE,
    VIBRATE_TYPE_GNOTIFICATION,
    VIBRATE_TYPE_MAX,
};

enum VibratePlayMode : int32_t {
    VIBRATE_PLAYMODE_INVALID = -1,
    VIBRATE_PLAYMODE_NONE,
    VIBRATE_PLAYMODE_SYNC,
    VIBRATE_PLAYMODE_CLASSIC,
};

// default value
const int32_t VIBRATE_ID_DEFAULT = -1;
const std::string DISPLAY_LANGUAGE_DEFAULT = "";
const int32_t VIBRATE_TYPE_DEFAULT = -1;

/** Supported vibrate types */
const std::string VIBRATE_CONTAINER_TYPE_JSON = "json";
} // namespace Media
} // namespace OHOS

#endif  // VIBRATE_TYPE_H
