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

#ifndef RINGTONE_RESTORE_H
#define RINGTONE_RESTORE_H

#include "rdb_helper.h"
#include "result_set.h"
#include "ringtone_metadata.h"
#include "ringtone_restore_base.h"

namespace OHOS {
namespace Media {
class RingtoneRestore final : public RingtoneRestoreBase {
public:
    RingtoneRestore() = default;
    virtual ~RingtoneRestore() = default;
    int32_t Init(const std::string &backupPath) override;
    int32_t StartRestore() override;
protected:
    virtual bool OnPrepare(FileInfo &info, const std::string &destPath) override;
    virtual void OnFinished(std::vector<FileInfo> &infos) override;
private:
    void CheckRestoreFileInfos(std::vector<FileInfo> &infos);
    std::vector<FileInfo> QueryFileInfos(int32_t offset);
    std::vector<FileInfo> ConvertToFileInfos(std::vector<std::shared_ptr<RingtoneMetadata>> &metaDatas);
    void UpdateRestoreFileInfo(FileInfo &info);
    void CheckNotRingtoneRestore();
    void UpdateSettingInfos();
private:
    std::shared_ptr<NativeRdb::RdbStore> restoreRdb_ = nullptr;
    std::string backupPath_ = {};
    std::string dbPath_ = {};
};
} // namespace Media
} // namespace OHOS

#endif  // RINGTONE_RESTORE_H
