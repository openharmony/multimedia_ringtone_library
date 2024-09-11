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
#define MLOG_TAG "FileUtils"

#include "ringtone_file_utils.h"

#include <dirent.h>
#include <fcntl.h>
#include <ftw.h>
#include <fstream>
#include <sstream>
#include <sys/sendfile.h>
#include <unistd.h>
#include <iostream>

#include "directory_ex.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_type.h"
#include "vibrate_type.h"
#include "securec.h"

namespace OHOS {
namespace Media {
using namespace std;
static const int32_t OPEN_FDS = 128;
static const mode_t MODE_RWX_USR_GRP = 02771;
static const mode_t MODE_RW_USR = 0644;
const vector<string> EXIF_SUPPORTED_EXTENSION = {
    RINGTONE_CONTAINER_TYPE_MP3,
    RINGTONE_CONTAINER_TYPE_OGG
};

static bool IsTargetExtension(const string &path)
{
    const string ext = RingtoneFileUtils::GetExtensionFromPath(path);
    bool ret = find(EXIF_SUPPORTED_EXTENSION.begin(), EXIF_SUPPORTED_EXTENSION.end(), ext) !=
        EXIF_SUPPORTED_EXTENSION.end();
    if (!ret) {
        RINGTONE_ERR_LOG("invalid target extension:%{public}s", ext.c_str());
    }
    return ret;
}

static bool IsVibrateFile(const string &path)
{
    const string ext = RingtoneFileUtils::GetExtensionFromPath(path);
    bool ret = (ext == VIBRATE_CONTAINER_TYPE_JSON);
    if (!ret) {
        RINGTONE_ERR_LOG("invalid target extension:%{public}s", ext.c_str());
    }
    return ret;
}

string RingtoneFileUtils::SplitByChar(const string &str, const char split)
{
    size_t splitIndex = str.find_last_of(split);
    return (splitIndex == string::npos) ? ("") : (str.substr(splitIndex + 1));
}

string RingtoneFileUtils::GetExtensionFromPath(const string &path)
{
    string extention = SplitByChar(path, '.');
    if (!extention.empty()) {
        transform(extention.begin(), extention.end(), extention.begin(), ::tolower);
    }
    return extention;
}

string RingtoneFileUtils::GetFileNameFromPath(const string &path)
{
    string fileName = {};
    size_t found = path.rfind("/");
    if (found != string::npos && (found + 1) < path.size()) {
        fileName = path.substr(found + 1);
    } else {
        fileName = "";
    }

    return fileName;
}

static string ParseFromUri(const string& path, const string& key)
{
    RINGTONE_INFO_LOG("parsing uri : %{public}s for key : %{public}s", path.c_str(), key.c_str());
    auto keyLen = key.size();
    auto found = path.find(key);
    if (found == string::npos) {
        RINGTONE_INFO_LOG("there is no such field in uri: %{public}s", path.c_str());
        return "";
    }
    string sub = path.substr(found + keyLen + 1);
    found = sub.find("&");
    if (found != string::npos) {
        sub = sub.substr(0, found);
    }
    sub = RingtoneFileUtils::UrlDecode(sub);
    RINGTONE_INFO_LOG("parsing uri : %{public}s -> key=%{public}s, value=%{public}s",
        path.c_str(), key.c_str(), sub.c_str());
    return sub;
}

string RingtoneFileUtils::GetFileNameFromPathOrUri(const string &path, bool &isTitle)
{
    string fileName = {};
    size_t found = path.find("content://");
    if (found == 0) {
        fileName = ParseFromUri(path, "title"); // Pay attention! It's actually "title".
        isTitle = true;
    } else {
        fileName = GetFileNameFromPath(path);
        isTitle = false;
    }
    RINGTONE_INFO_LOG("%{public}s -> %{public}s", path.c_str(), fileName.c_str());
    return fileName;
}

string RingtoneFileUtils::GetBaseNameFromPath(const string &path)
{
    size_t found = path.rfind("/");
    size_t foundDot = path.rfind(".");

    string baseName = {};
    found = (found == string::npos ? 0 : found);
    if ((foundDot > found) && (foundDot != string::npos)) {
        baseName = path.substr(found + 1, foundDot - found - 1);
    } else {
        baseName = "";
    }

    return baseName;
}

bool RingtoneFileUtils::IsSameFile(const string &srcPath, const string &dstPath)
{
    struct stat srcStatInfo {};
    struct stat dstStatInfo {};

    if (access(srcPath.c_str(), F_OK) || access(dstPath.c_str(), F_OK)) {
        return false;
    }
    if (stat(srcPath.c_str(), &srcStatInfo) != 0) {
        RINGTONE_ERR_LOG("Failed to get file %{private}s StatInfo, err=%{public}d", srcPath.c_str(), errno);
        return false;
    }
    if (stat(dstPath.c_str(), &dstStatInfo) != 0) {
        RINGTONE_ERR_LOG("Failed to get file %{private}s StatInfo, err=%{public}d", dstPath.c_str(), errno);
        return false;
    }
    if (srcStatInfo.st_size != dstStatInfo.st_size) { /* file size */
        RINGTONE_INFO_LOG("Size differs, srcStatInfo.st_size != dstStatInfo.st_size");
        return false;
    }

    return true;
}

static int32_t UnlinkCb(const char *fpath, const struct stat *sb, int32_t typeflag, struct FTW *ftwbuf)
{
    CHECK_AND_RETURN_RET_LOG(fpath != nullptr, E_FAIL, "fpath == nullptr");
    int32_t errRet = remove(fpath);
    if (errRet) {
        RINGTONE_ERR_LOG("Failed to remove errno: %{public}d, path: %{private}s", errno, fpath);
    }

    return errRet;
}

int32_t RingtoneFileUtils::RemoveDirectory(const string &path)
{
    return nftw(path.c_str(), UnlinkCb, OPEN_FDS, FTW_DEPTH | FTW_PHYS);
}

bool RingtoneFileUtils::Mkdir(const string &subStr, shared_ptr<int> errCodePtr)
{
    mode_t mask = umask(0);
    if (mkdir(subStr.c_str(), MODE_RWX_USR_GRP) == -1) {
        if (errCodePtr != nullptr) {
            *errCodePtr = errno;
        }
        RINGTONE_ERR_LOG("Failed to create directory %{public}d", errno);
        umask(mask);
        return (errno == EEXIST) ? true : false;
    }
    umask(mask);
    return true;
}

bool RingtoneFileUtils::IsDirectory(const string &dirName, shared_ptr<int> errCodePtr)
{
    struct stat statInfo {};

    if (stat(dirName.c_str(), &statInfo) == 0) {
        if (statInfo.st_mode & S_IFDIR) {
            return true;
        }
    } else if (errCodePtr != nullptr) {
        *errCodePtr = errno;
        return false;
    }

    return false;
}

bool RingtoneFileUtils::CreateDirectory(const string &dirPath, shared_ptr<int> errCodePtr)
{
    string subStr;
    string segment;

    stringstream folderStream(dirPath);
    while (getline(folderStream, segment, '/')) {
        if (segment.empty()) {
            continue;
        }

        subStr.append(RINGTONE_SLASH_CHAR + segment);
        if (!IsDirectory(subStr, errCodePtr)) {
            if (!Mkdir(subStr, errCodePtr)) {
                return false;
            }
        }
    }
    return true;
}

int32_t RingtoneFileUtils::OpenFile(const string &filePath, const string &mode)
{
    int32_t errCode = E_ERR;

    if (filePath.empty() || mode.empty()) {
        RINGTONE_ERR_LOG("Invalid open argument! mode: %{private}s, path: %{private}s", mode.c_str(), filePath.c_str());
        return errCode;
    }

    if (!IsTargetExtension(filePath)) {
        return E_INVALID_PATH;
    }

    static const unordered_map<string, int32_t> RINGTONE_OPEN_MODE_MAP = {
        { RINGTONE_FILEMODE_READONLY, O_RDONLY },
        { RINGTONE_FILEMODE_WRITEONLY, O_WRONLY },
        { RINGTONE_FILEMODE_READWRITE, O_RDWR },
        { RINGTONE_FILEMODE_WRITETRUNCATE, O_WRONLY | O_TRUNC },
        { RINGTONE_FILEMODE_WRITEAPPEND, O_WRONLY | O_APPEND },
        { RINGTONE_FILEMODE_READWRITETRUNCATE, O_RDWR | O_TRUNC },
        { RINGTONE_FILEMODE_READWRITEAPPEND, O_RDWR | O_APPEND },
    };
    if (RINGTONE_OPEN_MODE_MAP.find(mode) == RINGTONE_OPEN_MODE_MAP.end()) {
        return E_ERR;
    }

    if (filePath.size() >= PATH_MAX) {
        RINGTONE_ERR_LOG("File path too long %{public}d", (int)filePath.size());
        return errCode;
    }
    string absFilePath;
    if (!PathToRealPath(filePath, absFilePath)) {
        RINGTONE_ERR_LOG("file is not real path, file path: %{private}s", filePath.c_str());
        return errCode;
    }
    if (absFilePath.empty()) {
        RINGTONE_ERR_LOG("Failed to obtain the canonical path for source path %{public}d %{private}s",
            errno, filePath.c_str());
        return errCode;
    }
    RINGTONE_INFO_LOG("File absFilePath is %{private}s", absFilePath.c_str());
    return open(absFilePath.c_str(), RINGTONE_OPEN_MODE_MAP.at(mode));
}

int32_t RingtoneFileUtils::OpenVibrateFile(const std::string &filePath, const std::string &mode)
{
    int32_t errCode = E_ERR;

    if (filePath.empty() || mode.empty()) {
        RINGTONE_ERR_LOG("Invalid open argument! mode: %{private}s, path: %{private}s", mode.c_str(), filePath.c_str());
        return errCode;
    }

    if (!IsVibrateFile(filePath)) {
        return E_INVALID_PATH;
    }

    static const unordered_map<string, int32_t> RINGTONE_OPEN_MODE_MAP = {
        { RINGTONE_FILEMODE_READONLY, O_RDONLY },
        { RINGTONE_FILEMODE_WRITEONLY, O_WRONLY },
        { RINGTONE_FILEMODE_READWRITE, O_RDWR },
        { RINGTONE_FILEMODE_WRITETRUNCATE, O_WRONLY | O_TRUNC },
        { RINGTONE_FILEMODE_WRITEAPPEND, O_WRONLY | O_APPEND },
        { RINGTONE_FILEMODE_READWRITETRUNCATE, O_RDWR | O_TRUNC },
        { RINGTONE_FILEMODE_READWRITEAPPEND, O_RDWR | O_APPEND },
    };
    if (RINGTONE_OPEN_MODE_MAP.find(mode) == RINGTONE_OPEN_MODE_MAP.end()) {
        return E_ERR;
    }

    if (filePath.size() >= PATH_MAX) {
        RINGTONE_ERR_LOG("File path too long %{public}d", (int)filePath.size());
        return errCode;
    }
    string absFilePath;
    if (!PathToRealPath(filePath, absFilePath)) {
        RINGTONE_ERR_LOG("file is not real path, file path: %{private}s", filePath.c_str());
        return errCode;
    }
    if (absFilePath.empty()) {
        RINGTONE_ERR_LOG("Failed to obtain the canonical path for source path %{public}d %{private}s",
            errno, filePath.c_str());
        return errCode;
    }
    RINGTONE_INFO_LOG("File absFilePath is %{private}s", absFilePath.c_str());
    return open(absFilePath.c_str(), RINGTONE_OPEN_MODE_MAP.at(mode));
}

bool RingtoneFileUtils::IsFileExists(const string &fileName)
{
    struct stat statInfo {};

    return ((stat(fileName.c_str(), &statInfo)) == 0);
}

int32_t RingtoneFileUtils::CreateFile(const string &filePath)
{
    int32_t errCode = E_ERR;

    if (filePath.empty()) {
        RINGTONE_ERR_LOG("Filepath is empty");
        return E_VIOLATION_PARAMETERS;
    }

    if (!IsTargetExtension(filePath)) {
        return E_INVALID_PATH;
    }

    if (IsFileExists(filePath)) {
        RINGTONE_ERR_LOG("the file exists path: %{private}s", filePath.c_str());
        return E_FILE_EXIST;
    }

    ofstream file(filePath);
    if (!file) {
        RINGTONE_ERR_LOG("Output file path could not be created errno %{public}d", errno);
        return errCode;
    }

    file.close();

    return E_SUCCESS;
}

bool RingtoneFileUtils::DeleteFile(const string &fileName)
{
    return (remove(fileName.c_str()) == 0);
}

bool RingtoneFileUtils::MoveFile(const string &oldPath, const string &newPath)
{
    bool errRet = false;

    if (IsFileExists(oldPath) && !IsFileExists(newPath)) {
        errRet = (rename(oldPath.c_str(), newPath.c_str()) == 0);
    }

    return errRet;
}

bool RingtoneFileUtils::CopyFileUtil(const string &filePath, const string &newPath)
{
    struct stat fst{};
    bool ret = false;
    if (filePath.size() >= PATH_MAX) {
        RINGTONE_ERR_LOG("File path too long %{public}d", static_cast<int>(filePath.size()));
        return ret;
    }
    RINGTONE_INFO_LOG("File path is %{private}s", filePath.c_str());
    string absFilePath;
    if (!PathToRealPath(filePath, absFilePath)) {
        RINGTONE_ERR_LOG("file is not real path, file path: %{private}s", filePath.c_str());
        return ret;
    }
    if (absFilePath.empty()) {
        RINGTONE_ERR_LOG("Failed to obtain the canonical path for source path%{private}s %{public}d",
            filePath.c_str(), errno);
        return ret;
    }

    int32_t source = open(absFilePath.c_str(), O_RDONLY);
    if (source == -1) {
        RINGTONE_ERR_LOG("Open failed for source file");
        return ret;
    }

    int32_t dest = open(newPath.c_str(), O_WRONLY | O_CREAT, MODE_RW_USR);
    if (dest == -1) {
        RINGTONE_ERR_LOG("Open failed for destination file %{public}d", errno);
        close(source);
        return ret;
    }

    if (fstat(source, &fst) == 0) {
        // Copy file content
        if (sendfile(dest, source, nullptr, fst.st_size) != E_ERR) {
            // Copy ownership and mode of source file
            if (fchown(dest, fst.st_uid, fst.st_gid) == 0 &&
                fchmod(dest, fst.st_mode) == 0) {
                ret= true;
            }
        }
    }

    close(source);
    close(dest);

    return ret;
}

int64_t RingtoneFileUtils::Timespec2Millisecond(const struct timespec &time)
{
    return time.tv_sec * MSEC_TO_SEC + time.tv_nsec / MSEC_TO_NSEC;
}

bool RingtoneFileUtils::StartsWith(const string &str, const string &prefix)
{
    return str.compare(0, prefix.size(), prefix) == 0;
}

int64_t RingtoneFileUtils::UTCTimeMilliSeconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * MSEC_TO_SEC + t.tv_nsec / MSEC_TO_NSEC;
}

int64_t RingtoneFileUtils::UTCTimeSeconds()
{
    struct timespec t{};
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &t);
    return (int64_t)(t.tv_sec);
}

