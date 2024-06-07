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
#define MLOG_TAG "DfxReporter"

#include "dfx_reporter.h"

#include "dfx_const.h"
#include "dfx_manager.h"
#include "hisysevent.h"
#include "preferences_helper.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"

namespace OHOS {
namespace Media {
using namespace std;
static constexpr char RINGTONE_LIBRARY[] = "RINGTONE_LIBRARY";

DfxReporter::DfxReporter()
{
}

DfxReporter::~DfxReporter()
{
}

void DfxReporter::ReportDfxMessage()
{
    int32_t errCode;
    shared_ptr<NativePreferences::Preferences> prefs =
        NativePreferences::PreferencesHelper::GetPreferences(DFX_COMMON_XML, errCode);
    if (!prefs) {
        RINGTONE_ERR_LOG("get preferences error: %{public}d", errCode);
        return;
    }
    std::string date = RingtoneFileUtils::StrCreateTimeByMilliseconds(DATE_FORMAT,
        RingtoneFileUtils::UTCTimeMilliSeconds());
    int64_t presetNum = DfxManager::GetInstance()->RequestTonesCount(SourceType::SOURCE_TYPE_PRESET);
    int64_t customNum = DfxManager::GetInstance()->RequestTonesCount(SourceType::SOURCE_TYPE_CUSTOMISED);
    RINGTONE_INFO_LOG("presetNum:%{public}" PRId64 ", customNum:%{public}" PRId64, presetNum, customNum);

    int ret = HiSysEventWrite(
        RINGTONE_LIBRARY,
        "RINGTONELIB_DFX_MESSAGE",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "DATE", date,
        "PRESET_NUM", presetNum,
        "CUSTOM_NUM", customNum);
    if (ret != 0) {
        RINGTONE_ERR_LOG("ReportDfxMessage error:%{public}d", ret);
    }
    prefs->FlushSync();
}
} // namespace Media
} // namespace OHOS
