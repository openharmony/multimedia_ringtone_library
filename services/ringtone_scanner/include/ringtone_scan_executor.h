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

#ifndef RINGTONE_SCAN_EXECUTOR_H
#define RINGTONE_SCAN_EXECUTOR_H

#include <queue>

#include "ringtone_scanner.h"

namespace OHOS {
namespace Media {
class RingtoneScanExecutor {
public:
    RingtoneScanExecutor() = default;
    virtual ~RingtoneScanExecutor() = default;

    int32_t Commit(std::shared_ptr<RingtoneScannerObj> scanner);

    void Start();
    void Stop();

private:
    void HandleScanExecution();

    size_t activeThread_ = 0;

    std::queue<std::shared_ptr<RingtoneScannerObj>> queue_;
    std::mutex queueMtx_;

    std::shared_ptr<bool> stopFlag_ = std::make_shared<bool>(false);
    int32_t sleepTime_ = 200;
};
} // namespace Media
} // namespace OHOS

#endif /* RINGTONE_SCAN_EXECUTOR_H */
