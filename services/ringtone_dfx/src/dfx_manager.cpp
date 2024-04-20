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

#include "dfx_worker.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"

using namespace std;

namespace OHOS {
namespace Media {
RingtoneUnistore *g_dfxUnistore_ = nullptr;

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
    if (g_dfxUnistore_ == nullptr) {
        g_dfxUnistore_ = RingtoneRdbStore::GetInstance(context);
        if (g_dfxUnistore_ == nullptr) {
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

    auto resultSet = g_dfxUnistore_->Query(cmd, { RINGTONE_COLUMN_TONE_ID, RINGTONE_COLUMN_SOURCE_TYPE });
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
} // namespace Media
} // namespace OHOS
