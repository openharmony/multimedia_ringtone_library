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

#include "ringtone_utils.h"

#include <securec.h>

#include "parameter.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
using namespace std;
static const int32_t SYSPARA_SIZE = 128;

std::string RingtoneUtils::ReplaceAll(std::string str, const std::string &oldValue, const std::string &newValue)
{
    for (std::string::size_type pos(0); pos != std::string::npos; pos += newValue.length()) {
        if ((pos = str.find(oldValue, pos)) != std::string::npos) {
            str.replace(pos, oldValue.length(), newValue);
        } else {
            break;
        }
    }
    return str;
}

std::map<int, std::string> RingtoneUtils::GetDefaultSystemtoneInfo()
{
    map<int, string> defaultSystemtoneInfo;
    char paramValue[SYSPARA_SIZE] = {0};
    GetParameter(PARAM_RINGTONE_SETTING_RINGTONE, "", paramValue, SYSPARA_SIZE);
    if (strcmp(paramValue, "")) {
        defaultSystemtoneInfo.insert(make_pair(DEFAULT_RING_TYPE_SIM_CARD_1, string(paramValue)));
    }

    memset_s(paramValue, sizeof(paramValue), 0, sizeof(paramValue));
    GetParameter(PARAM_RINGTONE_SETTING_RINGTONE2, "", paramValue, SYSPARA_SIZE);
    if (strcmp(paramValue, "")) {
        defaultSystemtoneInfo.insert(make_pair(DEFAULT_RING_TYPE_SIM_CARD_2, string(paramValue)));
    }

    memset_s(paramValue, sizeof(paramValue), 0, sizeof(paramValue));
    GetParameter(PARAM_RINGTONE_SETTING_SHOT, "", paramValue, SYSPARA_SIZE);
    if (strcmp(paramValue, "")) {
        defaultSystemtoneInfo.insert(make_pair(DEFAULT_SHOT_TYPE_SIM_CARD_1, string(paramValue)));
    }

    memset_s(paramValue, sizeof(paramValue), 0, sizeof(paramValue));
    GetParameter(PARAM_RINGTONE_SETTING_SHOT2, "", paramValue, SYSPARA_SIZE);
    if (strcmp(paramValue, "")) {
        defaultSystemtoneInfo.insert(make_pair(DEFAULT_SHOT_TYPE_SIM_CARD_2, string(paramValue)));
    }

    memset_s(paramValue, sizeof(paramValue), 0, sizeof(paramValue));
    GetParameter(PARAM_RINGTONE_SETTING_NOTIFICATIONTONE, "", paramValue, SYSPARA_SIZE);
    if (strcmp(paramValue, "")) {
        defaultSystemtoneInfo.insert(make_pair(DEFAULT_NOTIFICATION_TYPE, string(paramValue)));
    }

    memset_s(paramValue, sizeof(paramValue), 0, sizeof(paramValue));
    GetParameter(PARAM_RINGTONE_SETTING_ALARM, "", paramValue, SYSPARA_SIZE);
    if (strcmp(paramValue, "")) {
        defaultSystemtoneInfo.insert(make_pair(DEFAULT_ALARM_TYPE, string(paramValue)));
    }
    return defaultSystemtoneInfo;
}
} // namespace Media
} // namespace OHOS
