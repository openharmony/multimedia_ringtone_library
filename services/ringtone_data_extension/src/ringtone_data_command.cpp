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

#include "ringtone_data_command.h"

#include "ringtone_errno.h"
#include "ringtone_log.h"

using namespace std;
using namespace OHOS::NativeRdb;
using namespace OHOS::DataShare;

namespace OHOS {
namespace Media {

RingtoneDataCommand::RingtoneDataCommand(const Uri &uri, const string &table, const RingtoneOperationType type)
    : uri_(uri), tableName_(table), oprnType_(type), toneId_(-1)
{
    toneId_ = GetToneIdFromUri(uri_);
}

RingtoneDataCommand::~RingtoneDataCommand() {}

// set functions
void RingtoneDataCommand::SetDataSharePred(const DataShare::DataSharePredicates &pred)
{
    datasharePred_ = make_unique<const DataSharePredicates>(pred);
}

void RingtoneDataCommand::SetValueBucket(const NativeRdb::ValuesBucket &value)
{
    insertValue_ = value;
}

void RingtoneDataCommand::SetBundleName(const string &bundleName)
{
    bundleName_ = bundleName;
}

void RingtoneDataCommand::SetResult(const string &result)
{
    result_ = result;
}

// get functions
ValuesBucket &RingtoneDataCommand::GetValueBucket()
{
    return insertValue_;
}

AbsRdbPredicates *RingtoneDataCommand::GetAbsRdbPredicates()
{
    if (absRdbPredicates_ == nullptr) {
        absRdbPredicates_ = make_unique<AbsRdbPredicates>(tableName_);
    }
    return absRdbPredicates_.get();
}

const string &RingtoneDataCommand::GetTableName()
{
    return tableName_;
}

const Uri &RingtoneDataCommand::GetUri() const
{
    return uri_;
}

const string &RingtoneDataCommand::GetBundleName()
{
    return bundleName_;
}

const string &RingtoneDataCommand::GetResult()
{
    return result_;
}

static bool IsNumber(const string &str)
{
    if (str.empty()) {
        RINGTONE_ERR_LOG("IsNumber input is empty ");
        return false;
    }

    for (char const &c : str) {
        if (isdigit(c) == 0) {
            return false;
        }
    }
    return true;
}

int32_t RingtoneDataCommand::GetToneIdFromUri(Uri &uri)
{
    string uriStr = uri.ToString();
    if (uriStr.empty()) {
        return E_INVALID_URI;
    }

    auto toneId = uriStr.substr(RINGTONE_PATH_URI.size(), uriStr.size());
    if (IsNumber(toneId)) {
        RINGTONE_INFO_LOG("Get toneId=%{public}s from Uri", toneId.c_str());
        return stoi(toneId);
    }

    return E_INVALID_URI;
}

RingtoneOperationType RingtoneDataCommand::GetOprnType() const
{
    return oprnType_;
}
} // namespace Media
} // namespace OHOS
