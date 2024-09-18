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

#ifndef RINGTONE_DUALFWK_RESTORE_H
#define RINGTONE_DUALFWK_RESTORE_H

#include "datashare_helper.h"
#include "dualfwk_sound_setting.h"
#include "rdb_helper.h"
#include "result_set.h"
#include "ringtone_metadata.h"
#include "ringtone_restore_base.h"
#include "ringtone_restore_type.h"
#include "medialibrary_db_const.h"
#include "userfilemgr_uri.h"

namespace OHOS {
namespace Media {
class RingtoneDualFwkRestore : public RingtoneRestoreBase {
public:
    RingtoneDualFwkRestore() = default;
    virtual ~RingtoneDualFwkRestore() = default;
    int32_t Init(const std::string &backupPath) override;
    int32_t StartRestore() override;
protected:
    std::unique_ptr<DualFwkSoundSetting> dualFwkSetting_ = nullptr;
private:
    virtual bool OnPrepare(FileInfo &info, const std::string &destPath) override;
    virtual void OnFinished(std::vector<FileInfo> &infos) override;
    virtual int32_t LoadDualFwkConf(const std::string &backupPath) override;
    int32_t ParseDualFwkConf(const std::string &xml);
    int32_t DupToneFile(FileInfo &info);
    void UpdateRestoreFileInfo(FileInfo &info);
    std::vector<FileInfo> BuildFileInfo();
    int32_t QueryMediaLibForFileInfo(const std::vector<std::string> &names,
        std::map<std::string, std::shared_ptr<FileInfo>> &infoMap,
        const std::string &queryFileUriBase = UFM_QUERY_AUDIO,
        const std::string &predicateColumn = MEDIA_DATA_DB_NAME);
    int32_t QueryRingToneDbForFileInfo(std::shared_ptr<NativeRdb::RdbStore> rdbStore,
        const std::vector<std::string> &names, std::map<std::string, std::shared_ptr<FileInfo>> &infoMap,
        const std::string &predicateColumn = MEDIA_DATA_DB_NAME);

private:
    std::shared_ptr<DataShare::DataShareHelper> mediaDataShare_ = nullptr;
    std::string dualFwkConf_ = {};
};

class RingtoneDualFwkRestoreClone : public RingtoneDualFwkRestore {
private:
    virtual int32_t LoadDualFwkConf(const std::string &backupPath) override;
};
} // namespace Media
} // namespace OHOS
#endif  // RINGTONE_DUALFWK_RESTORE_H
