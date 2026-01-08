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

#include "dfx_const.h"
#include "directory_ex.h"
#include "parameter.h"
#include "preferences_helper.h"
#include "ringtone_default_setting.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_mimetype_utils.h"
#include "ringtone_rdbstore.h"
#include "ringtone_scanner_utils.h"
#ifdef USE_CONFIG_POLICY
#include "config_policy_utils.h"
#endif

namespace OHOS {
namespace Media {
using namespace std;
using namespace OHOS::AppExecFwk;
using namespace OHOS::DataShare;
static const int32_t SCANNER_WAIT_FOR_TIMEOUT = 10000; // ms
static const std::string PATH_PLAY_MODE_SYNC = "/synchronized";
static const std::string PATH_PLAY_MODE_CLASSIC = "/non-synchronized";
static const std::string PATH_VIBRATE_TYPE_STANDARD = "/standard";
static const std::string PATH_VIBRATE_TYPE_GENTLE = "/gentle";
static const std::string ALARMS_TYPE = "alarms";
static const std::string RINGTONES_TYPE = "ringtones";
static const std::string NOTIFICATIONS_TYPE = "notifications";
static const char RINGTONE_RDB_SCANNER_FLAG_KEY[] = "RDBInitScanner";
static const int RINGTONE_RDB_SCANNER_FLAG_KEY_TRUE = 1;
static const int RINGTONE_RDB_SCANNER_FLAG_KEY_FALSE = 0;
static const char RINGTONE_PARAMETER_SCANNER_FIRST_KEY[] = "ringtone.scanner.first";
static const char RINGTONE_PARAMETER_SCANNER_FIRST_TRUE[] = "true";
static const char RINGTONE_RESOURCE_PATH[] = "resource/media/audio";
static const char VIBRATE_RESOURCE_PATH[] = "resource/media/haptics";

static std::unordered_map<std::string, std::pair<int32_t, int32_t>> g_typeMap = {
    // customized tones map
    {RINGTONE_CUSTOMIZED_ALARM_PATH, {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_ALARM}},
    {RINGTONE_CUSTOMIZED_RINGTONE_PATH, {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_RINGTONE}},
    {RINGTONE_CUSTOMIZED_NOTIFICATIONS_PATH, {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_NOTIFICATION}},
    {RINGTONE_CUSTOMIZED_CONTACTS_PATH, {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_CONTACTS}},
    {RINGTONE_CUSTOMIZED_APP_NOTIFICATIONS_PATH, {SOURCE_TYPE_CUSTOMISED, TONE_TYPE_APP_NOTIFICATION}},
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
// vibrate type map
static std::unordered_map<std::string, std::pair<int32_t, int32_t>> g_vibrateTypeMap = {
    {ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH + PATH_VIBRATE_TYPE_STANDARD, {SOURCE_TYPE_PRESET, VIBRATE_TYPE_STANDARD}},
    {ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH + PATH_VIBRATE_TYPE_GENTLE, {SOURCE_TYPE_PRESET, VIBRATE_TYPE_GENTLE}},
    {ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH + PATH_VIBRATE_TYPE_STANDARD, {SOURCE_TYPE_PRESET, VIBRATE_TYPE_STANDARD}},
    {ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH + PATH_VIBRATE_TYPE_GENTLE, {SOURCE_TYPE_PRESET, VIBRATE_TYPE_GENTLE}},
    {ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH + PATH_VIBRATE_TYPE_STANDARD, {SOURCE_TYPE_PRESET, VIBRATE_TYPE_STANDARD}},
    {ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH + PATH_VIBRATE_TYPE_GENTLE, {SOURCE_TYPE_PRESET, VIBRATE_TYPE_GENTLE}},
};

static std::unordered_map<std::string, std::pair<int32_t, int32_t>> g_vibratePlayModeMap = {
    {ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH + PATH_VIBRATE_TYPE_STANDARD + PATH_PLAY_MODE_SYNC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_SYNC}},
    {ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH + PATH_VIBRATE_TYPE_STANDARD + PATH_PLAY_MODE_CLASSIC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_CLASSIC}},
    {ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH + PATH_VIBRATE_TYPE_GENTLE + PATH_PLAY_MODE_SYNC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_SYNC}},
    {ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH + PATH_VIBRATE_TYPE_GENTLE + PATH_PLAY_MODE_CLASSIC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_CLASSIC}},
    {ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH + PATH_VIBRATE_TYPE_STANDARD + PATH_PLAY_MODE_SYNC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_SYNC}},
    {ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH + PATH_VIBRATE_TYPE_STANDARD + PATH_PLAY_MODE_CLASSIC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_CLASSIC}},
    {ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH + PATH_VIBRATE_TYPE_GENTLE + PATH_PLAY_MODE_SYNC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_SYNC}},
    {ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH + PATH_VIBRATE_TYPE_GENTLE + PATH_PLAY_MODE_CLASSIC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_CLASSIC}},
    {ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH + PATH_VIBRATE_TYPE_STANDARD + PATH_PLAY_MODE_SYNC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_SYNC}},
    {ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH + PATH_VIBRATE_TYPE_STANDARD + PATH_PLAY_MODE_CLASSIC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_CLASSIC}},
    {ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH + PATH_VIBRATE_TYPE_GENTLE + PATH_PLAY_MODE_SYNC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_SYNC}},
    {ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH + PATH_VIBRATE_TYPE_GENTLE + PATH_PLAY_MODE_CLASSIC,
        {SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_CLASSIC}},
};

