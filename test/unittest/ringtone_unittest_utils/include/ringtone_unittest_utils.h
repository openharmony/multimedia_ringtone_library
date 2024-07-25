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

#ifndef RINGTONE_UNITTEST_UTILS_H
#define RINGTONE_UNITTEST_UTILS_H

#include <condition_variable>

#include "iringtone_scanner_callback.h"
#include "ringtone_asset.h"

namespace OHOS {
namespace Media {
const std::string STORAGE_FILES_DIR = "/storage/cloud/files/";

class TestRingtoneScannerCallback : public IRingtoneScannerCallback {
public:
    TestRingtoneScannerCallback();
    ~TestRingtoneScannerCallback() = default;

    int32_t status_;
    std::condition_variable condVar_;
    int32_t OnScanFinished(const int32_t status, const std::string &uri, const std::string &path) override;
};

class RingtoneUnitTestUtils {
public:
    RingtoneUnitTestUtils() {}
    virtual ~RingtoneUnitTestUtils() {}
    static void Init();
    static void InitRootDirs();
    static void CleanTestFiles();
    static bool CreateFileFS(const std::string& filePath);
    static void WaitForCallback(std::shared_ptr<TestRingtoneScannerCallback> callback);
private:
    static inline bool isValid_ = false;
    static inline std::unordered_map<std::string, std::shared_ptr<RingtoneAsset>> rootDirAssetMap_;
};
}
}

#endif // RINGTONE_UNITTEST_UTILS_H