string RingtoneFileUtils::StrCreateTimeByMilliseconds(const string &format, int64_t time)
{
    char strTime[DEFAULT_TIME_SIZE] = "";
    int64_t times = time / MSEC_TO_SEC;
    auto tm = localtime(&times);
    if (tm == nullptr) {
        return "";
    }
    (void)strftime(strTime, sizeof(strTime), format.c_str(), tm);
    return strTime;
}

static const int URL_DECODE_DOUBLE = 2;
string RingtoneFileUtils::UrlDecode(const string &src)
{
    string ret;
    char ch;
    int tmpNum;
    for (size_t i = 0; i < src.length(); i++) {
        if (src[i]=='%') {
            if (sscanf_s(src.substr(i + 1, URL_DECODE_DOUBLE).c_str(), "%x", &tmpNum) == -1) {
                RINGTONE_ERR_LOG("Not a valid url: %{private}s", src.c_str());
                return src;
            }
            ch = static_cast<char>(tmpNum);
            ret += ch;
            i = i + URL_DECODE_DOUBLE;
        } else {
            ret += src[i];
        }
    }
    return ret;
}

static int32_t MkdirRecursive(const string &path, size_t start)
{
    RINGTONE_DEBUG_LOG("start pos %{public}zu", start);
    size_t end = path.find("/", start + 1);

    string subDir = "";
    if (end == std::string::npos) {
        if (start + 1 == path.size()) {
            RINGTONE_DEBUG_LOG("path size=%zu", path.size());
        } else {
            subDir = path.substr(start + 1, path.size() - start - 1);
        }
    } else {
        subDir = path.substr(start + 1, end - start - 1);
    }

    if (subDir.size() == 0) {
        return E_SUCCESS;
    } else {
        string real = path.substr(0, start + subDir.size() + 1);
        mode_t mask = umask(0);
        int result = mkdir(real.c_str(), MODE_RWX_USR_GRP);
        if (result == 0) {
            RINGTONE_INFO_LOG("mkdir %{public}s successfully", real.c_str());
        } else {
            RINGTONE_INFO_LOG("mkdir %{public}s failed, errno is %{public}d", real.c_str(), errno);
        }
        umask(mask);
    }
    if (end == std::string::npos) {
        return E_SUCCESS;
    }

    return MkdirRecursive(path, end);
}

