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

#include "ringtone_scanner.h"

#include "directory_ex.h"
#include "ringtone_default_setting.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_mimetype_utils.h"
#include "ringtone_rdbstore.h"

namespace OHOS {
namespace Media {
using namespace std;
using namespace OHOS::AppExecFwk;
using namespace OHOS::DataShare;
static const int32_t SCANNER_WAIT_FOR_TIMEOUT = 10000; // ms
#ifndef OHOS_LOCAL_DEBUG_DISABLE
// just for debug
static const std::string LOCAL_DIR = "/storage/media/local/data/com.ohos.ringtonelibrary.ringtonelibrarydata";
#endif
static std::unordered_map<std::string, std::pair<int32_t, int32_t>> g_typeMap = {
#ifndef OHOS_LOCAL_DEBUG_DISABLE
    // just for debug
    {LOCAL_DIR + "/alarms", {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_ALARM}},
    {LOCAL_DIR + "/ringtones", {SOURCE_TYPE_PRESET, TONE_TYPE_RINGTONE}},
    {LOCAL_DIR + "/notifications", {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_NOTIFICATION}},
#endif
    // customized tones map
    {RINGTONE_CUSTOMIZED_ALARM_PATH, {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_ALARM}},
    {RINGTONE_CUSTOMIZED_RINGTONE_PATH, {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_RINGTONE}},
    {RINGTONE_CUSTOMIZED_NOTIFICATIONS_PATH, {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_NOTIFICATION}},
    // customized tones map
    {ROOT_TONE_PRELOAD_PATH_NOAH_PATH + "/alarms", {SOURCE_TYPE_PRESET, TONE_TYPE_ALARM}},
    {ROOT_TONE_PRELOAD_PATH_NOAH_PATH + "/ringtones", {SOURCE_TYPE_PRESET, TONE_TYPE_RINGTONE}},
    {ROOT_TONE_PRELOAD_PATH_NOAH_PATH + "/notifications", {SOURCE_TYPE_PRESET, TONE_TYPE_NOTIFICATION}},
    {ROOT_TONE_PRELOAD_PATH_CHINA_PATH + "/alarms", {SOURCE_TYPE_PRESET, TONE_TYPE_ALARM}},
    {ROOT_TONE_PRELOAD_PATH_CHINA_PATH + "/ringtones", {SOURCE_TYPE_PRESET, TONE_TYPE_RINGTONE}},
    {ROOT_TONE_PRELOAD_PATH_CHINA_PATH + "/notifications", {SOURCE_TYPE_PRESET, TONE_TYPE_NOTIFICATION}},
    {ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "/alarms", {SOURCE_TYPE_PRESET, TONE_TYPE_ALARM}},
    {ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "/ringtones", {SOURCE_TYPE_PRESET, TONE_TYPE_RINGTONE}},
    {ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "/notifications", {SOURCE_TYPE_PRESET, TONE_TYPE_NOTIFICATION}},
};


RingtoneScannerObj::RingtoneScannerObj(const std::string &path,
    const std::shared_ptr<IRingtoneScannerCallback> &callback,
    RingtoneScannerObj::ScanType type) : type_(type), callback_(callback)
{
    if (type_ == DIRECTORY) {
        dir_ = path;
    } else if (type_ == FILE) {
        path_ = path;
    }
    // when path is /Photo, it means update or clone scene
    stopFlag_ = make_shared<bool>(false);
}

RingtoneScannerObj::RingtoneScannerObj(RingtoneScannerObj::ScanType type) : type_(type)
{
    stopFlag_ = make_shared<bool>(false);
}

void RingtoneScannerObj::SetStopFlag(std::shared_ptr<bool> &flag)
{
    stopFlag_ = flag;
}

int32_t RingtoneScannerObj::ScanFile()
{
    RINGTONE_DEBUG_LOG("scan file %{private}s", path_.c_str());

    int32_t ret = ScanFileInternal();
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("ScanFileInternal err %{public}d", ret);
    }

    (void)InvokeCallback(ret);

