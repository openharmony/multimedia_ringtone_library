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

#ifndef RINGTONE_DUALFW_RESTORE_H
#define RINGTONE_DUALFW_RESTORE_H

#include "datashare_helper.h"
#include "dualfw_sound_setting.h"
#include "rdb_helper.h"
#include "result_set.h"
#include "ringtone_metadata.h"
#include "ringtone_restore_base.h"
#include "ringtone_restore_type.h"

namespace OHOS {
namespace Media {
class RingtoneDualfwRestore final : public RingtoneRestoreBase {
public:
    RingtoneDualfwRestore() = default;
    virtual ~RingtoneDualfwRestore() = default;
    int32_t Init(const std::string &backupPath) override;
    void StartRestore() override;
protected:
    virtual bool OnPrepare(FileInfo &info, const std::string &destPath) override;
    virtual void OnFinished(std::vector<FileInfo> &infos) override;
private:
    int32_t ParseDualfwConf(std::string &xml);
    int32_t DupToneFile(FileInfo &info);
    void UpdateRestoreFileInfo(FileInfo &info);
private:
    std::shared_ptr<DataShare::DataShareHelper> mediaDataShare_ = nullptr;
    std::unique_ptr<DualfwSoundSetting> dualfwSetting_ = nullptr;
    std::string dualfwConf_ = {};
};
} // namespace Media
} // namespace OHOS

#endif  // RINGTONE_DUALFW_RESTORE_H
