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

int32_t RingtoneRestore::Init(const std::string &backupPath)
{
    RINGTONE_INFO_LOG("Init db start");
    if (backupPath.empty()) {
        RINGTONE_ERR_LOG("error: backup path is null");
        return E_INVALID_ARGUMENTS;
    }
    dbPath_ = backupPath + "/" + RINGTONE_LIBRARY_DB_NAME;
    backupPath_ = backupPath;

    if (!RingtoneFileUtils::IsFileExists(dbPath_)) {
        RINGTONE_ERR_LOG("ringtone db is not exist, path=%{public}s", dbPath_.c_str());
        return E_FAIL;
    }
    if (RingtoneRestoreBase::Init(backupPath) != E_OK) {
        return E_FAIL;
    }
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        RINGTONE_ERR_LOG("Failed to get context");
        return E_FAIL;
    }
    int32_t err = RingtoneRestoreDbUtils::InitDb(restoreRdb_, RINGTONE_LIBRARY_DB_NAME, dbPath_,
        context->GetBundleName(), true);
    if (err != E_OK) {
        RINGTONE_ERR_LOG("ringtone rdb fail, err = %{public}d", err);
        return E_HAS_DB_ERROR;
    }

    RINGTONE_INFO_LOG("Init db successfully");
    return E_OK;
}

void RingtoneRestore::ExtractMetaFromColumn(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &metadata, const std::string &col)
{
    ResultSetDataType dataType = ResultSetDataType::TYPE_NULL;
    RingtoneMetadata::RingtoneMetadataFnPtr requestFunc = nullptr;
    auto itr = metadata->memberFuncMap_.find(col);
    if (itr != metadata->memberFuncMap_.end()) {
        dataType = itr->second.first;
        requestFunc = itr->second.second;
    } else {
        RINGTONE_ERR_LOG("column name invalid %{private}s", col.c_str());
        return;
    }

    std::variant<int32_t, std::string, int64_t, double> data =
        ResultSetUtils::GetValFromColumn<const shared_ptr<NativeRdb::ResultSet>>(col, resultSet, dataType);

    if (requestFunc != nullptr) {
        (metadata.get()->*requestFunc)(data);
    }
}

int32_t RingtoneRestore::PopulateMetadata(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &metaData)
{
    std::vector<std::string> columnNames;
    int32_t err = resultSet->GetAllColumnNames(columnNames);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get all column names");
        return E_RDB;
    }

    for (const auto &col : columnNames) {
        ExtractMetaFromColumn(resultSet, metaData, col);
    }

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
        FileInfo info = {
            meta->GetToneId(),
            meta->GetData(),
            meta->GetSize(),
            meta->GetDisplayName(),
            meta->GetTitle(),
            meta->GetMediaType(),
            meta->GetToneType(),
            meta->GetMimeType(),
            meta->GetSourceType(),
            meta->GetDateAdded(),
            meta->GetDateModified(),
            meta->GetDateTaken(),
            meta->GetDuration(),
            meta->GetShotToneType(),
            meta->GetShotToneSourceType(),
            meta->GetNotificationToneType(),
            meta->GetNotificationToneSourceType(),
            meta->GetRingToneType(),
            meta->GetRingToneSourceType(),
            meta->GetAlarmToneType(),
            meta->GetAlarmToneSourceType(),
            {}
        };

        infos.emplace_back(info);
    }
    return infos;
}

void RingtoneRestore::CheckRestoreFileInfos(vector<FileInfo> &infos)
{
    for (auto it = infos.begin(); it != infos.end();) {
        // at first, check backup file path
        string srcPath = backupPath_ + it->data;
        if (!RingtoneFileUtils::IsFileExists(srcPath)) {
            RINGTONE_INFO_LOG("warnning:backup file is not exist, path=%{public}s", srcPath.c_str());
            infos.erase(it);
        } else {
            it++;
        }
    }
}

void RingtoneRestore::StartRestore()
{
    if (restoreRdb_ == nullptr || backupPath_.empty()) {
        return;
    }
    auto infos = QueryFileInfos(INVALID_QUERY_OFFSET);
    if ((!infos.empty()) && (infos.size() != 0)) {
        CheckRestoreFileInfos(infos);
        InsertTones(infos);
    }
}

bool RingtoneRestore::MoveFileInternal(const string &src, const string &dest)
{
    if (RingtoneFileUtils::MoveFile(src, dest)) {
        return true;
    }

    if (!RingtoneFileUtils::CopyFileUtil(src, dest)) {
        RINGTONE_ERR_LOG("copy-file failed, src: %{public}s, err: %{public}s", src.c_str(), strerror(errno));
        return false;
    }

    if (!RingtoneFileUtils::DeleteFile(src)) {
        RINGTONE_ERR_LOG("remove-file failed, filePath: %{public}s, err: %{public}s", src.c_str(), strerror(errno));
    }

    return true;
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
    if (fileName.empty() || fileName == "") {
        return false;
    }
    string baseName = RingtoneFileUtils::GetBaseNameFromPath(info.data);
    if (baseName.empty() || baseName == "") {
        return false;
    }
    string extensionName = RingtoneFileUtils::GetExtensionFromPath(info.data);

    int32_t repeatCount = 1;
    string srcPath = backupPath_ + info.data;
    while (RingtoneFileUtils::IsFileExists(destPath + "/" + fileName)) {
        // if (RingtoneFileUtils::IsSameFile(srcPath, destPath + "/" + fileName)) {
        //     RINGTONE_ERR_LOG("samefile: srcPath=%{public}s, dstPath=%{public}s", srcPath.c_str(),
        //         (destPath + "/" + fileName).c_str());
        //     return false;
        // }
        fileName = baseName + "(" + to_string(repeatCount++) + ")" + "." + extensionName;
    }
    info.restorePath = destPath + "/" + fileName;

    // if (!MoveFileInternal(srcPath, info.restorePath)) {
    //     return false;
    // }

    // UpdateRestoreFileInfo(info);

    return true;
}

void RingtoneRestore::OnFinished(vector<FileInfo> &infos)
{
    // if (!RingtoneFileUtils::RemoveDirectory(backupPath_)) {
    //     RINGTONE_ERR_LOG("cleanup backup dir failed, restorepath=%{public}s, err: %{public}s",
    //         backupPath_.c_str(), strerror(errno));
    // }
}
} // namespace Media
} // namespace OHOS
