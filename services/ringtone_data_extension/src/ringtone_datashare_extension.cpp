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
#define MLOG_TAG "ringtone_Extension"

#include "ringtone_datashare_extension.h"

#include "app_mgr_client.h"
#include "datashare_ext_ability_context.h"
#include "dfx_manager.h"
#include "parameter.h"
#include "permission_utils.h"
#include "ipc_skeleton.h"
#include "ringtone_data_manager.h"
#include "ringtone_datashare_stub_impl.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_scanner_manager.h"
#include "runtime.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace std;
using namespace OHOS::AppExecFwk;
using namespace OHOS::NativeRdb;
using namespace OHOS::Media;
using namespace OHOS::DataShare;

const char RINGTONE_PARAMETER_SCANNER_COMPLETED_KEY[] = "ringtone.scanner.completed";
const char RINGTONE_PARAMETER_SCANNER_COMPLETED_TRUE[] = "1";
const char RINGTONE_PARAMETER_SCANNER_COMPLETED_FALSE[] = "0";
const int32_t SYSPARA_SIZE = 64;
const string RINGTONE_SINGLE_CLONE_BACKUP_PATH = "/storage/media/local/files/Backup";
const std::vector<std::string> RINGTONE_OPEN_WRITE_MODE_VECTOR = {
    { RINGTONE_FILEMODE_WRITEONLY },
    { RINGTONE_FILEMODE_READWRITE },
    { RINGTONE_FILEMODE_WRITETRUNCATE },
    { RINGTONE_FILEMODE_WRITEAPPEND },
    { RINGTONE_FILEMODE_READWRITETRUNCATE },
    { RINGTONE_FILEMODE_READWRITEAPPEND },
};

RingtoneDataShareExtension *RingtoneDataShareExtension::Create(const unique_ptr<Runtime> &runtime)
{
    return new RingtoneDataShareExtension(static_cast<Runtime&>(*runtime));
}

RingtoneDataShareExtension::RingtoneDataShareExtension(Runtime &runtime) : DataShareExtAbility(), runtime_(runtime) {}

RingtoneDataShareExtension::~RingtoneDataShareExtension()
{
}

void RingtoneDataShareExtension::Init(const shared_ptr<AbilityLocalRecord> &record,
    const shared_ptr<OHOSApplication> &application, shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    if ((record == nullptr) || (application == nullptr) || (handler == nullptr) || (token == nullptr)) {
        RINGTONE_ERR_LOG("RingtoneDataShareExtension::init failed, some object is nullptr");
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->KillApplicationSelf();
        return;
    }
    DataShareExtAbility::Init(record, application, handler, token);
}

void RingtoneDataShareExtension::OnStart(const AAFwk::Want &want)
{
    RINGTONE_INFO_LOG("begin.");
    Extension::OnStart(want);
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        RINGTONE_ERR_LOG("Failed to get context");
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->KillApplicationSelf();
        return;
    }
    RINGTONE_INFO_LOG("runtime language %{public}d", runtime_.GetLanguage());
    auto dataManager = RingtoneDataManager::GetInstance();
    if (dataManager == nullptr) {
        RINGTONE_ERR_LOG("Failed to get dataManager");
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->KillApplicationSelf();
        return;
    }
    int32_t ret = dataManager->Init(context);
    if (ret != Media::E_OK) {
        RINGTONE_ERR_LOG("Failed to init RingtoneData Mgr");
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->KillApplicationSelf();
        return;
    }
    dataManager->SetOwner(static_pointer_cast<RingtoneDataShareExtension>(shared_from_this()));
    auto dfxMgr = DfxManager::GetInstance();
    dfxMgr->Init(context);

    // ringtone scan
    char scanCompletedValue[SYSPARA_SIZE] = {0};
    GetParameter(RINGTONE_PARAMETER_SCANNER_COMPLETED_KEY, RINGTONE_PARAMETER_SCANNER_COMPLETED_FALSE,
        scanCompletedValue, SYSPARA_SIZE);
    int isCompleted = atoi(scanCompletedValue);
    if (!isCompleted) {
        RingtoneScannerManager::GetInstance()->Start(false);
        SetParameter(RINGTONE_PARAMETER_SCANNER_COMPLETED_KEY, RINGTONE_PARAMETER_SCANNER_COMPLETED_TRUE);
    }

    RINGTONE_INFO_LOG("end.");
}

void RingtoneDataShareExtension::OnStop()
{
    RINGTONE_INFO_LOG("begin.");
    auto scannerManager = RingtoneScannerManager::GetInstance();
    if (scannerManager != nullptr) {
        scannerManager->Stop();
    }
    RingtoneDataManager::GetInstance()->ClearRingtoneDataMgr();
    RINGTONE_INFO_LOG("end.");
}

