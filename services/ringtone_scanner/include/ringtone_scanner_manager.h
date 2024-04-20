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

#ifndef RINGTONE_SCANNER_MANAGER_H
#define RINGTONE_SCANNER_MANAGER_H

#include "ringtone_scan_executor.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class RingtoneScannerManager final {
public:
    EXPORT static std::shared_ptr<RingtoneScannerManager> GetInstance();

    RingtoneScannerManager() = default;
    EXPORT virtual ~RingtoneScannerManager() = default;

    void Start(bool isSync = true);
    void Stop();

    EXPORT int32_t ScanFile(const std::string &path, const std::shared_ptr<IRingtoneScannerCallback> &callback);
    int32_t ScanFileSync(const std::string &path, const std::shared_ptr<IRingtoneScannerCallback> &callback,
        bool isForceScan = false);
    EXPORT int32_t ScanDir(const std::string &path, const std::shared_ptr<IRingtoneScannerCallback> &callback);
    EXPORT int32_t ScanDirSync(const std::string &path, const std::shared_ptr<IRingtoneScannerCallback> &callback);
private:
    static std::shared_ptr<RingtoneScannerManager> instance_;
    static std::mutex instanceMutex_;

    RingtoneScanExecutor executor_;
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_SCANNER_MANAGER_H
