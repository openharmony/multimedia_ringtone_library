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
#define MLOG_TAG "DfxManager"

#include "dfx_manager.h"

#include <filesystem>
#include "dfx_worker.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"
#include "result_set_utils.h"

namespace OHOS {
namespace Media {
using namespace std;

shared_ptr<RingtoneUnistore> g_dfxUnistore = nullptr;

shared_ptr<DfxManager> DfxManager::dfxManagerInstance_{nullptr};
mutex DfxManager::instanceLock_;

shared_ptr<DfxManager> DfxManager::GetInstance()
{
    if (dfxManagerInstance_ == nullptr) {
        lock_guard<mutex> lockGuard(instanceLock_);
        dfxManagerInstance_ = make_shared<DfxManager>();
    }
    return dfxManagerInstance_;
}

DfxManager::DfxManager() : isInitSuccess_(false)
{
}

DfxManager::~DfxManager()
{
}

int32_t DfxManager::Init(const shared_ptr<OHOS::AbilityRuntime::Context> &context)
{
    RINGTONE_INFO_LOG("Init DfxManager");
    if (context == nullptr) {
        return E_DB_FAIL;
    }
    if (g_dfxUnistore == nullptr) {
        g_dfxUnistore = RingtoneRdbStore::GetInstance(context);
        if (g_dfxUnistore == nullptr) {
            RINGTONE_ERR_LOG("RingtoneDataManager is not initialized");
            return E_DB_FAIL;
        }
    }
    dfxReporter_ = make_shared<DfxReporter>();
    DfxWorker::GetInstance()->Init();
    context_ = context;
    isInitSuccess_ = true;
    return E_OK;
}

int64_t DfxManager::RequestTonesCount(SourceType type)
{
    if (type > SOURCE_TYPE_CUSTOMISED || type < SOURCE_TYPE_PRESET) {
        RINGTONE_ERR_LOG("source type err, type=%{public}d", type);
        return 0;
    }

    Uri uri("");
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    cmd.GetAbsRdbPredicates()->EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, type);

    auto resultSet = g_dfxUnistore->Query(cmd, { RINGTONE_COLUMN_TONE_ID, RINGTONE_COLUMN_SOURCE_TYPE });
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("Failed to obtain file asset from database");
        return 0;
    }
    int32_t rowCount = 0;
    int32_t ret = resultSet->GetRowCount(rowCount);
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get row count");
        rowCount = 0;
    }
    return rowCount;
}

int64_t DfxManager::HandleReportXml()
{
    if (!isInitSuccess_) {
        RINGTONE_WARN_LOG("DfxManager not init");
        return RingtoneFileUtils::UTCTimeSeconds();
    }
    dfxReporter_->ReportDfxMessage();
    return RingtoneFileUtils::UTCTimeSeconds();
}

int64_t DfxManager::RequestTonesCountAndSize(SourceType type, ToneType toneType, int64_t &size, int mediaType)
{
    size = 0;
    if (type > SOURCE_TYPE_CUSTOMISED || type < SOURCE_TYPE_PRESET) {
        RINGTONE_ERR_LOG("source type err, type=%{public}d", type);
        return 0;
    }

    Uri uri("");
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    cmd.GetAbsRdbPredicates()->EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, type);
    if (toneType != TONE_TYPE_INVALID) {
        cmd.GetAbsRdbPredicates()->And();
        cmd.GetAbsRdbPredicates()->EqualTo(RINGTONE_COLUMN_TONE_TYPE, toneType);
    }
    if (mediaType >= 0) {
        cmd.GetAbsRdbPredicates()->And();
        cmd.GetAbsRdbPredicates()->EqualTo(RINGTONE_COLUMN_MEDIA_TYPE, mediaType);
    }

    auto resultSet = g_dfxUnistore->Query(cmd, { RINGTONE_COLUMN_TONE_ID, RINGTONE_COLUMN_SIZE });
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("Failed to obtain file asset from database");
        return 0;
    }
    int32_t rowCount = 0;
    int64_t totalSize = 0;
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        rowCount++;
        int64_t fileSize = GetInt64Val(RINGTONE_COLUMN_SIZE, resultSet);
        totalSize += fileSize;
    }
    size = totalSize;
    resultSet->Close();
    return rowCount;
}

