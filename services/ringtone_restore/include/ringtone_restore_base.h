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

#ifndef RINGTONE_RESTORE_BASE_H
#define RINGTONE_RESTORE_BASE_H

#include "restore_interface.h"
#include "ringtone_restore_type.h"
#include "ringtone_rdb_transaction.h"
#include "ringtone_setting_manager.h"
#include "simcard_setting_asset.h"

namespace OHOS {
namespace Media {
class RingtoneRestoreBase : public RestoreInterface {
public:
    RingtoneRestoreBase() = default;
    virtual ~RingtoneRestoreBase() = default;
    int32_t Init(const std::string &backupPath) override;
    int32_t StartRestore() override;
    virtual std::shared_ptr<NativeRdb::RdbStore> GetBaseDb() override
    {
        return localRdb_;
    }
    static std::string GetRestoreDir(const int32_t toneType);
protected:
    virtual bool OnPrepare(FileInfo &info, const std::string &destPath) = 0;
    virtual void OnFinished(std::vector<FileInfo> &fileInfos) = 0;
    virtual std::vector<NativeRdb::ValuesBucket> MakeInsertValues(std::vector<FileInfo> &infos);
    virtual int32_t InsertTones(std::vector<FileInfo> &infos);
    virtual int32_t LoadDualFwkConf(const std::string &backupPath) { return -1; }
    virtual void CheckSetting(FileInfo &info);
    void CheckShotSetting(FileInfo &info);
    void CheckRingtoneSetting(FileInfo &info);
    static bool MoveFile(const std::string &src, const std::string &dst);
    static int32_t MoveDirectory(const std::string &srcDir, const std::string &dstDir);
    void ExtractMetaFromColumn(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
        std::unique_ptr<RingtoneMetadata> &metadata, const std::string &col);
    int32_t PopulateMetadata(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
        std::unique_ptr<RingtoneMetadata> &metaData);
    virtual void FlushSettings();
    bool DetermineNoRingtone(const std::string &typeColumn, const std::string &sourceColumn,
        int type, int allSetType, const std::shared_ptr<NativeRdb::RdbStore> &rdbStore);
    bool NeedCommitSetting(const std::string &typeColumn, const std::string &sourceColumn,
        int type, int allSetType);
    void SetNotRingtone(const std::string &typeColumn, const std::string &sourceColumn, int32_t simCard);
    void UpdateSettingTable(const SimcardSettingAsset &asset);
private:
    void CheckUpdateVibrateSetting(const FileInfo &info);
    std::string QuerySingleColumn(const std::string &dataColumn,
        const NativeRdb::AbsRdbPredicates &predicates);
    static NativeRdb::ValuesBucket SetInsertValue(const FileInfo &fileInfo);
    int32_t BatchInsert(const std::string &tableName, std::vector<NativeRdb::ValuesBucket> &values, int64_t &rowNum);
    std::shared_ptr<NativeRdb::RdbStore> localRdb_ = nullptr;
    std::unique_ptr<RingtoneSettingManager> settingMgr_ = nullptr;
};
} // namespace Media
} // namespace OHOS

#endif  // RINGTONE_RESTORE_BASE_H
