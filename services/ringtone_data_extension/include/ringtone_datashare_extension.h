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

#ifndef FOUNDATION_ABILITYRUNTIME_OHOS_RINGTONE_COLUMN_DATASHARE_EXT_ABILITY_H
#define FOUNDATION_ABILITYRUNTIME_OHOS_RINGTONE_COLUMN_DATASHARE_EXT_ABILITY_H

#include "datashare_ext_ability.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace DataShare;
/**
 * @brief Basic datashare extension ability components.
 */
#define EXPORT __attribute__ ((visibility ("default")))
class RingtoneDataShareExtension : public DataShareExtAbility {
public:
    EXPORT RingtoneDataShareExtension(Runtime &runtime);
    EXPORT virtual ~RingtoneDataShareExtension() override;

    /**
     * @brief Create RingtoneDataShareExtension.
     *
     * @param runtime The runtime.
     * @return The RingtoneDataShareExtension instance.
     */
    EXPORT static RingtoneDataShareExtension *Create(const std::unique_ptr<Runtime> &runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    EXPORT void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this datashare extension ability is started. You must override this function if you want to
     *        perform some initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    EXPORT void OnStart(const AAFwk::Want &want) override;

    EXPORT void OnStop() override;

    /**
     * @brief Called when this datashare extension ability is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the datashare
     * extension.
     * @return Returns a pointer to the <b>sid</b> of the connected datashare extension ability.
     */
    EXPORT sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;
    /**
     * @brief Inserts a single data record into the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
     *
     * @return Returns the index of the inserted data record.
     */
    EXPORT int Insert(const Uri &uri, const DataShareValuesBucket &value) override;

    /**
     * @brief Deletes one or more data records from the database.
     *
     * @param uri Indicates the path of the data to operate.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the number of data records deleted.
     */
    EXPORT int Delete(const Uri &uri, const DataSharePredicates &predicates) override;

    /**
     * @brief Updates data records in the database.
     *
     * @param uri Indicates the path of data to update.
     * @param value Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     *
     * @return Returns the number of data records updated.
     */
    EXPORT int Update(const Uri &uri, const DataSharePredicates &predicates,
            const DataShareValuesBucket &value) override;

    /**
     * @brief query one or more data records from the database.
     *
     * @param uri Indicates the path of data to query.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
     * @param businessError Indicates errorcode and message.
     
     * @return Returns the query result.
     */
    EXPORT std::shared_ptr<DataShareResultSet> Query(const Uri &uri, const DataSharePredicates &predicates,
            std::vector<std::string> &columns, DatashareBusinessError &businessError) override;

    /**
     * @brief Opens a file in a specified remote path.
     *
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
     * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
     *  or "rwt" for read and write access that truncates any existing file.
     *
     * @return Returns the file descriptor.
     */
    EXPORT int OpenFile(const Uri &uri, const std::string &mode) override;
private:
    EXPORT static void DumpDataShareValueBucket(const std::vector<std::string> &tabFields,
        const DataShareValuesBucket &value);
    void RingtoneScanner();
    Runtime &runtime_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITYRUNTIME_OHOS_RINGTONE_COLUMN_DATASHARE_EXT_ABILITY_H
