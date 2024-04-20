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

#define MLOG_TAG "Scanner"

#include "ringtone_scanner_manager.h"
#include "directory_ex.h"
#include "ringtone_log.h"

namespace OHOS {
namespace Media {
std::shared_ptr<RingtoneScannerManager> RingtoneScannerManager::instance_ = nullptr;
std::mutex RingtoneScannerManager::instanceMutex_;

std::shared_ptr<RingtoneScannerManager> RingtoneScannerManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> guard(instanceMutex_);

        if (instance_ != nullptr) {
            return instance_;
        }

        instance_ = std::make_shared<RingtoneScannerManager>();
    }

    return instance_;
}

int32_t RingtoneScannerManager::ScanFile(const std::string &path,
    const std::shared_ptr<IRingtoneScannerCallback> &callback)
{
    RINGTONE_DEBUG_LOG("scan file %{private}s", path.c_str());

    std::string realPath;
    if (!PathToRealPath(path, realPath)) {
        RINGTONE_ERR_LOG("failed to get real path %{private}s, errno %{public}d", path.c_str(), errno);
        return E_INVALID_PATH;
    }

    if (!RingtoneScannerUtils::IsRegularFile(realPath)) {
        RINGTONE_ERR_LOG("the path %{private}s is not a regular file", realPath.c_str());
        return E_INVALID_PATH;
    }

    std::unique_ptr<RingtoneScannerObj> scanner =
        std::make_unique<RingtoneScannerObj>(realPath, callback, RingtoneScannerObj::FILE);
    executor_.Commit(move(scanner));

    return E_OK;
}

int32_t RingtoneScannerManager::ScanFileSync(const std::string &path,
    const std::shared_ptr<IRingtoneScannerCallback> &callback, bool isForceScan)
{
    RINGTONE_DEBUG_LOG("scan file %{private}s", path.c_str());

    std::string realPath;
    if (!PathToRealPath(path, realPath)) {
        RINGTONE_ERR_LOG("failed to get real path %{private}s, errno %{public}d", path.c_str(), errno);
        return E_INVALID_PATH;
    }

    if (!RingtoneScannerUtils::IsRegularFile(realPath)) {
        RINGTONE_ERR_LOG("the path %{private}s is not a regular file", realPath.c_str());
        return E_INVALID_PATH;
    }

    RingtoneScannerObj scanner = RingtoneScannerObj(realPath, callback, RingtoneScannerObj::FILE);
    if (isForceScan) {
        scanner.SetForceScan(true);
    }
    scanner.Scan();

    return E_OK;
}

int32_t RingtoneScannerManager::ScanDir(const std::string &path,
    const std::shared_ptr<IRingtoneScannerCallback> &callback)
{
    RINGTONE_DEBUG_LOG("scan dir %{private}s", path.c_str());

    std::string realPath;
    if (!PathToRealPath(path, realPath)) {
        RINGTONE_ERR_LOG("failed to get real path %{private}s, errno %{public}d", path.c_str(), errno);
        return E_INVALID_PATH;
    }

    if (!RingtoneScannerUtils::IsDirectory(realPath)) {
        RINGTONE_ERR_LOG("the path %{private}s is not a directory", realPath.c_str());
        return E_INVALID_PATH;
    }

    std::unique_ptr<RingtoneScannerObj> scanner = std::make_unique<RingtoneScannerObj>(realPath, callback,
        RingtoneScannerObj::DIRECTORY);
    executor_.Commit(move(scanner));

    return E_OK;
}

int32_t RingtoneScannerManager::ScanDirSync(const std::string &path,
    const std::shared_ptr<IRingtoneScannerCallback> &callback)
{
    RINGTONE_DEBUG_LOG("scan dir %{private}s", path.c_str());

    std::string realPath;
    if (!PathToRealPath(path, realPath)) {
        RINGTONE_ERR_LOG("failed to get real path %{private}s, errno %{public}d", path.c_str(), errno);
        return E_INVALID_PATH;
    }

    if (!RingtoneScannerUtils::IsDirectory(realPath)) {
        RINGTONE_ERR_LOG("the path %{private}s is not a directory", realPath.c_str());
        return E_INVALID_PATH;
    }

    RingtoneScannerObj scanner = RingtoneScannerObj(realPath, callback, RingtoneScannerObj::DIRECTORY);
    scanner.Scan();

    return E_OK;
}

void RingtoneScannerManager::Start(bool isSync)
{
    if (isSync) {
        RINGTONE_DEBUG_LOG("scan start, isSync = %{public}d", isSync);
        RingtoneScannerObj scanner = RingtoneScannerObj(RingtoneScannerObj::START);
        scanner.Scan();
        RINGTONE_DEBUG_LOG("scan finished");
    } else {
        RINGTONE_DEBUG_LOG("scan start, isSync = %{public}d", isSync);
        executor_.Start();
        std::shared_ptr<RingtoneScannerObj> scanner = std::make_shared<RingtoneScannerObj>(RingtoneScannerObj::START);
        executor_.Commit(scanner);
        scanner->WaitFor();
        RINGTONE_DEBUG_LOG("scan finished");
    }
}

void RingtoneScannerManager::Stop()
{
    /* stop all working threads */
    this->executor_.Stop();
}
} // namespace Media
} // namespace OHOS
