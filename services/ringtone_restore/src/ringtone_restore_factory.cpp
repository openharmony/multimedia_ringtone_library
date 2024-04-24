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

#define MLOG_TAG "RingtoneRestoreFactory"

#include <memory>

#include "ringtone_restore_factory.h"

#include "restore_interface.h"
#include "ringtone_dualfw_restore.h"
#include "ringtone_log.h"
#include "ringtone_restore.h"

namespace OHOS {
namespace Media {
std::unique_ptr<RestoreInterface> RingtoneRestoreFactory::CreateObj(RestoreSceneType type)
{
    std::unique_ptr<RestoreInterface> obj = nullptr;

    if (type == RESTORE_SCENE_TYPE_SINGLE_CLONE) {
        obj = std::make_unique<RingtoneRestore>();
    } else if (RESTORE_SCENE_TYPE_DUAL_CLONE || RESTORE_SCENE_TYPE_DUAL_UPGRADE) {
        // reserve for dual-fw restore
        obj = std::make_unique<RingtoneDualfwRestore>();
    } else {
        RINGTONE_ERR_LOG("error: invalid argument");
    }

    return obj;
}
} // namespace Media
} // namespace OHOS
