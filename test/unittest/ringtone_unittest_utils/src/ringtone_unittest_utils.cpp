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

#include "ringtone_unittest_utils.h"

#include <fstream>

#include "ability_context_impl.h"
#include "abs_rdb_predicates.h"
#include "ringtone_data_manager.h"
#include "ringtone_db_const.h"
#include "ringtone_fetch_result.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_mimetype_utils.h"
#include "ringtone_scanner_utils.h"

using namespace std;
using namespace OHOS::DataShare;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace Media {

void RingtoneUnitTestUtils::Init()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    auto ret = RingtoneDataManager::GetInstance()->Init(abilityContextImpl);
    CHECK_AND_RETURN_LOG(ret == E_OK, "InitMediaLibraryMgr failed, ret: %{public}d", ret);
    isValid_ = true;
}

void RingtoneUnitTestUtils::InitRootDirs()
{
    system("mkdir /storage/cloud/files");
    system("mkdir /storage/cloud/files/Ringtone");
}

void RingtoneUnitTestUtils::CleanTestFiles()
{
    system("rm -rf /storage/cloud/files/");
}

bool RingtoneUnitTestUtils::CreateFileFS(const string &filePath)
{
    bool errCode = false;

    if (filePath.empty()) {
        return errCode;
    }

    ofstream file(filePath);
    if (!file) {
        RINGTONE_ERR_LOG("Output file path could not be created");
        return errCode;
    }

    const mode_t CHOWN_RW_UG = 0660;
    if (chmod(filePath.c_str(), CHOWN_RW_UG) == 0) {
        errCode = true;
    }

    file.close();

    return errCode;
}

void RingtoneUnitTestUtils::WaitForCallback(shared_ptr<TestRingtoneScannerCallback> callback)
{
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    const int waitSeconds = 10;
    callback->condVar_.wait_until(lock, std::chrono::system_clock::now() + std::chrono::seconds(waitSeconds));
}

TestRingtoneScannerCallback::TestRingtoneScannerCallback() : status_(-1) {}

int32_t TestRingtoneScannerCallback::OnScanFinished(const int32_t status, const std::string &uri,
    const std::string &path)
{
    status_ = status;
    condVar_.notify_all();
    return E_OK;
}
}
}
