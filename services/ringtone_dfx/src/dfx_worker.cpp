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
#define MLOG_TAG "DfxWorker"

#include "dfx_worker.h"

#include "dfx_const.h"
#include "dfx_manager.h"
#include "parameters.h"
#include "preferences_helper.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"

namespace OHOS {
namespace Media {
using namespace std;
shared_ptr<DfxWorker> DfxWorker::dfxWorkerInstance_{nullptr};

shared_ptr<DfxWorker> DfxWorker::GetInstance()
{
    if (dfxWorkerInstance_ == nullptr) {
        dfxWorkerInstance_ = make_shared<DfxWorker>();
    }
    return dfxWorkerInstance_;
}

DfxWorker::DfxWorker()
{
    longTimeSec_ = stoi(system::GetParameter("persist.multimedia.ringtonelibrary.dfx.longtime", ONE_WEEK)) *
        ONE_MINUTE * ONE_MINUTE;
}

DfxWorker::~DfxWorker()
{
}

void DfxWorker::Init()
{
    RINGTONE_INFO_LOG("init");
    thread(bind(&DfxWorker::InitCycleThread, this)).detach();
}

void DfxWorker::InitCycleThread()
{
    RINGTONE_INFO_LOG("DFX start");
    int32_t errCode;
    shared_ptr<NativePreferences::Preferences> prefs =
        NativePreferences::PreferencesHelper::GetPreferences(DFX_COMMON_XML, errCode);
    if (!prefs) {
        RINGTONE_ERR_LOG("get preferences error: %{public}d", errCode);
        return;
    }
    lastReportTime_ = prefs->GetLong(LAST_REPORT_TIME, 0);
    if (RingtoneFileUtils::UTCTimeSeconds() - lastReportTime_ > longTimeSec_) {
        RINGTONE_INFO_LOG("Report Xml");
        lastReportTime_ = DfxManager::GetInstance()->HandleReportXml();
        prefs->PutLong(LAST_REPORT_TIME, lastReportTime_);
        prefs->FlushSync();
    }
    RINGTONE_INFO_LOG("DFX end");
}
} // namespace Media
} // namespace OHOS
