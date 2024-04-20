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

#ifndef RINGTONE_COLUMN_DATASHARE_STUB_IMPL_H
#define RINGTONE_COLUMN_DATASHARE_STUB_IMPL_H

#include "datashare_stub.h"
#include "ringtone_datashare_extension.h"

namespace OHOS {
namespace DataShare {
using DataShare::RingtoneDataShareExtension;
#define EXPORT __attribute__ ((visibility ("default")))
class EXPORT RingtoneDataShareStubImpl : public DataShareStub {
public:
    EXPORT explicit RingtoneDataShareStubImpl(const std::shared_ptr<RingtoneDataShareExtension>& extension,
        napi_env env) : extension_(extension) {}

    EXPORT virtual ~RingtoneDataShareStubImpl() {}

    EXPORT std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override;

    EXPORT int OpenFile(const Uri &uri, const std::string &mode) override;

    EXPORT int Insert(const Uri &uri, const DataShareValuesBucket &value) override;

    EXPORT int InsertExt(const Uri &uri, const DataShareValuesBucket &value, std::string &result) override;

    EXPORT int Update(const Uri &uri, const DataSharePredicates &predicates,
        const DataShareValuesBucket &value) override;

    EXPORT int Delete(const Uri &uri, const DataSharePredicates &predicates) override;

    EXPORT std::shared_ptr<DataShareResultSet> Query(const Uri &uri, const DataSharePredicates &predicates,
        std::vector<std::string> &columns, DatashareBusinessError &businessError) override;

    EXPORT int OpenRawFile(const Uri &uri, const std::string &mode) override;

    EXPORT std::string GetType(const Uri &uri) override;

    EXPORT int BatchInsert(const Uri &uri, const std::vector<DataShareValuesBucket> &values) override;

    EXPORT bool RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override;

    EXPORT bool UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override;

    EXPORT bool NotifyChange(const Uri &uri) override;

    EXPORT Uri NormalizeUri(const Uri &uri) override;

    EXPORT Uri DenormalizeUri(const Uri &uri) override;

private:
    EXPORT std::shared_ptr<RingtoneDataShareExtension> GetOwner();

private:
    std::shared_ptr<RingtoneDataShareExtension> extension_;
};
} // namespace Media_DataShare
} // namespace OHOS
#endif // RINGTONE_COLUMN_DATASHARE_STUB_IMPL_H

