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

#ifndef RINGTONE_UTILS_H
#define RINGTONE_UTILS_H

#include <string>
#include <map>

namespace OHOS {
namespace Media {
class RingtoneUtils {
public:
    static std::string ReplaceAll(std::string str, const std::string &oldValue, const std::string &newValue);
    static std::map<int, std::string> GetDefaultSystemtoneInfo();
    static int32_t ChecMoveDb();
    static int32_t CheckNeedScanner(const std::string &xmlFilePath);
    static bool MoveEL2DBToEL1DB();
    static bool SetMoveEL2DBToEL1();
    static bool IsNumber(const std::string &str);
    static int64_t GetTotalSize();
    static int64_t GetFreeSize();
    static bool CheckRemainSpaceMeetCondition();
};
} // namespace Media
} // namespace OHOS
#endif // RINGTONE_UTILS_H
