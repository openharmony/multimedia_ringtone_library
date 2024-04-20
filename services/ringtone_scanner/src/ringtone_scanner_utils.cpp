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
#define MLOG_TAG "RingtoneScannerUtils"

#include "ringtone_scanner_utils.h"

#include "ringtone_log.h"

namespace OHOS {
namespace Media {
using namespace std;

// Check if file exists or not
bool RingtoneScannerUtils::IsExists(const string &path)
{
    struct stat statInfo {};

    if (path.empty()) {
        RINGTONE_ERR_LOG("Given path name is empty");
        return false;
    }

    return ((stat(path.c_str(), &statInfo)) == ERR_SUCCESS);
}

// Get the file name from file URI
string RingtoneScannerUtils::GetFileNameFromUri(const string &path)
{
    if (!path.empty()) {
        size_t lastSlashPosition = path.rfind("/");
        if (lastSlashPosition != string::npos) {
            if (path.size() > lastSlashPosition) {
                return path.substr(lastSlashPosition + 1);
            }
        }
    }

    RINGTONE_ERR_LOG("Failed to obtain file name because given pathname is empty");
    return "";
}

// Get file extension from the given filepath uri
string RingtoneScannerUtils::GetFileExtension(const string &path)
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

// Check if the given path is a directory path
bool RingtoneScannerUtils::IsDirectory(const string &path)
{
    struct stat s;

    if (!path.empty()) {
        if (stat(path.c_str(), &s) == 0) {
            if (s.st_mode & S_IFDIR) {
                return true;
            }
        }
    }

    RINGTONE_ERR_LOG("Either path is empty or it is not a directory");
    return false;
}

bool RingtoneScannerUtils::IsRegularFile(const string &path)
{
    struct stat s;
    if (!path.empty()) {
        if (stat(path.c_str(), &s) == 0) {
            if (s.st_mode & S_IFREG) {
                return true;
            }
        }
    }

    return false;
}

// Check if the given file starts with '.' , i.e. if it is hidden
bool RingtoneScannerUtils::IsFileHidden(const string &path)
{
    if (!path.empty()) {
        string fileName = GetFileNameFromUri(path);
        if (!fileName.empty() && fileName.at(0) == '.') {
            return true;
        }
    }

    return false;
}

// Get the parent path
string RingtoneScannerUtils::GetParentPath(const string &path)
{
    if (!path.empty()) {
        size_t lastSlashPosition = path.rfind("/");
        if (lastSlashPosition != string::npos && path.size() > lastSlashPosition) {
            return path.substr(0, lastSlashPosition);
        }
    }

    RINGTONE_ERR_LOG("Failed to obtain the parent path");
    return "";
}

string RingtoneScannerUtils::GetFileTitle(const string &displayName)
{
    string::size_type pos = displayName.find_last_of('.');
    return (pos == string::npos) ? displayName : displayName.substr(0, pos);
}

bool RingtoneScannerUtils::IsDirHidden(const string &path)
{
    bool dirHid = false;

    if (!path.empty()) {
        string dirName = RingtoneScannerUtils::GetFileNameFromUri(path);
        if (!dirName.empty() && dirName.at(0) == '.') {
            RINGTONE_DEBUG_LOG("hidden Directory, name:%{private}s path:%{private}s", dirName.c_str(), path.c_str());
            return true;
        }

        string curPath = path;
        string excludePath = curPath.append("/.nomedia");
        // Check is the folder consist of .nomedia file
        if (RingtoneScannerUtils::IsExists(excludePath)) {
            return true;
        }
    }

    return dirHid;
}

bool RingtoneScannerUtils::IsDirHiddenRecursive(const string &path)
{
    bool dirHid = true;
    string curPath = path;

    while (!IsDirHidden(curPath)) {
        curPath = RingtoneScannerUtils::GetParentPath(curPath);
        if (curPath.empty()) {
            dirHid = false;
            break;
        }
    }

    return dirHid;
}
} // namespace Media
} // namespace OHOS
