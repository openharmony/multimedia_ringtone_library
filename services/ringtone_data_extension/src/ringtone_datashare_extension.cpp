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
#include "dfx_const.h"
#include "os_account_manager.h"
#include "parameter.h"
#include "permission_utils.h"
#include "preferences_helper.h"
#include "ipc_skeleton.h"
#include "ringtone_data_manager.h"
#include "ringtone_datashare_stub_impl.h"
#include "ringtone_file_utils.h"
#include "ringtone_language_manager.h"
#include "ringtone_log.h"
#include "ringtone_scanner_manager.h"
#include "ringtone_tracer.h"
#include "runtime.h"
#include "singleton.h"
#include "ringtone_proxy_uri.h"
#include "ringtone_fetch_result.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace std;
using namespace OHOS::AppExecFwk;
using namespace OHOS::NativeRdb;
using namespace OHOS::Media;
using namespace OHOS::DataShare;

const char RINGTONE_PARAMETER_SCANNER_FIRST_KEY[] = "ringtone.scanner.first";
const char RINGTONE_PARAMETER_SCANNER_FIRST_TRUE[] = "true";
const char RINGTONE_PARAMETER_SCANNER_FIRST_FALSE[] = "false";
const char RINGTONE_PARAMETER_SCANNER_USERID_KEY[] = "ringtone.scanner.userId";
const int32_t RINGTONEPARA_SIZE = 64;
const std::string OLD_RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH = "/storage/media/local/files/Ringtone";
const std::vector<std::string> RINGTONE_OPEN_WRITE_MODE_VECTOR = {
    { RINGTONE_FILEMODE_WRITEONLY },
    { RINGTONE_FILEMODE_READWRITE },
    { RINGTONE_FILEMODE_WRITETRUNCATE },
    { RINGTONE_FILEMODE_WRITEAPPEND },
    { RINGTONE_FILEMODE_READWRITETRUNCATE },
    { RINGTONE_FILEMODE_READWRITEAPPEND },
};

std::map<std::string, std::string> VALID_URI_TO_TABLE {
    {SIMCARD_SETTING_PATH_URI, SIMCARD_SETTING_TABLE},
    {RINGTONE_PATH_URI, RINGTONE_TABLE},
    {VIBRATE_PATH_URI, VIBRATE_TABLE}
};

static const char RINGTONE_RDB_SCANNER_FLAG_KEY[] = "RDBInitScanner";
static const int RINGTONE_RDB_SCANNER_FLAG_KEY_TRUE = 1;
static const int RINGTONE_RDB_SCANNER_FLAG_KEY_FALSE = 0;

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

static void CheckRingtoneDbPath()
{
    int errCode = 0;
    shared_ptr<NativePreferences::Preferences> prefs =
        NativePreferences::PreferencesHelper::GetPreferences(COMMON_XML_EL1, errCode);
    if (!prefs) {
        RINGTONE_ERR_LOG("get preferences error: %{public}d", errCode);
        return;
    }
    int isScanner = prefs->GetInt(RINGTONE_RDB_SCANNER_FLAG_KEY, RINGTONE_RDB_SCANNER_FLAG_KEY_FALSE);
    if (isScanner == RINGTONE_RDB_SCANNER_FLAG_KEY_TRUE) {
        return;
    }
    string dbPath = RINGTONE_LIBRARY_DB_PATH_EL1 + "/rdb" + "/" + RINGTONE_LIBRARY_DB_NAME;
    if (RingtoneFileUtils::IsFileExists(dbPath)) {
        prefs->PutInt(RINGTONE_RDB_SCANNER_FLAG_KEY, RINGTONE_RDB_SCANNER_FLAG_KEY_TRUE);
        prefs->FlushSync();
    }
}