    return ret;
}

int32_t RingtoneScannerObj::InvokeCallback(int32_t err)
{
    if (callback_ == nullptr) {
        return E_OK;
    }

    return callback_->OnScanFinished(err, uri_, path_);
}

void RingtoneScannerObj::Scan()
{
    switch (type_) {
        case FILE:
            ScanFile();
            break;
        case DIRECTORY:
            ScanDir();
            break;
        case START:
            BootScan();
            break;
        default:
            break;
    }
}

int32_t RingtoneScannerObj::BootScan()
{
    static const std::vector<std::string> preloadDirs = {
#ifndef OHOS_LOCAL_DEBUG_DISABLE
        LOCAL_DIR,
#endif
        ROOT_TONE_PRELOAD_PATH_NOAH_PATH,
        ROOT_TONE_PRELOAD_PATH_CHINA_PATH,
        ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH,
    };

    int64_t scanStart = RingtoneFileUtils::UTCTimeMilliSeconds();
    for (auto &dir : preloadDirs) {
        RINGTONE_INFO_LOG("start to scan realpath %{private}s", dir.c_str());
        string realPath;
        if (!PathToRealPath(dir, realPath)) {
            RINGTONE_ERR_LOG("failed to get realpath %{private}s, errno %{public}d", dir.c_str(), errno);
            continue;
        }

        RINGTONE_INFO_LOG("start to scan realpath %{private}s", dir.c_str());
        callback_ = make_shared<ScanErrCallback>(dir);

        if (RingtoneScannerUtils::IsDirectory(realPath)) {
            dir_ = move(realPath);
            (void)ScanDir();
        } else if (RingtoneScannerUtils::IsRegularFile(realPath)) {
            path_ = move(realPath);
            (void)ScanFile();
        }
    }

    // reset ringtone default settings
    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    RingtoneDefaultSetting::GetObj(rawRdb)->Update();

    int64_t scanEnd = RingtoneFileUtils::UTCTimeMilliSeconds();
    RINGTONE_INFO_LOG("total preload tone files count:%{public}d, scanned: %{public}d, costed-time:%{public}lld ms",
        tonesScannedCount_, tonesScannedCount_, scanEnd - scanStart);
    unique_lock<mutex> lock(scannerLock_);
    scannerCv_.notify_one();
    return E_OK;
}

void RingtoneScannerObj::WaitFor()
{
    unique_lock<mutex> lock(scannerLock_);
    scannerCv_.wait_for(lock, chrono::milliseconds(SCANNER_WAIT_FOR_TIMEOUT));
}

int32_t RingtoneScannerObj::ScanDir()
{
    RINGTONE_INFO_LOG("scan dir %{private}s", dir_.c_str());

    int32_t ret = ScanDirInternal();
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("ScanDirInternal err %{public}d", ret);
    }

    (void)InvokeCallback(ret);

    return ret;
}

int32_t RingtoneScannerObj::ScanDirInternal()
{
    if (RingtoneScannerUtils::IsDirHiddenRecursive(dir_)) {
        RINGTONE_ERR_LOG("the dir %{private}s is hidden", dir_.c_str());
        return E_DIR_HIDDEN;
    }

    /* no further operation when stopped */
    auto err = WalkFileTree(dir_);
    if (err != E_OK) {
        RINGTONE_ERR_LOG("walk file tree err %{public}d", err);
        return err;
    }
    err = CommitTransaction();
    if (err != E_OK) {
        RINGTONE_ERR_LOG("commit transaction err %{public}d", err);
        return err;
    }
    err = CleanupDirectory();
    if (err != E_OK) {
        RINGTONE_ERR_LOG("clean up dir err %{public}d", err);
        return err;
    }

    return E_OK;
}

int32_t RingtoneScannerObj::CleanupDirectory()
{
    return E_OK;
}

int32_t RingtoneScannerObj::CommitTransaction()
{
    unordered_set<MediaType> mediaTypeSet = {};
    unique_ptr<RingtoneMetadata> data;
    string tableName = RINGTONE_TABLE;

    // will begin a transaction in later pr
    for (uint32_t i = 0; i < dataBuffer_.size(); i++) {
        data = move(dataBuffer_[i]);
        if (data->GetToneId() != FILE_ID_DEFAULT) {
            RingtoneScannerDb::UpdateMetadata(*data, tableName);
        } else {
            RingtoneScannerDb::InsertMetadata(*data, tableName);
        }
    }
    if (dataBuffer_.size() > 0) {
        tonesScannedCount_ += dataBuffer_.size();
    }
    dataBuffer_.clear();

    return E_OK;
}

int32_t RingtoneScannerObj::WalkFileTree(const string &path)
{
    int err = E_OK;
    DIR *dirPath = nullptr;
    struct dirent *ent = nullptr;
    size_t len = path.length();
    struct stat statInfo;
    if (len >= FILENAME_MAX - 1) {
        return ERR_INCORRECT_PATH;
    }
    auto fName = (char *)calloc(FILENAME_MAX, sizeof(char));
    if (fName == nullptr) {
        return E_NO_MEMORY;
    }
    if (strcpy_s(fName, FILENAME_MAX, path.c_str()) != ERR_SUCCESS) {
        free(fName);
        return E_ERR;
    }
    fName[len++] = '/';
    if ((dirPath = opendir(path.c_str())) == nullptr) {
        free(fName);
        return E_PERMISSION_DENIED;
    }
    while ((ent = readdir(dirPath)) != nullptr) {
        if (*stopFlag_) {
            err = E_STOP;
            break;
        }
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
            continue;
        }
        if (strncpy_s(fName + len, FILENAME_MAX - len, ent->d_name, FILENAME_MAX - len)) {
            continue;
        }
        if (lstat(fName, &statInfo) == -1) {
            continue;
        }
        string currentPath = fName;
        if (S_ISDIR(statInfo.st_mode)) {
            if (RingtoneScannerUtils::IsDirHidden(currentPath)) {
                continue;
            }
            (void)WalkFileTree(currentPath);
        } else {
            (void)ScanFileInTraversal(currentPath);
        }
    }
    closedir(dirPath);
    free(fName);
    return err;
}

