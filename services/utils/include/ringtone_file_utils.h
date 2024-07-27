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

#ifndef RINGTONE_FILE_UTILS_H
#define RINGTONE_FILE_UTILS_H

#include <string>
#include <unordered_set>

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))

EXPORT const std::string RINGTONE_FILEMODE_READONLY = "r";
EXPORT const std::string RINGTONE_FILEMODE_WRITEONLY = "w";
EXPORT const std::string RINGTONE_FILEMODE_READWRITE = "rw";
EXPORT const std::string RINGTONE_FILEMODE_WRITETRUNCATE = "wt";
EXPORT const std::string RINGTONE_FILEMODE_WRITEAPPEND = "wa";
EXPORT const std::string RINGTONE_FILEMODE_READWRITETRUNCATE = "rwt";
EXPORT const std::string RINGTONE_FILEMODE_READWRITEAPPEND = "rwa";
EXPORT const std::unordered_set<std::string> RINGTONE_OPEN_MODES = {
    RINGTONE_FILEMODE_READONLY,
    RINGTONE_FILEMODE_WRITEONLY,
    RINGTONE_FILEMODE_READWRITE,
    RINGTONE_FILEMODE_WRITETRUNCATE,
    RINGTONE_FILEMODE_WRITEAPPEND,
    RINGTONE_FILEMODE_READWRITETRUNCATE,
    RINGTONE_FILEMODE_READWRITEAPPEND
};

constexpr int64_t MSEC_TO_SEC = 1e3;
constexpr int64_t MSEC_TO_NSEC = 1e6;
constexpr size_t DEFAULT_TIME_SIZE = 32;

class RingtoneFileUtils {
public:
    EXPORT static int64_t Timespec2Millisecond(const struct timespec &time);
    EXPORT static bool StartsWith(const std::string &str, const std::string &prefix);
    EXPORT static int64_t UTCTimeMilliSeconds();
    EXPORT static int64_t UTCTimeSeconds();
    EXPORT static std::string StrCreateTimeByMilliseconds(const std::string &format, int64_t time);
    EXPORT static std::string SplitByChar(const std::string &str, const char split);
    EXPORT static std::string GetExtensionFromPath(const std::string &path);
    EXPORT static std::string GetFileNameFromPath(const std::string &path);
    EXPORT static std::string GetFileNameFromPathOrUri(const std::string &path);
    EXPORT static std::string GetBaseNameFromPath(const std::string &path);
    EXPORT static int32_t OpenFile(const std::string &filePath, const std::string &mode);
    EXPORT static int32_t CreateFile(const std::string &filePath);
    EXPORT static bool IsSameFile(const std::string &srcPath, const std::string &dstPath);
    EXPORT static bool IsFileExists(const std::string &fileName);
    EXPORT static bool DeleteFile(const std::string &fileName);
    EXPORT static bool MoveFile(const std::string &oldPath, const std::string &newPath);
    EXPORT static bool CopyFileUtil(const std::string &filePath, const std::string &newPath);
    EXPORT static int32_t RemoveDirectory(const std::string &path);
    EXPORT static bool Mkdir(const std::string &subStr, std::shared_ptr<int> errCodePtr = nullptr);
    EXPORT static bool IsDirectory(const std::string &dirName, std::shared_ptr<int> errCodePtr = nullptr);
    EXPORT static bool CreateDirectory(const std::string &dirPath, std::shared_ptr<int> errCodePtr = nullptr);
    EXPORT static std::string UrlDecode(const std::string &url);
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_FILE_UTILS_H
