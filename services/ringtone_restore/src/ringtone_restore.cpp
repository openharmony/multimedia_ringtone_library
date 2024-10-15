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

#define MLOG_TAG "RingtoneRestore"

#include "ringtone_restore.h"

#include <sys/stat.h>

#include "datashare_ext_ability.h"
#include "datashare_ext_ability_context.h"
#include "result_set_utils.h"
#include "ringtone_restore_type.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
using namespace std;
static const int32_t QUERY_COUNT = 500;
static const int32_t INVALID_QUERY_OFFSET = -1;
static string RINGTONELIBRARY_DB_PATH = "/data/storage/el2/database";
int32_t RingtoneRestore::Init(const std::string &backupPath)
{
    RINGTONE_INFO_LOG("Init db start");
    if (backupPath.empty()) {
        RINGTONE_ERR_LOG("error: backup path is null");
        return E_INVALID_ARGUMENTS;
    }
    dbPath_ = backupPath + RINGTONELIBRARY_DB_PATH + "/rdb" + "/" + RINGTONE_LIBRARY_DB_NAME;
    backupPath_ = backupPath;

    if (!RingtoneFileUtils::IsFileExists(dbPath_)) {
        RINGTONE_ERR_LOG("ringtone db is not exist, path=%{public}s", dbPath_.c_str());
        return E_FAIL;
    }
    if (RingtoneRestoreBase::Init(backupPath) != E_OK) {
        return E_FAIL;
    }
    int32_t err = RingtoneRestoreDbUtils::InitDb(restoreRdb_, RINGTONE_LIBRARY_DB_NAME, dbPath_,
        RINGTONE_BUNDLE_NAME, true);
    if (err != E_OK) {
        RINGTONE_ERR_LOG("ringtone rdb fail, err = %{public}d", err);
        return E_HAS_DB_ERROR;
    }

    RINGTONE_INFO_LOG("Init db successfully");
    return E_OK;
}

vector<FileInfo> RingtoneRestore::QueryFileInfos(int32_t offset)
{
    vector<FileInfo> result;
    string querySql = "SELECT * FROM " + RINGTONE_TABLE;
    if (offset != INVALID_QUERY_OFFSET) {
        querySql += " LIMIT " + to_string(offset) + ", " + to_string(QUERY_COUNT);
    }
    auto resultSet = restoreRdb_->QuerySql(querySql);
    if (resultSet == nullptr) {
        return {};
    }

    vector<shared_ptr<RingtoneMetadata>> metaDatas {};
    auto ret = resultSet->GoToFirstRow();
    while (ret == NativeRdb::E_OK) {
        auto metaData = make_unique<RingtoneMetadata>();
        if (PopulateMetadata(resultSet, metaData) != E_OK) {
            RINGTONE_INFO_LOG("read resultset error");
            continue;
        }
        metaDatas.push_back(std::move(metaData));
        ret = resultSet->GoToNextRow();
    };
    resultSet->Close();

    return ConvertToFileInfos(metaDatas);
}

vector<FileInfo> RingtoneRestore::ConvertToFileInfos(vector<shared_ptr<RingtoneMetadata>> &metaDatas)
{
    vector<FileInfo> infos = {};
    for (auto meta : metaDatas) {
        infos.emplace_back(*meta);
    }
    return infos;
}

void RingtoneRestore::CheckRestoreFileInfos(vector<FileInfo> &infos)
{
    for (auto it = infos.begin(); it != infos.end();) {
        // at first, check backup file path
        string srcPath = backupPath_ + it->data;
        if (!RingtoneFileUtils::IsFileExists(srcPath)) {
            // 系统铃音不克隆，需要进行设置铃音判断
            if (it->sourceType == SOURCE_TYPE_PRESET) {
                it->restorePath = it->data;
                CheckSetting(*it);
            }
            RINGTONE_INFO_LOG("warnning:backup file is not exist, path=%{private}s", srcPath.c_str());
            infos.erase(it);
        } else {
            it++;
        }
    }
}

int32_t RingtoneRestore::StartRestore()
{
    if (restoreRdb_ == nullptr || backupPath_.empty()) {
        return E_FAIL;
    }
    auto ret = RingtoneRestoreBase::StartRestore();
    if (ret != E_OK) {
        return ret;
    }
    auto infos = QueryFileInfos(INVALID_QUERY_OFFSET);
    if ((!infos.empty()) && (infos.size() != 0)) {
        CheckRestoreFileInfos(infos);
        ret = InsertTones(infos);
    }
    FlushSettings();
    return ret;
}

void RingtoneRestore::UpdateRestoreFileInfo(FileInfo &info)
{
    info.displayName = RingtoneFileUtils::GetFileNameFromPath(info.restorePath);
    if (info.title == TITLE_DEFAULT) {
        info.title = RingtoneFileUtils::GetBaseNameFromPath(info.restorePath);
    }

    struct stat statInfo;
    if (stat(info.restorePath.c_str(), &statInfo) != 0) {
        RINGTONE_ERR_LOG("stat syscall err %{public}d", errno);
        return;
    }
    info.dateModified = static_cast<int64_t>(RingtoneFileUtils::Timespec2Millisecond(statInfo.st_mtim));
}

bool RingtoneRestore::OnPrepare(FileInfo &info, const std::string &destPath)
{
    if (!RingtoneFileUtils::IsFileExists(destPath)) {
        return false;
    }

    string fileName = RingtoneFileUtils::GetFileNameFromPath(info.data);
    if (fileName.empty()) {
        RINGTONE_ERR_LOG("src file name is null");
        return false;
    }
    string baseName = RingtoneFileUtils::GetBaseNameFromPath(info.data);
    if (baseName.empty()) {
        RINGTONE_ERR_LOG("src file base name is null");
        return false;
    }
    string extensionName = RingtoneFileUtils::GetExtensionFromPath(info.data);

    int32_t repeatCount = 1;
    string srcPath = backupPath_ + info.data;
    info.restorePath = destPath + "/" + fileName;
    while (RingtoneFileUtils::IsFileExists(info.restorePath)) {
        if (RingtoneFileUtils::IsSameFile(srcPath, info.restorePath)) {
            CheckSetting(info);
            RINGTONE_ERR_LOG("samefile: srcPath=%{private}s, dstPath=%{private}s", srcPath.c_str(),
                info.restorePath.c_str());
            return false;
        }
        info.restorePath = destPath + "/" + baseName + "(" + to_string(repeatCount++) + ")" + "." + extensionName;
    }

    if (!RingtoneRestoreBase::MoveFile(srcPath, info.restorePath)) {
        return false;
    }

    UpdateRestoreFileInfo(info);

    return true;
}

void RingtoneRestore::OnFinished(vector<FileInfo> &infos)
{
    if (!RingtoneFileUtils::RemoveDirectory(backupPath_)) {
        RINGTONE_ERR_LOG("cleanup backup dir failed, restorepath=%{public}s, err: %{public}s",
            backupPath_.c_str(), strerror(errno));
    }
}
} // namespace Media
} // namespace OHOS
