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

#define MLOG_TAG "RingtoneXCollie"

#include "ringtone_xcollie.h"

#include <unistd.h>

#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace Media {

RingtoneXCollie::RingtoneXCollie(const std::string &tag, std::function<void(void *)> func,
    uint32_t timeoutSeconds, void *arg, uint32_t flag)
{
    tag_ = tag;
    id_ = HiviewDFX::XCollie::GetInstance().SetTimer(tag, timeoutSeconds, func, arg, flag);
    if (id_ == HiviewDFX::INVALID_ID) {
        RINGTONE_ERR_LOG("Set XCollie Timer failed");
        return;
    }
    RINGTONE_INFO_LOG("Set XCollie OK");
}

RingtoneXCollie::~RingtoneXCollie()
{
    CancelXCollieTimer();
}

void RingtoneXCollie::CancelXCollieTimer()
{
    if (!isCanceled_ && id_ != HiviewDFX::INVALID_ID) {
        HiviewDFX::XCollie::GetInstance().CancelTimer(id_);
        isCanceled_ = true;
        RINGTONE_INFO_LOG("Cancel XCollie OK");
    }
}

} // namespace Media
} // namespace OHOS