sptr<IRemoteObject> RingtoneDataShareExtension::OnConnect(const AAFwk::Want &want)
{
    RINGTONE_INFO_LOG("begin.");
    Extension::OnConnect(want);
    sptr<RingtoneDataShareStubImpl> remoteObject = new (nothrow) RingtoneDataShareStubImpl(
        static_pointer_cast<RingtoneDataShareExtension>(shared_from_this()),
        nullptr);
    if (remoteObject == nullptr) {
        RINGTONE_ERR_LOG("No memory allocated for DataShareStubImpl");
        return nullptr;
    }
    RINGTONE_INFO_LOG("end.");
    return remoteObject->AsObject();
}

static int32_t CheckRingtonePerm(RingtoneDataCommand &cmd, bool isWrite)
{
    auto err = E_SUCCESS;
    if (!RingtonePermissionUtils::IsSystemApp() && IPCSkeleton::GetCallingUid() != 0
        && !RingtonePermissionUtils::IsNativeSAApp()) {
        RINGTONE_ERR_LOG("RingtoneLibrary should only be called by system applications!");
        return E_PERMISSION_DENIED;
    }

    if (isWrite) {
        err = (RingtonePermissionUtils::CheckCallerPermission(PERM_WRITE_RINGTONE) ? E_SUCCESS : E_PERMISSION_DENIED);
    }

    return err;
}

static int32_t UriValidCheck(const Uri &uri)
{
    string uriStr = uri.ToString();
    if (uriStr.compare(0, RINGTONE_PATH_URI.size(), RINGTONE_PATH_URI)) {
        RINGTONE_ERR_LOG("error: invalid uri!");
        return E_INVALID_URI;
    }
    return Media::E_OK;
}

static const std::vector<string> g_ringToneTableFields = {
    { RINGTONE_COLUMN_TONE_ID },
    { RINGTONE_COLUMN_DATA },
    { RINGTONE_COLUMN_SIZE },
    { RINGTONE_COLUMN_DISPLAY_NAME },
    { RINGTONE_COLUMN_TITLE },
    { RINGTONE_COLUMN_MEDIA_TYPE },
    { RINGTONE_COLUMN_TONE_TYPE },
    { RINGTONE_COLUMN_MIME_TYPE },
    { RINGTONE_COLUMN_SOURCE_TYPE },
    { RINGTONE_COLUMN_DATE_ADDED },
    { RINGTONE_COLUMN_DATE_MODIFIED },
    { RINGTONE_COLUMN_DATE_TAKEN },
    { RINGTONE_COLUMN_DURATION },
    { RINGTONE_COLUMN_SHOT_TONE_TYPE },
    { RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE },
    { RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE },
    { RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE },
    { RINGTONE_COLUMN_RING_TONE_TYPE },
    { RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE },
    { RINGTONE_COLUMN_ALARM_TONE_TYPE },
    { RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE },
};

void RingtoneDataShareExtension::DumpDataShareValueBucket(const std::vector<string> &tabFields,
    const DataShareValuesBucket &value)
{
    bool isValid = false;
    for (auto tab : tabFields) {
        auto valueObject = value.Get(tab, isValid);
        if (!isValid) {
            RINGTONE_INFO_LOG("not set field: %{public}s", tab.c_str());
            continue;
        }

        if (std::get_if<std::vector<uint8_t>>(&(valueObject.value))) {
            auto value = static_cast<std::vector<uint8_t>>(valueObject);
            RINGTONE_INFO_LOG("field: %{public}s, value=%{public}s",
                tab.c_str(), value.data());
        } else if (std::get_if<int64_t>(&(valueObject.value))) {
            auto value = static_cast<int64_t>(valueObject);
            RINGTONE_INFO_LOG("field: %{public}s, value=%{public}" PRId64,
                tab.c_str(), value);
        } else if (std::get_if<std::string>(&(valueObject.value))) {
            auto value = static_cast<std::string>(valueObject);
            RINGTONE_INFO_LOG("field: %{public}s, value=%{public}s",
                tab.c_str(), value.c_str());
        } else if (std::get_if<bool>(&(valueObject.value))) {
            auto value = static_cast<bool>(valueObject);
            RINGTONE_INFO_LOG("field: %{public}s, value=%{public}d",
                tab.c_str(), value);
        } else if (std::get_if<double>(&(valueObject.value))) {
            auto value = static_cast<double>(valueObject);
            RINGTONE_INFO_LOG("field: %{public}s, value=%{public}lf",
                tab.c_str(), value);
        } else {
            RINGTONE_INFO_LOG("unkown field: %{public}s type", tab.c_str());
        }
    }
}

