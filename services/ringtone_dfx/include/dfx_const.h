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

#ifndef DFX_CONST_H
#define DFX_CONST_H

#include <string>

namespace OHOS {
namespace Media {
constexpr int32_t ONE_MINUTE = 60;
constexpr int32_t ONE_WEEK_TIME = 168;

enum DfxType {
    RINGTONELIB_DFX_MESSAGE = 0,
};

const std::string ONE_WEEK = "168";

const std::string DFX_COMMON_XML = "/data/storage/el2/base/preferences/dfx_ringtone_common.xml";
const std::string COMMON_XML_EL1 = "/data/storage/el1/base/preferences/ringtone_common.xml";
const std::string LAST_REPORT_TIME = "last_report_time";

const std::string DATE_FORMAT = "%Y/%m/%d";
} // namespace Media
} // namespace OHOS

#endif  // DFX_CONST_H
