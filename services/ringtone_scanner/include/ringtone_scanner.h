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

#ifndef RINGTONE_SCANNER_OBJ_H
#define RINGTONE_SCANNER_OBJ_H

#include <dirent.h>
#include <sys/stat.h>

#include "iringtone_scanner_callback.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_metadata_extractor.h"
#include "ringtone_scanner_db.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
/**
 * Ringtone Scanner class for scanning files and folders in RingtoneLibrary Database
 * and updating the metadata for each Ringtone file
 *
 * @since 1.0
 * @version 1.0
 */
class RingtoneScannerObj {
public:
    enum ScanType {
        FILE,
        DIRECTORY,
        START
    };
    EXPORT RingtoneScannerObj(const std::string &path, const std::shared_ptr<IRingtoneScannerCallback> &callback,
        RingtoneScannerObj::ScanType type);
    EXPORT RingtoneScannerObj(RingtoneScannerObj::ScanType type);
    EXPORT virtual ~RingtoneScannerObj() = default;

    EXPORT void Scan();

    /* stop */
    EXPORT void SetStopFlag(std::shared_ptr<bool> &stopFlag);

    /* wait for scanning finished*/
    EXPORT void WaitFor();

    /* set is Force Scan */
    EXPORT void SetForceScan(bool isForceScan)
    {
        isForceScan_ = isForceScan;
    }
private:
    /* boot scan */
    EXPORT int32_t BootScan();

    /* file */
    EXPORT int32_t ScanFile();
    EXPORT int32_t ScanFileInternal();
    EXPORT int32_t ScanVibrateFile();
    EXPORT int32_t BuildFileInfo();
    EXPORT int32_t BuildData(const struct stat &statInfo);
    EXPORT int32_t BuildVibrateData(const struct stat &statInfo);
    EXPORT int32_t GetFileMetadata();
    EXPORT int32_t GetMediaInfo();

    /* dir */
    EXPORT int32_t ScanDir();
    EXPORT int32_t ScanDirInternal();
    EXPORT int32_t ScanFileInTraversal(const std::string &path);
    EXPORT int32_t WalkFileTree(const std::string &path);
    EXPORT int32_t CleanupDirectory();

    /* db ops */
    EXPORT int32_t Commit();
    EXPORT int32_t AddToTransaction();
    EXPORT int32_t CommitTransaction();
    EXPORT int32_t CommitVibrateTransaction();
    EXPORT int32_t UpdateToneTypeSettings();

    /* callback */
    EXPORT int32_t InvokeCallback(int32_t code);

    ScanType type_;
    std::string path_;
    std::string dir_;
    std::string uri_;
    std::shared_ptr<IRingtoneScannerCallback> callback_;
    std::shared_ptr<bool> stopFlag_;

    std::unique_ptr<RingtoneMetadata> data_;
    std::vector<std::unique_ptr<RingtoneMetadata>> dataBuffer_;
    std::unique_ptr<VibrateMetadata> vibrateData_;
    std::vector<std::unique_ptr<VibrateMetadata>> vibrateDataBuffer_;
    bool isVibrateFile_ = false;
    bool isForceScan_ = false;
    uint32_t tonesScannedCount_ = 0;
    std::mutex scannerLock_;
    std::condition_variable scannerCv_;
};

class ScanErrCallback : public IRingtoneScannerCallback {
public:
    ScanErrCallback(const std::string &err) : err_(err) {};
    ~ScanErrCallback() = default;

    int32_t OnScanFinished(const int32_t status, const std::string &uri, const std::string &path) override
    {
        return E_OK;
    }

private:
    std::string err_;
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_SCANNER_OBJ_H
