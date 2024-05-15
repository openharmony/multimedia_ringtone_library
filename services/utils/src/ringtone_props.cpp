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
#define MLOG_TAG "RingtoneProps"

#include "ringtone_props.h"

#include "rdb_store.h"
#include "result_set_utils.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"

namespace OHOS::Media {
using namespace std;
using namespace OHOS;
const std::string CREATE_PROPERTIES_TABLE = "CREATE TABLE IF NOT EXISTS " + PROPERTIES_TABLE + "(" +
    RINGTONEPROPS_COLUMN_ID                       + " INTEGER  PRIMARY KEY AUTOINCREMENT, " +
    RINGTONEPROPS_COLUMN_NAME                     + " TEXT NOT NULL UNIQUE, " +
    RINGTONEPROPS_COLUMN_VALUE                    + " TEXT DEFAULT NULL" + ")";

RingtoneProps::RingtoneProps(shared_ptr<NativeRdb::RdbStore> rdb) : store_(rdb)
{
}

int32_t RingtoneProps::Init()
{
    if (store_ == nullptr) {
        RINGTONE_ERR_LOG("store_ is nullptr");
        return E_HAS_DB_ERROR;
    }
    if (store_->ExecuteSql(CREATE_PROPERTIES_TABLE) != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("Failed to execute sql");
        return E_HAS_DB_ERROR;
    }
    return E_OK;
}

int32_t RingtoneProps::GetPropFromResultSet(shared_ptr<NativeRdb::ResultSet> resultSet, string &propVal)
{
    if (resultSet == nullptr) {
        RINGTONE_INFO_LOG("invalid argument");
        return E_INVALID_ARGUMENTS;
    }
    auto ret = resultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("go to first row err");
        return E_HAS_DB_ERROR;
    }
    propVal = std::get<string>(ResultSetUtils::GetValFromColumn<shared_ptr<NativeRdb::ResultSet>>(
        RINGTONEPROPS_COLUMN_VALUE, resultSet, RingtoneResultSetDataType::DATA_TYPE_STRING));

    return E_OK;
}

string RingtoneProps::GetProp(const string &propName, const string &defaultVal)
{
    if (store_ == nullptr) {
        RINGTONE_ERR_LOG("store_ is nullptr");
        return defaultVal;
    }

    const string queryPropsSql = "SELECT " + RINGTONEPROPS_COLUMN_VALUE + " FROM " + PROPERTIES_TABLE + " WHERE " +
        RINGTONEPROPS_COLUMN_NAME + " = \"" + propName + "\"";

    auto resultSet = store_->QuerySql(queryPropsSql);
    if (resultSet == nullptr) {
        RINGTONE_INFO_LOG("resultset is null");
        return defaultVal;
    }

    int32_t count = 0;
    int32_t nativeRdbRet = resultSet->GetRowCount(count);
    if (count <= 0 || nativeRdbRet != NativeRdb::E_OK) {
        RINGTONE_INFO_LOG("resultset row count is zero, count=%{public}d", count);
        return defaultVal;
    }

    string retVal = {};
    int32_t ret = GetPropFromResultSet(resultSet, retVal);
    if (ret != E_OK) {
        RINGTONE_INFO_LOG("get pro err");
        return defaultVal;
    }

    return retVal;
}

bool RingtoneProps::SetProp(const string &propName, const string &propVal)
{
    if (store_ == nullptr) {
        RINGTONE_ERR_LOG("store_ is nullptr");
        return false;
    }

    const string queryPropsSql = "SELECT " + RINGTONEPROPS_COLUMN_VALUE + " FROM " + PROPERTIES_TABLE + " WHERE " +
        RINGTONEPROPS_COLUMN_NAME + " = \"" + propName + "\"";

    const string insertPropsSql = "INSERT INTO " + PROPERTIES_TABLE + "(" +  RINGTONEPROPS_COLUMN_NAME + ", " +
        RINGTONEPROPS_COLUMN_VALUE + ")" + " VALUES (\"" +  propName + "\", \"" + propVal + "\"" + ")";

    const string updatePropsSql = "UPDATE RingtoneProps SET " + RINGTONEPROPS_COLUMN_VALUE + " = \"" + propVal + "\"" +
        " WHERE " + RINGTONEPROPS_COLUMN_NAME + " = \"" + propName + "\"";

    auto resultSet = store_->QuerySql(queryPropsSql);

    int32_t ret = NativeRdb::E_OK;
    if (resultSet != nullptr) {
        int32_t count = 0;
        auto internalRet = resultSet->GetRowCount(count);
        if (count <= 0 || internalRet != NativeRdb::E_OK) {
            ret = store_->ExecuteSql(insertPropsSql);
        } else {
            ret = store_->ExecuteSql(updatePropsSql);
        }
    } else {
        ret = store_->ExecuteSql(insertPropsSql);
    }

    return (ret != NativeRdb::E_OK) ? false : true;
}
} // namespace OHOS::Media