int64_t DfxManager::RequestTonesCountOnly(SourceType type, ToneType toneType)
{
    if (type > SOURCE_TYPE_CUSTOMISED || type < SOURCE_TYPE_PRESET) {
        RINGTONE_ERR_LOG("source type err, type=%{public}d", type);
        return 0;
    }

    Uri uri("");
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    cmd.GetAbsRdbPredicates()->EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, type);
    if (toneType != TONE_TYPE_INVALID) {
        cmd.GetAbsRdbPredicates()->And();
        cmd.GetAbsRdbPredicates()->EqualTo(RINGTONE_COLUMN_TONE_TYPE, toneType);
    }

    auto resultSet = g_dfxUnistore->Query(cmd, { RINGTONE_COLUMN_TONE_ID });
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("Failed to obtain file asset from database");
        return 0;
    }
    int32_t rowCount = 0;
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        rowCount++;
    }
    resultSet->Close();
    return rowCount;
}

int64_t DfxManager::ScanDirectorySize(const std::string &path)
{
    int64_t totalSize = 0;
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        RINGTONE_WARN_LOG("ScanDirectorySize path not exists: %{public}s, ec=%{public}d", path.c_str(), ec.value());
        return 0;
    }
    for (const auto &entry : std::filesystem::directory_iterator(path, ec)) {
        if (ec.value() != 0) {
            RINGTONE_WARN_LOG("directory_iterator failed, ec=%{public}d", ec.value());
            break;
        }
        if (entry.is_regular_file(ec)) {
            totalSize += entry.file_size(ec);
        } else if (entry.is_directory(ec)) {
            totalSize += ScanDirectorySize(entry.path().string());
        }
    }
    return totalSize;
}

// 统计内容: 预置铃声数、自定义铃声总数、各类型(闹钟/联系人/通知/应用通知/来电)数量和大小
// 说明: 除CUST_VIDEO_TOTAL_SIZE外,其他Size字段均为实际物理磁盘大小
RingtoneCountInfo DfxManager::GetRingtoneCountInfo()
{
    RingtoneCountInfo info = {0};
    info.presetNum = RequestTonesCount(SourceType::SOURCE_TYPE_PRESET);
    info.customNum = RequestTonesCount(SourceType::SOURCE_TYPE_CUSTOMISED);
    info.custTotalSize = ScanDirectorySize(RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH);

    info.custAlarmNum = RequestTonesCountOnly(SourceType::SOURCE_TYPE_CUSTOMISED, TONE_TYPE_ALARM);
    info.custAlarmSize = ScanDirectorySize(RINGTONE_CUSTOMIZED_ALARM_PATH);

    info.custContactAudioNum = RequestTonesCountOnly(SourceType::SOURCE_TYPE_CUSTOMISED, TONE_TYPE_CONTACTS);
    info.custContactSize = ScanDirectorySize(RINGTONE_CUSTOMIZED_CONTACTS_PATH);

    info.custAppNotifNum = RequestTonesCountOnly(SourceType::SOURCE_TYPE_CUSTOMISED, TONE_TYPE_APP_NOTIFICATION);
    info.custAppNotifSize = ScanDirectorySize(RINGTONE_CUSTOMIZED_APP_NOTIFICATIONS_PATH);

    info.custNotifNum = RequestTonesCountOnly(SourceType::SOURCE_TYPE_CUSTOMISED, TONE_TYPE_NOTIFICATION);
    info.custNotifSize = ScanDirectorySize(RINGTONE_CUSTOMIZED_NOTIFICATIONS_PATH);

    info.custRingtoneAudioNum = RequestTonesCountOnly(SourceType::SOURCE_TYPE_CUSTOMISED, TONE_TYPE_RINGTONE);
    info.custRingtoneSize = ScanDirectorySize(RINGTONE_CUSTOMIZED_RINGTONE_PATH);

    // 来电视频统计(数据库中记录的大小)
    int64_t videoSize = 0;
    info.custRingtoneVideoNum = RequestTonesCountAndSize(SourceType::SOURCE_TYPE_CUSTOMISED, TONE_TYPE_RINGTONE,
        videoSize, RINGTONE_MEDIA_TYPE_VIDEO);
    info.custVideoTotalSize = videoSize;

    return info;
}
} // namespace Media
} // namespace OHOS
