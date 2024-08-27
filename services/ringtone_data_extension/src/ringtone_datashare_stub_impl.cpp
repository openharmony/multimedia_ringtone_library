/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define MLOG_TAG "Extension"

#include "ringtone_datashare_stub_impl.h"

#include "ringtone_log.h"

namespace OHOS {
namespace DataShare {
std::shared_ptr<RingtoneDataShareExtension> RingtoneDataShareStubImpl::GetOwner()
{
    return extension_;
}

std::vector<std::string> RingtoneDataShareStubImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    RINGTONE_DEBUG_LOG("begin.");
    std::vector<std::string> ret;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->GetFileTypes(uri, mimeTypeFilter);
    RINGTONE_INFO_LOG("end successfully.");
    return ret;
}

int RingtoneDataShareStubImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    int ret = -1;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->OpenFile(uri, mode);
    return ret;
}

int RingtoneDataShareStubImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    RINGTONE_DEBUG_LOG("begin.");
    int ret = -1;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->OpenRawFile(uri, mode);
    RINGTONE_INFO_LOG("end successfully. ret: %{public}d", ret);
    return ret;
}

int RingtoneDataShareStubImpl::Insert(const Uri &uri, const DataShareValuesBucket &value)
{
    int ret = 0;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->Insert(uri, value);
    return ret;
}

int RingtoneDataShareStubImpl::InsertExt(const Uri &uri, const DataShareValuesBucket &value, std::string &result)
{
    int ret = 0;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->InsertExt(uri, value, result);
    return ret;
}

int RingtoneDataShareStubImpl::Update(const Uri &uri, const DataSharePredicates &predicates,
    const DataShareValuesBucket &value)
{
    int ret = 0;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->Update(uri, predicates, value);
    return ret;
}

int RingtoneDataShareStubImpl::Delete(const Uri &uri, const DataSharePredicates &predicates)
{
    int ret = 0;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->Delete(uri, predicates);
    return ret;
}

std::shared_ptr<DataShareResultSet> RingtoneDataShareStubImpl::Query(const Uri &uri,
    const DataSharePredicates &predicates, std::vector<std::string> &columns, DatashareBusinessError &businessError)
{
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return nullptr;
    }
    return extension->Query(uri, predicates, columns, businessError);
}

std::string RingtoneDataShareStubImpl::GetType(const Uri &uri)
{
    RINGTONE_DEBUG_LOG("begin.");
    std::string ret = "";
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->GetType(uri);
    RINGTONE_INFO_LOG("end successfully.");
    return ret;
}

int RingtoneDataShareStubImpl::BatchInsert(const Uri &uri, const std::vector<DataShareValuesBucket> &values)
{
    RINGTONE_DEBUG_LOG("begin.");
    int ret = 0;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->BatchInsert(uri, values);
    RINGTONE_INFO_LOG("end successfully.");
    return ret;
}

bool RingtoneDataShareStubImpl::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    RINGTONE_DEBUG_LOG("begin.");
    bool ret = false;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->RegisterObserver(uri, dataObserver);
    RINGTONE_INFO_LOG("end successfully. ret: %{public}d", ret);
    return ret;
}

bool RingtoneDataShareStubImpl::UnregisterObserver(const Uri &uri,
    const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    RINGTONE_DEBUG_LOG("begin.");
    bool ret = false;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->UnregisterObserver(uri, dataObserver);
    RINGTONE_INFO_LOG("end successfully. ret: %{public}d", ret);
    return ret;
}

bool RingtoneDataShareStubImpl::NotifyChange(const Uri &uri)
{
    RINGTONE_DEBUG_LOG("begin.");
    bool ret = false;
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return ret;
    }
    ret = extension->NotifyChange(uri);
    RINGTONE_INFO_LOG("end successfully. ret: %{public}d", ret);
    return ret;
}

Uri RingtoneDataShareStubImpl::NormalizeUri(const Uri &uri)
{
    RINGTONE_DEBUG_LOG("begin.");
    Uri urivalue("");
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return urivalue;
    }
    urivalue = extension->NormalizeUri(uri);
    RINGTONE_INFO_LOG("end successfully.");
    return urivalue;
}

Uri RingtoneDataShareStubImpl::DenormalizeUri(const Uri &uri)
{
    RINGTONE_DEBUG_LOG("begin.");
    Uri urivalue("");
    auto client = sptr<RingtoneDataShareStubImpl>(this);
    auto extension = client->GetOwner();
    if (extension == nullptr) {
        RINGTONE_ERR_LOG("end failed.");
        return urivalue;
    }
    urivalue = extension->DenormalizeUri(uri);
    RINGTONE_INFO_LOG("end successfully.");
    return urivalue;
}
} // namespace DataShare
} // namespace OHOS