int32_t RingtoneScannerObj::ScanFileInTraversal(const string &path)
{
    path_ = path;
    if (RingtoneScannerUtils::IsFileHidden(path_)) {
        RINGTONE_ERR_LOG("the file is hidden");
        return E_FILE_HIDDEN;
    }
    int32_t err = GetFileMetadata();
    if (err != E_OK) {
        if (err != E_SCANNED) {
            RINGTONE_ERR_LOG("failed to get file metadata");
        }
        return err;
    }

    err = BuildFileInfo();
    if (err != E_OK) {
        RINGTONE_ERR_LOG("failed to get other file metadata");
        return err;
    }

    return E_OK;
}

int32_t RingtoneScannerObj::GetFileMetadata()
{
    if (path_.empty()) {
        return E_INVALID_ARGUMENTS;
    }
    struct stat statInfo = { 0 };
    if (stat(path_.c_str(), &statInfo) != 0) {
        RINGTONE_ERR_LOG("stat syscall err %{public}d", errno);
        return E_SYSCALL;
    }
    int errCode = BuildData(statInfo);
    if (errCode != E_OK) {
        return errCode;
    }

    return E_OK;
}

int32_t RingtoneScannerObj::BuildFileInfo()
{
    auto err = GetMediaInfo();
    if (err != E_OK) {
        RINGTONE_ERR_LOG("failed to get media info");
    }

    err = AddToTransaction();
    if (err != E_OK) {
        RINGTONE_ERR_LOG("failed to add to transaction err %{public}d", err);
        return err;
    }

    return E_OK;
}

int32_t RingtoneScannerObj::AddToTransaction()
{
    dataBuffer_.emplace_back(move(data_));
    if (dataBuffer_.size() >= MAX_BATCH_SIZE) {
        return CommitTransaction();
    }

    return E_OK;
}

int32_t RingtoneScannerObj::GetMediaInfo()
{
    auto pos = data_->GetMimeType().find_first_of("/");
    std::string mimePrefix = data_->GetMimeType().substr(0, pos) + "/*";
    if (find(EXTRACTOR_SUPPORTED_MIME.begin(), EXTRACTOR_SUPPORTED_MIME.end(),
        mimePrefix) != EXTRACTOR_SUPPORTED_MIME.end()) {
        return RingtoneMetadataExtractor::Extract(data_);
    }

    return E_OK;
}

int32_t RingtoneScannerObj::BuildData(const struct stat &statInfo)
{
    data_ = make_unique<RingtoneMetadata>();
    if (data_ == nullptr) {
        RINGTONE_ERR_LOG("failed to make unique ptr for metadata");
        return E_DATA;
    }

    if (S_ISDIR(statInfo.st_mode)) {
        return E_INVALID_ARGUMENTS;
    }

    int32_t err = RingtoneScannerDb::GetFileBasicInfo(path_, data_);
    if (err != E_OK) {
        RINGTONE_ERR_LOG("failed to get file basic info");
        return err;
    }

    for (const auto& pair : g_typeMap) {
        if (path_.find(pair.first) == 0) {
            data_->SetSourceType(pair.second.first);
            data_->SetToneType(pair.second.second);
        }
    }

    // file path
    data_->SetData(path_);
    auto dispName = RingtoneScannerUtils::GetFileNameFromUri(path_);
    data_->SetDisplayName(dispName);
    if (data_->GetTitle() == TITLE_DEFAULT) {
        data_->SetTitle(RingtoneScannerUtils::GetFileTitle(data_->GetDisplayName()));
    }

    // statinfo
    data_->SetSize(statInfo.st_size);
    data_->SetDateModified(static_cast<int64_t>(RingtoneFileUtils::Timespec2Millisecond(statInfo.st_mtim)));

    // extension and type
    std::string extension = RingtoneScannerUtils::GetFileExtension(path_);
    std::string mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(extension);
    data_->SetMimeType(mimeType);
    int32_t mime = RingtoneMimeTypeUtils::GetMediaTypeFromMimeType(mimeType);
    data_->SetMediaType(mime);

    return E_OK;
}

int32_t RingtoneScannerObj::ScanFileInternal()
{
    if (RingtoneScannerUtils::IsFileHidden(path_)) {
        RINGTONE_ERR_LOG("the file is hidden");
        return E_FILE_HIDDEN;
    }

    int32_t err = GetFileMetadata();
    if (err != E_OK) {
        if (err != E_SCANNED) {
            RINGTONE_ERR_LOG("failed to get file metadata");
        }
        return err;
    }
    err = GetMediaInfo();
    if (err != E_OK) {
        RINGTONE_ERR_LOG("failed to get ringtone info");
    }

    err = Commit();
    if (err != E_OK) {
        RINGTONE_ERR_LOG("failed to commit err %{public}d", err);
        return err;
    }

    return E_OK;
}

int32_t RingtoneScannerObj::Commit()
{
    std::string tab = RINGTONE_TABLE;
    uri_ = RingtoneScannerDb::InsertMetadata(*data_, tab);
    return E_OK;
}
} // namespace Media
} // namespace OHOS
