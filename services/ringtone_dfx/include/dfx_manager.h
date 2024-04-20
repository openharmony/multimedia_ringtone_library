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

#ifndef OHOS_RINGTONE_DFX_MANAGER_H
#define OHOS_RINGTONE_DFX_MANAGER_H

#include "ability_context.h"
#include "dfx_reporter.h"
#include "ringtone_type.h"

#define EXPORT __attribute__ ((visibility ("default")))

namespace OHOS {
namespace Media {
class DfxManager {
public:
    DfxManager();
    ~DfxManager();
    EXPORT static std::shared_ptr<DfxManager> GetInstance();
    EXPORT int64_t HandleReportXml();
    EXPORT int32_t Init(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context);
    EXPORT int64_t RequestTonesCount(SourceType type);

private:
    static std::mutex instanceLock_;
    static std::shared_ptr<DfxManager> dfxManagerInstance_;
    std::atomic<bool> isInitSuccess_;
    std::shared_ptr<DfxReporter> dfxReporter_;
    std::shared_ptr<OHOS::AbilityRuntime::Context> context_ = nullptr;
};
} // namespace Media
} // namespace OHOS

#endif  // OHOS_RINGTONE_DFX_MANAGER_H