void RingtoneDataShareExtension::OnStart(const AAFwk::Want &want)
{
    RINGTONE_DEBUG_LOG("begin.");
    Extension::OnStart(want);
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        RINGTONE_ERR_LOG("Failed to get context");
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->KillApplicationSelf();
        return;
    }
    RINGTONE_INFO_LOG("runtime language %{public}d", runtime_.GetLanguage());
    CheckRingtoneDbPath();
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

    RingtoneScanner();
    
    if (RingtoneFileUtils::IsFileExists(OLD_RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH)) {
        UpdataRdbPathData();
    }

    RingtoneLanguageManager::GetInstance()->SyncAssetLanguage();
    RINGTONE_DEBUG_LOG("end.");
}

void RingtoneDataShareExtension::OnStop()
{
    RINGTONE_DEBUG_LOG("begin.");
    auto scannerManager = RingtoneScannerManager::GetInstance();
    if (scannerManager != nullptr) {
        scannerManager->Stop();
    }
    RingtoneDataManager::GetInstance()->ClearRingtoneDataMgr();
    RINGTONE_DEBUG_LOG("end.");
}

sptr<IRemoteObject> RingtoneDataShareExtension::OnConnect(const AAFwk::Want &want)
{
    RINGTONE_DEBUG_LOG("begin.");
    Extension::OnConnect(want);
    sptr<RingtoneDataShareStubImpl> remoteObject = new (nothrow) RingtoneDataShareStubImpl(
        static_pointer_cast<RingtoneDataShareExtension>(shared_from_this()),
        nullptr);
    if (remoteObject == nullptr) {
        RINGTONE_ERR_LOG("No memory allocated for DataShareStubImpl");
        return nullptr;
    }
    RINGTONE_DEBUG_LOG("end.");
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
        err = (RingtonePermissionUtils::CheckCallerPermission(PERM_WRITE_RINGTONE)||
            (RingtonePermissionUtils::CheckCallerPermission(PERM_ACCESS_CUSTOM_RINGTONE)) ?
            E_SUCCESS : E_PERMISSION_DENIED);
    }

    return err;
}