int RingtoneDataShareExtension::Insert(const Uri &uri, const DataShareValuesBucket &value)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s", uri.ToString().c_str());

    DumpDataShareValueBucket(g_ringToneTableFields, value);

    int err = UriValidCheck(uri);
    if (err != Media::E_OK) {
        return err;
    }
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    err = CheckRingtonePerm(cmd, true);
    if (err < 0) {
        RINGTONE_ERR_LOG("Check Insert-permission failed, errCode: %{public}d", err);
        return err;
    }

    auto ret = RingtoneDataManager::GetInstance()->Insert(cmd, value);
    return ret;
}

int RingtoneDataShareExtension::Update(const Uri &uri, const DataSharePredicates &predicates,
    const DataShareValuesBucket &value)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s", uri.ToString().c_str());
    RINGTONE_DEBUG_LOG("WhereClause=%{public}s", predicates.GetWhereClause().c_str());

    int err = UriValidCheck(uri);
    if (err != Media::E_OK) {
        return err;
    }
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::UPDATE);
    err = CheckRingtonePerm(cmd, false);
    if (err < 0) {
        RINGTONE_ERR_LOG("Check Update-permission failed, errCode: %{public}d", err);
        return err;
    }

    return RingtoneDataManager::GetInstance()->Update(cmd, value, predicates);
}

int RingtoneDataShareExtension::Delete(const Uri &uri, const DataSharePredicates &predicates)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s", uri.ToString().c_str());

    int err = UriValidCheck(uri);
    if (err != Media::E_OK) {
        return err;
    }
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::DELETE);
    err = CheckRingtonePerm(cmd, true);
    if (err < 0) {
        RINGTONE_ERR_LOG("Check Delete-permission failed, errCode: %{public}d", err);
        return err;
    }

    return RingtoneDataManager::GetInstance()->Delete(cmd, predicates);
}

shared_ptr<DataShareResultSet> RingtoneDataShareExtension::Query(const Uri &uri,
    const DataSharePredicates &predicates, vector<string> &columns, DatashareBusinessError &businessError)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s", uri.ToString().c_str());
    int err = UriValidCheck(uri);
    if (err != Media::E_OK) {
        return nullptr;
    }
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    err = CheckRingtonePerm(cmd, false);
    if (err < 0) {
        businessError.SetCode(err);
        RINGTONE_ERR_LOG("Check Query-permission failed, errCode: %{public}d", err);
        return nullptr;
    }

    int errCode = businessError.GetCode();
    auto queryResultSet = RingtoneDataManager::GetInstance()->Query(cmd, columns, predicates, errCode);
    businessError.SetCode(to_string(errCode));
    if (queryResultSet == nullptr) {
        RINGTONE_ERR_LOG("queryResultSet is nullptr! errCode: %{public}d", errCode);
        return nullptr;
    }
    shared_ptr<DataShareResultSet> resultSet = make_shared<DataShareResultSet>(queryResultSet);
    return resultSet;
}

int RingtoneDataShareExtension::OpenFile(const Uri &uri, const string &mode)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s, mode=%{public}s",
        uri.ToString().c_str(), mode.c_str());

    int err = UriValidCheck(uri);
    if (err != Media::E_OK) {
        return err;
    }
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::OPEN);

    string unifyMode = mode;
    transform(unifyMode.begin(), unifyMode.end(), unifyMode.begin(), ::tolower);

    bool isWrite = false;
    auto iter = find(RINGTONE_OPEN_WRITE_MODE_VECTOR.begin(), RINGTONE_OPEN_WRITE_MODE_VECTOR.end(), unifyMode);
    if (iter != RINGTONE_OPEN_WRITE_MODE_VECTOR.end()) {
        isWrite = true;
    }
    err = CheckRingtonePerm(cmd, isWrite);
    if (err == E_PERMISSION_DENIED) {
        RINGTONE_ERR_LOG("OpenFile denied, errCode: %{public}d", err);
        return err;
    }
    return RingtoneDataManager::GetInstance()->OpenFile(cmd, unifyMode);
}

static DataShare::DataShareExtAbility *RingtoneDataShareCreator(const unique_ptr<Runtime> &runtime)
{
    RINGTONE_DEBUG_LOG("entry");
    return  RingtoneDataShareExtension::Create(runtime);
}

__attribute__((constructor)) void RegisterDataShareCreator()
{
    RINGTONE_ERR_LOG("entry");
    DataShare::DataShareExtAbility::SetCreator(RingtoneDataShareCreator);

    RINGTONE_ERR_LOG("End");
}
} // namespace AbilityRuntime
} // namespace OHOS
