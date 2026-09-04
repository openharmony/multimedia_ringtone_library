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

#ifndef OHOS_RINGTONE_DFX_MANAGER_H
#define OHOS_RINGTONE_DFX_MANAGER_H

#include "fa_ability_context.h"
#include "dfx_reporter.h"
#include "ringtone_type.h"

#define EXPORT __attribute__ ((visibility ("default")))

namespace OHOS {
namespace Media {
struct RingtoneCountInfo {
    int32_t presetNum;            // 预置铃声数量
    int32_t customNum;            // 自定义铃声总数
    int64_t custTotalSize;        // 自定义铃声总大小(字节)
    int32_t custAlarmNum;         // 自定义闹钟铃声数量
    int64_t custAlarmSize;        // 自定义闹钟铃声大小(字节)
    int32_t custContactAudioNum;  // 自定义联系人音频数量
    int32_t custContactVideoNum;  // 自定义联系人视频数量
    int64_t custContactSize;      // 自定义联系人铃声大小(字节)
    int32_t custAppNotifNum;      // 自定义应用通知数量
    int64_t custAppNotifSize;     // 自定义应用通知大小(字节)
    int32_t custNotifNum;         // 自定义通知数量
    int64_t custNotifSize;        // 自定义通知大小(字节)
    int32_t custRingtoneAudioNum; // 自定义来电音频数量
    int32_t custRingtoneVideoNum; // 自定义来电视频数量
    int64_t custRingtoneSize;    // 自定义来电铃声大小(字节)
    int64_t custVideoTotalSize;  // 所有自定义视频铃声大小(字节)
};

class DfxManager {
public:
    DfxManager();
    ~DfxManager();
    EXPORT static std::shared_ptr<DfxManager> GetInstance();
    EXPORT int64_t HandleReportXml();
    EXPORT int32_t Init(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context);
    EXPORT int64_t RequestTonesCount(SourceType type);
    EXPORT RingtoneCountInfo GetRingtoneCountInfo();

private:
    // 按铃声类型统计数量和大小的内部方法
    int64_t RequestTonesCountAndSize(SourceType type, ToneType toneType, int64_t &size, int mediaType = -1);
    int64_t RequestTonesCountOnly(SourceType type, ToneType toneType, int mediaType = -1);
    int64_t ScanDirectorySize(const std::string &path);  // 扫描目录计算总大小的内部方法
    static std::mutex instanceLock_;
    static std::shared_ptr<DfxManager> dfxManagerInstance_;
    std::atomic<bool> isInitSuccess_;
    std::shared_ptr<DfxReporter> dfxReporter_;
    std::shared_ptr<OHOS::AbilityRuntime::Context> context_ = nullptr;
};
} // namespace Media
} // namespace OHOS

#endif  // OHOS_RINGTONE_DFX_MANAGER_H