static int32_t GetValidUriTab(const Uri &uri, string &tab)
{
    string uriStr = uri.ToString();

    auto proxyStringPos = uriStr.find(RINGTONE_URI_PROXY_STRING);
    auto lastSlash = uriStr.find_last_of('/');
    if (lastSlash != std::string::npos &&
        proxyStringPos != std::string::npos) {
        auto tablePos = lastSlash + 1;
        tab = uriStr.substr(tablePos, proxyStringPos - tablePos);
        return Media::E_OK;
    }

    for (const auto &pair : VALID_URI_TO_TABLE) {
        if (uriStr.find(pair.first) != std::string::npos) {
            tab = pair.second;
            return Media::E_OK;
        }
    }

    RINGTONE_INFO_LOG("INVALID uri=%{public}s", uriStr.c_str());
    return E_INVALID_URI;
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
    { RINGTONE_COLUMN_SCANNER_FLAG },
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

void RingtoneDataShareExtension::UpdataRdbPathData()
{
    auto result = RingtoneFileUtils::MoveRingtoneFolder();
    CHECK_AND_RETURN_LOG(result, "MoveRingtoneFolder failed");

    Uri uri(RINGTONE_LIBRARY_PROXY_DATA_URI_TONE_FILES);

    DataSharePredicates predicates;
    const std::string selection = RINGTONE_COLUMN_DATA + " LIKE ? ";
    std::vector<std::string> selectionArgs = {"%/storage/media/local/files/%"};
    predicates.SetWhereClause(selection);
    predicates.SetWhereArgs(selectionArgs);

    vector<string> columns { RINGTONE_COLUMN_DATA, RINGTONE_COLUMN_TONE_ID };
    DatashareBusinessError businessError;
    auto resultSet = Query(uri, predicates, columns, businessError);
    CHECK_AND_RETURN_LOG(resultSet != nullptr, "query is nullptr");
 
    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    unique_ptr<RingtoneAsset> ringtoneAsset = results->GetFirstObject();
    CHECK_AND_RETURN_LOG(ringtoneAsset != nullptr, "ringtoneAsset is nullptr");

    while (ringtoneAsset != nullptr) {
        int toneId = ringtoneAsset->GetId();
        std::string oldPath = ringtoneAsset->GetPath();
        std::string newPath = oldPath;
        size_t start_pos = 0;
        if ((start_pos = newPath.find("/storage/media/local/files/Ringtone/")) != std::string::npos) {
            newPath.replace(start_pos, std::string("/storage/media/local/files/Ringtone").length(),
                "/data/storage/el2/base/files/Ringtone");
        }
        if (!RingtoneFileUtils::IsFileExists(newPath)) {
            RINGTONE_ERR_LOG("the file is not exists, path: %{public}s", newPath.c_str());
            ringtoneAsset = results->GetNextObject();
            continue;
        }
        DataSharePredicates predicates;
        const std::string selection = RINGTONE_COLUMN_TONE_ID + " = ? ";
        predicates.SetWhereClause(selection);
        predicates.SetWhereArgs({to_string(toneId)});
        DataShare::DataShareValuesBucket valuesBucket;
        valuesBucket.Put(RINGTONE_COLUMN_DATA, newPath);
        Update(uri, predicates, valuesBucket);
        ringtoneAsset = results->GetNextObject();
    }
    RingtoneFileUtils::RemoveRingtoneFolder(OLD_RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH);
    return;
}

int RingtoneDataShareExtension::Insert(const Uri &uri, const DataShareValuesBucket &value)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s", uri.ToString().c_str());

    DumpDataShareValueBucket(g_ringToneTableFields, value);

    string tab("");
    int err = GetValidUriTab(uri, tab);
    if (err != Media::E_OK) {
        return err;
    }

    RingtoneDataCommand cmd(uri, tab, RingtoneOperationType::INSERT);
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

    string tab("");
    int err = GetValidUriTab(uri, tab);
    if (err != Media::E_OK) {
        return err;
    }

    RingtoneDataCommand cmd(uri, tab, RingtoneOperationType::UPDATE);
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

    string tab("");
    int err = GetValidUriTab(uri, tab);
    if (err != Media::E_OK) {
        return err;
    }

    RingtoneDataCommand cmd(uri, tab, RingtoneOperationType::DELETE);
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

    string tab("");
    int err = GetValidUriTab(uri, tab);
    if (err != Media::E_OK) {
        return nullptr;
    }

    RingtoneDataCommand cmd(uri, tab, RingtoneOperationType::QUERY);
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

    string tab("");
    int err = GetValidUriTab(uri, tab);
    if (err != Media::E_OK) {
        return err;
    }

    RingtoneDataCommand cmd(uri, tab, RingtoneOperationType::OPEN);
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

int32_t RingtoneDataShareExtension::GetUserId()
{
    RINGTONE_INFO_LOG("GetUserID Start.");
    int32_t userId = 0;
    std::vector<int> activeIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        RINGTONE_ERR_LOG("QueryActiveOsAccountIds failed ret:%{public}d", ret);
        return userId;
    }
    if (activeIds.empty()) {
        RINGTONE_ERR_LOG("QueryActiveOsAccountIds activeIds empty");
        return userId;
    }
    userId = activeIds[0];
    RINGTONE_INFO_LOG("GetUserID End, userId: %{private}d", userId);
    return userId;
}

bool RingtoneDataShareExtension::IdExists(const std::string &ids, int32_t id)
{
    RINGTONE_INFO_LOG("IdExists Start.");
    if (ids.empty()) {
        return false;
    }
    
    size_t pos = 0;
    std::string idStr = std::to_string(id);
    
    while ((pos = ids.find(idStr, pos)) != std::string::npos) {
        bool startPos = (pos == 0) || (ids[pos - 1] == ' ');
        bool endPos = (pos + idStr.length() == ids.length()) || (ids[pos + idStr.length()] == ' ');
        if (startPos && endPos) {
            return true;
        }
        pos += idStr.length();
    }
    RINGTONE_INFO_LOG("IdExists End.");
    return false;
}

