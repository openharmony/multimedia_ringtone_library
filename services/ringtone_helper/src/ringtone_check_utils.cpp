/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define MLOG_TAG "RingtoneCheckUtils"

#include "ringtone_check_utils.h"

#include <string>

#include "ringtone_errno.h"
#include "ringtone_db_const.h"

namespace OHOS {
namespace Media {
using namespace std;

const std::string RINGTONE_CUSTOM_PATH_FIRST = "/storage/media/local/files";
const std::string RINGTONE_CUSTOM_PATH_SECOND = "/data/storage/el2/base/files";

std::string RingtoneCheckUtils::GetCustomRingtoneCurrentPath(const std::string &uri)
{
    std::string uriCopy = uri;
    if (uriCopy.compare(0, RINGTONE_CUSTOM_PATH_FIRST.size(), RINGTONE_CUSTOM_PATH_FIRST) == 0) {
        uriCopy.replace(0, RINGTONE_CUSTOM_PATH_FIRST.size(), RINGTONE_CUSTOMIZED_BASE_PATH);
        return uriCopy;
    }
    if (uriCopy.compare(0, RINGTONE_CUSTOM_PATH_SECOND.size(), RINGTONE_CUSTOM_PATH_SECOND) == 0) {
        uriCopy.replace(0, RINGTONE_CUSTOM_PATH_SECOND.size(), RINGTONE_CUSTOMIZED_BASE_PATH);
        return uriCopy;
    }
    return uriCopy;
}
} // namespace Media
} // namespace OHOS