static const std::vector<std::string> g_preloadDirs = {
    {ROOT_TONE_PRELOAD_PATH_NOAH_PATH + "/alarms"},
    {ROOT_TONE_PRELOAD_PATH_NOAH_PATH + "/ringtones"},
    {ROOT_TONE_PRELOAD_PATH_NOAH_PATH + "/notifications"},
    {ROOT_TONE_PRELOAD_PATH_CHINA_PATH + "/alarms"},
    {ROOT_TONE_PRELOAD_PATH_CHINA_PATH + "/ringtones"},
    {ROOT_TONE_PRELOAD_PATH_CHINA_PATH + "/notifications"},
    {ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "/alarms"},
    {ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "/ringtones"},
    {ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH + "/notifications"},
    {ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH + "/standard"},
    {ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH + "/gentle"},
    {ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH + "/standard"},
    {ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH + "/gentle"},
    {ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH + "/standard"},
    {ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH + "/gentle"},
};

static const std::vector<std::string> g_ringtoneAndVibratePaths = {
    {ROOT_TONE_PRELOAD_PATH_NOAH_PATH},
    {ROOT_TONE_PRELOAD_PATH_CHINA_PATH},
    {ROOT_TONE_PRELOAD_PATH_OVERSEA_PATH},
    {ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH},
    {ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH},
    {ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH},
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

int32_t RingtoneScannerObj::UpdateDefaultTone()
{
    auto rdbStore = RingtoneRdbStore::GetInstance();
    CHECK_AND_RETURN_RET_LOG(rdbStore != nullptr, E_ERR, "rdbStore is nullptr");
    auto rawRdb = rdbStore->GetRaw();
    CHECK_AND_RETURN_RET_LOG(rawRdb != nullptr, E_ERR, "rawRdb is nullptr");
    int errCode = 0;
    shared_ptr<NativePreferences::Preferences> prefs =
        NativePreferences::PreferencesHelper::GetPreferences(COMMON_XML_EL1, errCode);
    CHECK_AND_RETURN_RET_LOG(prefs, E_ERR, "Preferences is null");
    int isScanner = prefs->GetInt(RINGTONE_RDB_SCANNER_FLAG_KEY, RINGTONE_RDB_SCANNER_FLAG_KEY_FALSE);
    if (isScanner == RINGTONE_RDB_SCANNER_FLAG_KEY_TRUE) {
        RingtoneDefaultSetting::GetObj(rawRdb)->UpdateDefaultSystemTone();
        RINGTONE_INFO_LOG("The default ringtone has been set, no need to be configured again");
        return E_OK;
    }
    // reset ringtone default settings
    RingtoneDefaultSetting::GetObj(rawRdb)->Update();
    RingtoneDefaultSetting::GetObj(rawRdb)->UpdateDefaultSystemTone();
    prefs->PutInt(RINGTONE_RDB_SCANNER_FLAG_KEY, RINGTONE_RDB_SCANNER_FLAG_KEY_TRUE);
    prefs->FlushSync();
    return E_OK;
}

int32_t RingtoneScannerObj::BootScanProcess()
{
    int64_t scanStart = RingtoneFileUtils::UTCTimeMilliSeconds();
    int32_t ret = E_OK;
    bool res = true;
    res = RingtoneScannerDb::UpdateScannerFlag();
    CHECK_AND_RETURN_RET_LOG(res, E_HAS_DB_ERROR, "UpdateScannerFlag operation failed, res: %{public}d",
        E_HAS_DB_ERROR);
    IncrementalScannResource();

    ret = ScanDirectories(g_preloadDirs);
    CHECK_AND_RETURN_RET_LOG(ret == E_OK, ret, "ScanDirectories for g_preloadDirs err, ret: %{public}d", ret);

    ret = UpdateDefaultTone();
    CHECK_AND_RETURN_RET_LOG(ret == E_OK, ret, "UpdateDefaultTone operation failed, ret: %{public}d", ret);

    int64_t scanEnd = RingtoneFileUtils::UTCTimeMilliSeconds();
    RINGTONE_INFO_LOG("total preload tone files count:%{public}d, scanned: %{public}d, costed-time:%{public}"
        PRId64 " ms", tonesScannedCount_, tonesScannedCount_, scanEnd - scanStart);
    res = RingtoneScannerDb::DeleteNotExist();
    CHECK_AND_RETURN_RET_LOG(res, E_ERR, "DeleteNotExist operation failed, res: %{public}d", res);
    return ret;
}

int32_t RingtoneScannerObj::BootScan()
{
    RINGTONE_INFO_LOG("start to BootScan");
    int32_t ret = BootScanProcess();
    unique_lock<mutex> lock(scannerLock_);
    scannerCv_.notify_one();
    if (ret == E_OK) {
        int result = SetParameter(RINGTONE_PARAMETER_SCANNER_FIRST_KEY, RINGTONE_PARAMETER_SCANNER_FIRST_TRUE);
        RINGTONE_INFO_LOG("SetParameter scan end, result: %{public}d", result);
    }
    return ret;
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

    err = CommitVibrateTransaction();
    if (err != E_OK) {
        RINGTONE_ERR_LOG("commit vibrate transaction err %{public}d", err);
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

int32_t RingtoneScannerObj::CommitVibrateTransaction()
{
    unique_ptr<VibrateMetadata> vibrateData;
    string vibrateTableName = VIBRATE_TABLE;

    for (uint32_t i = 0; i < vibrateDataBuffer_.size(); i++) {
        vibrateData = move(vibrateDataBuffer_[i]);
        if (vibrateData->GetVibrateId() != FILE_ID_DEFAULT) {
            RingtoneScannerDb::UpdateVibrateMetadata(*vibrateData, vibrateTableName);
        } else {
            RingtoneScannerDb::InsertVibrateMetadata(*vibrateData, vibrateTableName);
        }
    }

    if (vibrateDataBuffer_.size() > 0) {
        tonesScannedCount_ += vibrateDataBuffer_.size();
    }
    vibrateDataBuffer_.clear();

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
            RINGTONE_ERR_LOG("Failed to copy file name %{private}s", fName);
            continue;
        }
        if (lstat(fName, &statInfo) == -1) {
            RINGTONE_ERR_LOG("Failed to get info of directory %{private}s", fName);
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

    std::vector<string> vibratePath;
    GetRingToneSourcePath(VIBRATE_RESOURCE_PATH, vibratePath);
    bool flag = (path_.find(ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH) != std::string::npos) ? true : false;
    flag |= (path_.find(ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH) != std::string::npos);
    flag |= (path_.find(ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH) != std::string::npos);
    flag |= ContainsAnyPath(path_, vibratePath);
    std::string extension = RingtoneScannerUtils::GetFileExtension(path_);

    if (flag) {
        if (extension.compare("json") == 0) {
            isVibrateFile_ = true;
            return ScanVibrateFile();
        }
        return E_OK;
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

    int errCode = 0;
    if (isVibrateFile_) {
        errCode = BuildVibrateData(statInfo);
        if (errCode != E_OK) {
            return errCode;
        }
    } else {
        errCode = BuildData(statInfo);
        if (errCode != E_OK) {
            return errCode;
        }
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
    if (isVibrateFile_) {
        vibrateDataBuffer_.emplace_back(move(vibrateData_));
        if (vibrateDataBuffer_.size() >= MAX_BATCH_SIZE) {
            return CommitVibrateTransaction();
        }
    } else {
        dataBuffer_.emplace_back(move(data_));
        if (dataBuffer_.size() >= MAX_BATCH_SIZE) {
            return CommitTransaction();
        }
    }

    return E_OK;
}

int32_t RingtoneScannerObj::GetMediaInfo()
{
#ifdef ENABLE_METADATA_EXTRACTOR
    auto pos = data_->GetMimeType().find_first_of("/");
    std::string mimePrefix = data_->GetMimeType().substr(0, pos) + "/*";
    if (find(EXTRACTOR_SUPPORTED_MIME.begin(), EXTRACTOR_SUPPORTED_MIME.end(), mimePrefix) !=
        EXTRACTOR_SUPPORTED_MIME.end()) {
        return RingtoneMetadataExtractor::Extract(data_);
    }
#endif // ENABLE_METADATA_EXTRACTOR
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

int32_t RingtoneScannerObj::BuildVibrateData(const struct stat &statInfo)
{
    vibrateData_ = make_unique<VibrateMetadata>();
    if (vibrateData_ == nullptr) {
        RINGTONE_ERR_LOG("failed to make unique ptr for metadata");
        return E_DATA;
    }

    if (S_ISDIR(statInfo.st_mode)) {
        return E_INVALID_ARGUMENTS;
    }

    int32_t err = RingtoneScannerDb::GetVibrateFileBasicInfo(path_, vibrateData_);
    if (err != E_OK) {
        RINGTONE_ERR_LOG("failed to get file basic info");
        return err;
    }

    for (const auto &pair : g_vibrateTypeMap) {
        if (path_.find(pair.first) == 0) {
            vibrateData_->SetSourceType(pair.second.first);
            vibrateData_->SetVibrateType(pair.second.second);
            int32_t ntype = 0;
            if (pair.second.second == VIBRATE_TYPE_STANDARD) {
                ntype = (path_.find(ALARMS_TYPE) != string::npos) ? VIBRATE_TYPE_SALARM : VIBRATE_TYPE_STANDARD;
                ntype = (path_.find(RINGTONES_TYPE) != string::npos) ? VIBRATE_TYPE_SRINGTONE : ntype;
                ntype = (path_.find(NOTIFICATIONS_TYPE) != string::npos) ? \
                    VIBRATE_TYPE_SNOTIFICATION : ntype;
                vibrateData_->SetVibrateType(ntype);
            } else {
                ntype = (path_.find(ALARMS_TYPE) != string::npos) ? VIBRATE_TYPE_GALARM : VIBRATE_TYPE_GENTLE;
                ntype = (path_.find(RINGTONES_TYPE) != string::npos) ? VIBRATE_TYPE_GRINGTONE : ntype;
                ntype = (path_.find(NOTIFICATIONS_TYPE) != string::npos) ? \
                    VIBRATE_TYPE_GNOTIFICATION : ntype;
                vibrateData_->SetVibrateType(ntype);
            }
        }
    }

    for (const auto &pair : g_vibratePlayModeMap) {
        if (path_.find(pair.first) == 0) {
            vibrateData_->SetPlayMode(pair.second.second);
        }
    }

    // file path
    vibrateData_->SetData(path_);
    auto dispName = RingtoneScannerUtils::GetFileNameFromUri(path_);
    vibrateData_->SetDisplayName(dispName);
    if (vibrateData_->GetTitle() == TITLE_DEFAULT) {
        vibrateData_->SetTitle(RingtoneScannerUtils::GetFileTitle(vibrateData_->GetDisplayName()));
    }

    // statinfo
    vibrateData_->SetSize(statInfo.st_size);
    vibrateData_->SetDateModified(static_cast<int64_t>(RingtoneFileUtils::Timespec2Millisecond(statInfo.st_mtim)));

    return E_OK;
}

int32_t RingtoneScannerObj::ScanFileInternal()
{
    if (RingtoneScannerUtils::IsFileHidden(path_)) {
        RINGTONE_ERR_LOG("the file is hidden");
        return E_FILE_HIDDEN;
    }

    std::vector<string> vibratePath;
    GetRingToneSourcePath(VIBRATE_RESOURCE_PATH, vibratePath);
    bool flag = (path_.find(ROOT_VIBRATE_PRELOAD_PATH_NOAH_PATH) != std::string::npos) ? true : false;
    flag |= (path_.find(ROOT_VIBRATE_PRELOAD_PATH_CHINA_PATH) != std::string::npos);
    flag |= (path_.find(ROOT_VIBRATE_PRELOAD_PATH_OVERSEA_PATH) != std::string::npos);
    flag |= ContainsAnyPath(path_, vibratePath);
    std::string extension = RingtoneScannerUtils::GetFileExtension(path_);

    if (flag) {
        if (extension.compare("json") == 0) {
            isVibrateFile_ = true;
            return ScanVibrateFile();
        }
        return E_INVALID_PATH;
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

int32_t RingtoneScannerObj::ScanVibrateFile()
{
    int32_t err = GetFileMetadata();
    if (err != E_OK) {
        if (err != E_SCANNED) {
            RINGTONE_ERR_LOG("failed to get vibrate file metadata");
        }
        isVibrateFile_ = false;
        return err;
    }

    if (type_ == FILE) {
        err = Commit();
        if (err != E_OK) {
            RINGTONE_ERR_LOG("failed to commit err %{public}d", err);
            isVibrateFile_ = false;
            return err;
        }
    } else {
        err = AddToTransaction();
        if (err != E_OK) {
            RINGTONE_ERR_LOG("failed to add to transaction err %{public}d", err);
            isVibrateFile_ = false;
            return err;
        }
    }

    isVibrateFile_ = false;
    return E_OK;
}

int32_t RingtoneScannerObj::Commit()
{
    std::string tab = RINGTONE_TABLE;

    if (isVibrateFile_) {
        tab = VIBRATE_TABLE;

        if (vibrateData_->GetVibrateId() != FILE_ID_DEFAULT) {
            uri_ = RingtoneScannerDb::UpdateVibrateMetadata(*vibrateData_, tab);
        } else {
            uri_ = RingtoneScannerDb::InsertVibrateMetadata(*vibrateData_, tab);
        }
    } else {
        if (data_->GetToneId() != FILE_ID_DEFAULT) {
            uri_ = RingtoneScannerDb::UpdateMetadata(*data_, tab);
        } else {
            uri_ = RingtoneScannerDb::InsertMetadata(*data_, tab);
        }
    }

    return E_OK;
}

int32_t RingtoneScannerObj::GetRingToneSourcePath(const char *source, vector<string> &sourcePaths)
{
    RINGTONE_INFO_LOG("start GetRingToneSourcePath");
#ifdef USE_CONFIG_POLICY
    CfgFiles *cfgFiles = GetCfgFiles(source);
    if (cfgFiles == nullptr) {
        RINGTONE_INFO_LOG("not found cfgFiles");
        return E_ERR;
    }

    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        if (cfgFiles->paths[i] && *(cfgFiles->paths[i]) != '\0') {
            sourcePaths.push_back(cfgFiles->paths[i]);
            RINGTONE_INFO_LOG("extra parameter config file path: %{public}s",
                RingtoneScannerUtils::GetSafePath(cfgFiles->paths[i]).c_str());
        }
    }
    FreeCfgFiles(cfgFiles);
#endif
    return E_OK;
}

int32_t RingtoneScannerObj::ScanDirectories(const std::vector<std::string>& dirs)
{
    int32_t ret = E_OK;
    if (dirs.empty()) {
        RINGTONE_INFO_LOG("dirs is empty");
        return E_ERR;
    }
    for (const auto& dir : dirs) {
        RINGTONE_DEBUG_LOG("start to scan realpath %{public}s",
            RingtoneScannerUtils::GetSafePath(dir.c_str()).c_str());
        std::string realPath;
        if (!PathToRealPath(dir, realPath)) {
            RINGTONE_DEBUG_LOG("failed to get realpath %{public}s, errno %{public}d",
                RingtoneScannerUtils::GetSafePath(dir.c_str()).c_str(), errno);
            continue;
        }

        RINGTONE_DEBUG_LOG("start to scan realpath %{public}s",
            RingtoneScannerUtils::GetSafePath(dir.c_str()).c_str());
        callback_ = std::make_shared<ScanErrCallback>(dir);

        if (RingtoneScannerUtils::IsDirectory(realPath)) {
            dir_ = std::move(realPath);
            ret = ScanDir();
        } else if (RingtoneScannerUtils::IsRegularFile(realPath)) {
            path_ = std::move(realPath);
            ret = ScanFile();
        }
        CHECK_AND_RETURN_RET_LOG(ret == E_OK, ret, "BootScan err, ret: %{public}d", ret);
    }
    return ret;
}

vector<string> RingtoneScannerObj::FilterResourcePaths(
    const vector<string>& sourcePaths, const vector<string>& pathsToFilter)
{
    if (sourcePaths.empty() || pathsToFilter.empty()) {
        RINGTONE_ERR_LOG("sourcePaths or pathsToFilter is empty");
        return {};
    }

    const unordered_set<string> filterSet(pathsToFilter.begin(), pathsToFilter.end());

    vector<string> filteredPaths;
    for (const auto& path : sourcePaths) {
        if (filterSet.find(path) == filterSet.end()) {
            filteredPaths.push_back(path);
        }
    }

    return filteredPaths;
}

vector<string> RingtoneScannerObj::BuildRingtoneDirs(const vector<string>& sourcePaths)
{
    if (sourcePaths.empty()) {
        RINGTONE_INFO_LOG("sourcePaths is empty");
        return {};
    }

    vector<string> filteredPaths;
    for (const auto& path : sourcePaths) {
        filteredPaths.push_back(path + "/" + ALARMS_TYPE);
        filteredPaths.push_back(path + "/" + RINGTONES_TYPE);
        filteredPaths.push_back(path + "/" + NOTIFICATIONS_TYPE);
    }

    return filteredPaths;
}

vector<string> RingtoneScannerObj::BuildVibrateDirs(const vector<string>& sourcePaths)
{
    if (sourcePaths.empty()) {
        RINGTONE_INFO_LOG("sourcePaths is empty");
        return {};
    }

    vector<string> filteredPaths;
    for (const auto& path : sourcePaths) {
        filteredPaths.push_back(path + PATH_VIBRATE_TYPE_STANDARD);
        filteredPaths.push_back(path + PATH_VIBRATE_TYPE_GENTLE);
    }

    return filteredPaths;
}

int32_t RingtoneScannerObj::AdditionalVibrateType(const std::vector<std::string>& vibratePaths)
{
    if (!vibratePaths.empty()) {
        for (const auto& path : vibratePaths) {
            g_vibrateTypeMap[path+PATH_VIBRATE_TYPE_STANDARD] = {SOURCE_TYPE_PRESET, VIBRATE_TYPE_STANDARD};
            g_vibrateTypeMap[path+PATH_VIBRATE_TYPE_GENTLE] = {SOURCE_TYPE_PRESET, VIBRATE_TYPE_GENTLE};
        }
    }
    return E_OK;
}

int32_t RingtoneScannerObj::AdditionalVibratePlayMode(const std::vector<std::string>& vibratePaths)
{
    if (!vibratePaths.empty()) {
        for (const auto& path : vibratePaths) {
            g_vibratePlayModeMap[path+PATH_VIBRATE_TYPE_STANDARD+PATH_PLAY_MODE_SYNC] = {
                SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_SYNC
            };
            g_vibratePlayModeMap[path+PATH_VIBRATE_TYPE_STANDARD + PATH_PLAY_MODE_CLASSIC] = {
                SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_CLASSIC
            };
            g_vibratePlayModeMap[path+PATH_VIBRATE_TYPE_GENTLE + PATH_PLAY_MODE_SYNC] = {
                SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_SYNC};
            g_vibratePlayModeMap[path+PATH_VIBRATE_TYPE_GENTLE + PATH_PLAY_MODE_CLASSIC] = {
                SOURCE_TYPE_PRESET, VIBRATE_PLAYMODE_CLASSIC};
        }
    }
    return E_OK;
}

bool RingtoneScannerObj::ContainsAnyPath(const std::string& input, const std::vector<std::string>& paths)
{
    if (paths.empty()) {
        return false;
    }
    return std::any_of(paths.begin(), paths.end(),
        [&input](const std::string& path) {
            return input.find(path) != std::string::npos;
        });
}

int32_t RingtoneScannerObj::AdditionalToneTypeMap(const std::vector<std::string>& tonePaths)
{
    if (!tonePaths.empty()) {
        for (const auto& path : tonePaths) {
            g_typeMap[path+ "/" + ALARMS_TYPE] = {
                SOURCE_TYPE_PRESET, TONE_TYPE_ALARM
            };
            g_typeMap[path+ "/" + RINGTONES_TYPE] = {
                SOURCE_TYPE_PRESET, TONE_TYPE_RINGTONE
            };
            g_typeMap[path+ "/" + NOTIFICATIONS_TYPE] = {
                SOURCE_TYPE_PRESET, TONE_TYPE_NOTIFICATION
            };
        }
    }
    return E_OK;
}

void RingtoneScannerObj::IncrementalScannResource()
{
    std::vector<string> ringtonePath;
    GetRingToneSourcePath(RINGTONE_RESOURCE_PATH, ringtonePath);
    auto filterRingtonePath = FilterResourcePaths(ringtonePath, g_ringtoneAndVibratePaths);
    AdditionalToneTypeMap(filterRingtonePath);

    std::vector<string> vibratePath;
    GetRingToneSourcePath(VIBRATE_RESOURCE_PATH, vibratePath);
    auto filterVibratePath = FilterResourcePaths(vibratePath, g_ringtoneAndVibratePaths);
    AdditionalVibrateType(filterVibratePath);
    AdditionalVibratePlayMode(filterVibratePath);

    ScanDirectories(BuildRingtoneDirs(filterRingtonePath));
    ScanDirectories(BuildVibrateDirs(filterVibratePath));
}
} // namespace Media
} // namespace OHOS
