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

#ifndef RINGTONE_COLUMN_DATA_MANAGER_H
#define RINGTONE_COLUMN_DATA_MANAGER_H

#include "datashare_values_bucket.h"
#include "rdb_utils.h"
#include "ringtone_asset.h"
#include "vibrate_asset.h"
#include "ringtone_data_command.h"
#include "ringtone_datashare_extension.h"

#define EXPORT __attribute__ ((visibility ("default")))
namespace OHOS {
namespace AbilityRuntime {
class RingtoneDataShareExtension;
}
namespace Media {
using OHOS::AbilityRuntime::RingtoneDataShareExtension;
class RingtoneDataManager {
public:
    EXPORT RingtoneDataManager();
    EXPORT ~RingtoneDataManager();
    EXPORT static std::shared_ptr<RingtoneDataManager> GetInstance();

    EXPORT int32_t Insert(RingtoneDataCommand &cmd, const DataShare::DataShareValuesBucket &value);
    EXPORT int32_t Delete(RingtoneDataCommand &cmd, const DataShare::DataSharePredicates &predicates,
        bool onlyDb = false);
    EXPORT int32_t Update(RingtoneDataCommand &cmd, const DataShare::DataShareValuesBucket &value,
        const DataShare::DataSharePredicates &predicates);
    EXPORT std::shared_ptr<DataShare::ResultSetBridge> Query(RingtoneDataCommand &cmd,
        const std::vector<std::string> &columns, const DataShare::DataSharePredicates &predicates, int &errCode);
    EXPORT int32_t OpenFile(RingtoneDataCommand &cmd, const std::string &mode);

    EXPORT int32_t Init(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context);
    EXPORT void ClearRingtoneDataMgr();
    EXPORT void SetOwner(const std::shared_ptr<RingtoneDataShareExtension> &datashareExtension);
    EXPORT std::shared_ptr<RingtoneDataShareExtension> GetOwner();
private:
    std::string GetIdFromUri(Uri &uri, const std::string &uriPath);
    std::shared_ptr<RingtoneAsset> GetRingtoneAssetFromId(const std::string &id);
    std::shared_ptr<VibrateAsset> GetVibrateAssetFromId(const std::string &id);
    int32_t DeleteFileFromResultSet(std::shared_ptr<NativeRdb::ResultSet> &resultSet);
    int32_t DeleteRingtoneRowById(int32_t toneId);
    int32_t OpenRingtoneFile(RingtoneDataCommand &cmd, const std::string &mode);
    int32_t OpenVibrateFile(RingtoneDataCommand &cmd, const std::string &mode);

    std::shared_mutex mgrSharedMutex_;
    std::shared_ptr<OHOS::AbilityRuntime::Context> context_ = nullptr;
    static std::mutex mutex_;
    static std::shared_ptr<RingtoneDataManager> instance_;
    std::atomic<int> refCnt_ {0};
    std::shared_ptr<RingtoneDataShareExtension> extension_ = nullptr;
};
} // namespace Media
} // namespace OHOS
#endif // RINGTONE_COLUMN_DATA_MANAGER_H