int32_t RingtoneFileUtils::CreatePreloadFolder(const string &path)
{
    RINGTONE_DEBUG_LOG("start");
    if (access(path.c_str(), F_OK) == 0) {
        RINGTONE_DEBUG_LOG("dir is existing");
        return E_SUCCESS;
    }

    auto start = path.find(RINGTONE_CUSTOMIZED_BASE_PATH);
    if (start == string::npos) {
        RINGTONE_ERR_LOG("base dir is wrong");
        return E_ERR;
    }

    return MkdirRecursive(path, start + RINGTONE_CUSTOMIZED_BASE_PATH.size());
}

void RingtoneFileUtils::CreateRingtoneDir()
{
    static const vector<string> userPreloadDirs = {
        { RINGTONE_CUSTOMIZED_ALARM_PATH }, { RINGTONE_CUSTOMIZED_RINGTONE_PATH },
        { RINGTONE_CUSTOMIZED_NOTIFICATIONS_PATH }
    };

    for (const auto &dir: userPreloadDirs) {
        if (CreatePreloadFolder(dir) != E_SUCCESS) {
            RINGTONE_INFO_LOG("scan failed on dir %{public}s", dir.c_str());
            continue;
        }
    }
}

int32_t RingtoneFileUtils::MoveDirectory(const std::string &srcDir, const std::string &dstDir)
{
    if (access(srcDir.c_str(), F_OK) != 0) {
        RINGTONE_ERR_LOG("access srcDir failed, errno is %{public}d", errno);
        return E_FAIL;
    }
    if (access(dstDir.c_str(), F_OK) != 0) {
        RINGTONE_ERR_LOG("access dstDir failed, errno is %{public}d", errno);
        return E_FAIL;
    }
    int ret = E_SUCCESS;
    for (const auto &dirEntry : std::filesystem::directory_iterator{ srcDir }) {
        std::string srcFilePath = dirEntry.path();
        std::string tmpFilePath = srcFilePath;
        std::string dstFilePath = tmpFilePath.replace(0, srcDir.length(), dstDir);
        if (!MoveFile(srcFilePath, dstFilePath)) {
            RINGTONE_ERR_LOG("Move file failed, errno is %{public}d", errno);
            ret = E_FAIL;
        }
    }
    return ret;
}

