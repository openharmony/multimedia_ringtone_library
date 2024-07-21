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
#define MLOG_TAG "RdbStore"

#include "ringtone_rdbstore.h"

#include <sys/stat.h>

#include "rdb_sql_utils.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_tracer.h"
#include "result_set_utils.h"
#include "ringtone_rdb_callbacks.h"

namespace OHOS::Media {
using namespace std;
using namespace OHOS;

shared_ptr<NativeRdb::RdbStore> RingtoneRdbStore::rdbStore_;

shared_ptr<RingtoneUnistore> RingtoneRdbStore::GetInstance(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context)
{
    static shared_ptr<RingtoneRdbStore> instance = nullptr;
    if (instance == nullptr && context == nullptr) {
        RINGTONE_ERR_LOG("RingtoneRdbStore is not initialized");
        return nullptr;
    }
    if (instance == nullptr) {
        instance = make_shared<RingtoneRdbStore>(context);
        if (instance->Init() != 0) {
            RINGTONE_ERR_LOG("init RingtoneRdbStore failed");
            instance = nullptr;
            return instance;
        }
    }
    return instance;
}

RingtoneRdbStore::RingtoneRdbStore(const shared_ptr<OHOS::AbilityRuntime::Context> &context)
{
    if (context == nullptr) {
        RINGTONE_ERR_LOG("Failed to get context");
        return;
    }
    string databaseDir = context->GetDatabaseDir();
    string name = RINGTONE_LIBRARY_DB_NAME;
    int32_t errCode = 0;
    string realPath = NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(databaseDir, name, errCode);
    config_.SetName(move(name));
    config_.SetPath(move(realPath));
    config_.SetBundleName(context->GetBundleName());
    config_.SetArea(context->GetArea());
    config_.SetSecurityLevel(NativeRdb::SecurityLevel::S3);
}

int32_t RingtoneRdbStore::Init()
{
    RINGTONE_INFO_LOG("Init rdb store");
    if (rdbStore_ != nullptr) {
        return E_OK;
    }

    int32_t errCode = 0;
    RingtoneDataCallBack rdbDataCallBack;
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(config_, RINGTONE_RDB_VERSION, rdbDataCallBack, errCode);
    if (rdbStore_ == nullptr) {
        RINGTONE_ERR_LOG("GetRdbStore is failed , errCode=%{public}d", errCode);
        return E_ERR;
    }
    RINGTONE_INFO_LOG("SUCCESS");
    return E_OK;
}

RingtoneRdbStore::~RingtoneRdbStore() = default;

void RingtoneRdbStore::Stop()
{
    if (rdbStore_ == nullptr) {
        return;
    }

    rdbStore_ = nullptr;
}

int32_t RingtoneRdbStore::Insert(RingtoneDataCommand &cmd, int64_t &rowId)
{
    RingtoneTracer tracer;
    tracer.Start("RingtoneRdbStore::Insert");
    if (rdbStore_ == nullptr) {
        RINGTONE_ERR_LOG("Pointer rdbStore_ is nullptr. Maybe it didn't init successfully.");
        return E_HAS_DB_ERROR;
    }

    int32_t ret = rdbStore_->Insert(rowId, cmd.GetTableName(), cmd.GetValueBucket());
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("rdbStore_->Insert failed, ret = %{public}d", ret);
        return E_HAS_DB_ERROR;
    }

    RINGTONE_DEBUG_LOG("rdbStore_->Insert end, rowId = %d, ret = %{public}d", (int)rowId, ret);
    return ret;
}

int32_t RingtoneRdbStore::Delete(RingtoneDataCommand &cmd, int32_t &deletedRows)
{
    if (rdbStore_ == nullptr) {
        RINGTONE_ERR_LOG("Pointer rdbStore_ is nullptr. Maybe it didn't init successfully.");
        return E_HAS_DB_ERROR;
    }
    RingtoneTracer tracer;
    tracer.Start("RdbStore->DeleteByCmd");

    auto predicates = cmd.GetAbsRdbPredicates();
    int32_t ret = rdbStore_->Delete(deletedRows, cmd.GetTableName(), predicates->GetWhereClause(),
        predicates->GetWhereArgs());
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("rdbStore_->Delete failed, ret = %{public}d", ret);
        return E_HAS_DB_ERROR;
    }
    return ret;
}

int32_t RingtoneRdbStore::Update(RingtoneDataCommand &cmd, int32_t &changedRows)
{
    if (rdbStore_ == nullptr) {
        RINGTONE_ERR_LOG("rdbStore_ is nullptr");
        return E_HAS_DB_ERROR;
    }

    RingtoneTracer tracer;
    tracer.Start("RdbStore->UpdateByCmd");
    int32_t ret = rdbStore_->Update(changedRows, cmd.GetTableName(), cmd.GetValueBucket(),
        cmd.GetAbsRdbPredicates()->GetWhereClause(), cmd.GetAbsRdbPredicates()->GetWhereArgs());
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("rdbStore_->Update failed, ret = %{public}d", ret);
        return E_HAS_DB_ERROR;
    }
    return ret;
}

shared_ptr<NativeRdb::ResultSet> RingtoneRdbStore::Query(RingtoneDataCommand &cmd,
    const vector<string> &columns)
{
    if (rdbStore_ == nullptr) {
        RINGTONE_ERR_LOG("rdbStore_ is nullptr");
        return nullptr;
    }

    RingtoneTracer tracer;
    tracer.Start("RdbStore->QueryByCmd");

    auto resultSet = rdbStore_->Query(*cmd.GetAbsRdbPredicates(), columns);
    return resultSet;
}

int32_t RingtoneRdbStore::ExecuteSql(const string &sql)
{
    if (rdbStore_ == nullptr) {
        RINGTONE_ERR_LOG("Pointer rdbStore_ is nullptr. Maybe it didn't init successfully.");
        return E_HAS_DB_ERROR;
    }
    RingtoneTracer tracer;
    tracer.Start("RdbStore->ExecuteSql");
    int32_t ret = rdbStore_->ExecuteSql(sql);
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("rdbStore_->ExecuteSql failed, ret = %{public}d", ret);
        return E_HAS_DB_ERROR;
    }
    return ret;
}

shared_ptr<NativeRdb::ResultSet> RingtoneRdbStore::QuerySql(const string &sql, const vector<string> &selectionArgs)
{
    if (rdbStore_ == nullptr) {
        RINGTONE_ERR_LOG("Pointer rdbStore_ is nullptr. Maybe it didn't init successfully.");
        return nullptr;
    }

    RingtoneTracer tracer;
    tracer.Start("RdbStore->QuerySql");
    auto resultSet = rdbStore_->QuerySql(sql, selectionArgs);
    return resultSet;
}

shared_ptr<NativeRdb::RdbStore> RingtoneRdbStore::GetRaw()
{
    return rdbStore_;
}
} // namespace OHOS::Media