bool RingtoneDataShareExtension::CheckCurrentUser()
{
    RINGTONE_INFO_LOG("CheckCurrentUser Start.");
    char paramValue[RINGTONEPARA_SIZE] = {0};
    GetParameter(RINGTONE_PARAMETER_SCANNER_USERID_KEY, "", paramValue, RINGTONEPARA_SIZE);
    std::string ids(paramValue);
    RINGTONE_INFO_LOG("GetParameter end, paramValue: %{private}s .", ids.c_str());
    int32_t currentUserId = GetUserId();
    if (IdExists(ids, currentUserId)) {
        return true;
    }
    if (!ids.empty() && ids.back() != ' ') {
        ids += " ";
    }
    ids += std::to_string(currentUserId);
    RINGTONE_INFO_LOG("CurrentUserIds: %{private}s .", ids.c_str());
    const char* cstr = ids.c_str();
    int result = SetParameter(RINGTONE_PARAMETER_SCANNER_USERID_KEY, cstr);
    RINGTONE_INFO_LOG("CheckCurrentUser End. SetParameter end, result: %{public}d", result);
    return false;
}

void RingtoneDataShareExtension::RingtoneScanner()
{
    RingtoneTracer tracer;
    tracer.Start("Ringtone Scanner");
    RINGTONE_INFO_LOG("Ringtone Scanner Start.");
    RingtoneFileUtils::AccessRingtoneDir();
    // ringtone scan
    char paramValue[RINGTONEPARA_SIZE] = {0};
    bool currentFlag = CheckCurrentUser();
    GetParameter(RINGTONE_PARAMETER_SCANNER_FIRST_KEY, "", paramValue, RINGTONEPARA_SIZE);
    if (!currentFlag && strcmp(paramValue, RINGTONE_PARAMETER_SCANNER_FIRST_TRUE) == 0) {
        int result = SetParameter(RINGTONE_PARAMETER_SCANNER_FIRST_KEY, RINGTONE_PARAMETER_SCANNER_FIRST_FALSE);
        RINGTONE_INFO_LOG("CheckCurrentUser SetParameter end, result: %{public}d", result);
    }
    GetParameter(RINGTONE_PARAMETER_SCANNER_FIRST_KEY, "", paramValue, RINGTONEPARA_SIZE);
    std::string parameter(paramValue);
    RINGTONE_INFO_LOG("GetParameter end, paramValue: %{public}s .", parameter.c_str());
    if (strcmp(paramValue, RINGTONE_PARAMETER_SCANNER_FIRST_FALSE) == 0) {
        RingtoneScannerManager::GetInstance()->Start(false);
        int result = SetParameter(RINGTONE_PARAMETER_SCANNER_FIRST_KEY, RINGTONE_PARAMETER_SCANNER_FIRST_TRUE);
        RINGTONE_INFO_LOG("SetParameter end, result: %{public}d", result);
    }
    RINGTONE_INFO_LOG("Ringtone Scanner End.");
}

static DataShare::DataShareExtAbility *RingtoneDataShareCreator(const unique_ptr<Runtime> &runtime)
{
    RINGTONE_DEBUG_LOG("entry");
    return  RingtoneDataShareExtension::Create(runtime);
}

__attribute__((constructor)) void RegisterDataShareCreator()
{
    RINGTONE_DEBUG_LOG("entry");
    DataShare::DataShareExtAbility::SetCreator(RingtoneDataShareCreator);

    RINGTONE_DEBUG_LOG("End");
}
} // namespace AbilityRuntime
} // namespace OHOS