void RingtoneFileUtils::AccessRingtoneDir()
{
    if (access(RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH.c_str(), F_OK) != 0) {
        CreateRingtoneDir();
        return;
    }
    struct stat fileStat;
    if (stat(RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH.c_str(), &fileStat) != 0) {
        RINGTONE_ERR_LOG("stat dir failed, errno is %{public}d", errno);
        return;
    }
    // 检查组的写权限
    if ((fileStat.st_mode & S_IWGRP) != 0) {
        return;
    }
    if (IsEmptyFolder(RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH.c_str())) {
        RINGTONE_ERR_LOG("The directory is empty and lacks group write permission.");
        if (DeleteFile(RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH.c_str())) {
            RINGTONE_ERR_LOG("DeleteFile denied, errCode: %{public}d", errno);
        }
        CreateRingtoneDir();
    } else { //rename and move file
        if (MoveFile(RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH.c_str(),
            RINGTONE_CUSTOMIZED_BASE_RINGTONETMP_PATH.c_str())) {
            if (CreatePreloadFolder(RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH.c_str()) != E_SUCCESS) {
                RINGTONE_ERR_LOG("Create Ringtone dir failed, errno is %{public}d", errno);
                //restore dir
                MoveFile(RINGTONE_CUSTOMIZED_BASE_RINGTONETMP_PATH.c_str(),
                    RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH.c_str());
                return;
            }
            if (MoveDirectory(RINGTONE_CUSTOMIZED_BASE_RINGTONETMP_PATH.c_str(),
                RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH.c_str()) != E_SUCCESS) {
                RINGTONE_ERR_LOG("Move dir failed, errno is %{public}d", errno);
                CreateRingtoneDir();
                return;
            }
            if (DeleteFile(RINGTONE_CUSTOMIZED_BASE_RINGTONETMP_PATH.c_str())) {
                RINGTONE_ERR_LOG("DeleteFile denied, errCode: %{public}d", errno);
            }
        } else {
            RINGTONE_ERR_LOG("Move Ringtone dir failed, errno is %{public}d", errno);
        }
    }
    return;
}

string RingtoneFileUtils::GetFileExtension(const string &path)
{
    if (!path.empty()) {
        size_t dotIndex = path.rfind(".");
        if (dotIndex != string::npos) {
            return path.substr(dotIndex + 1);
        }
    }
 
    RINGTONE_ERR_LOG("Failed to obtain file extension because given pathname is empty");
    return "";
}
} // namespace Media
} // namespace OHOS
