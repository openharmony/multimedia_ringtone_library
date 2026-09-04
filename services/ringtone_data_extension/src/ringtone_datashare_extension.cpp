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
#include "result_set_utils.h"
#include "ringtone_data_manager.h"
#include "ringtone_datashare_stub_impl.h"
#include "ringtone_file_utils.h"
#include "ringtone_language_manager.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"
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
    {VIBRATE_PATH_URI, VIBRATE_TABLE},
    {HAPTIC_2_TONE_PATH_URI, HAPTIC_2_TONE_TABLE}
};

static const char RINGTONE_RDB_SCANNER_FLAG_KEY[] = "RDBInitScanner";
static const int RINGTONE_RDB_SCANNER_FLAG_KEY_TRUE = 1;
static const int RINGTONE_RDB_SCANNER_FLAG_KEY_FALSE = 0;
const std::string PRELOAD_RINGTONE_TYPE = "1";

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

/**
 * @brief 检查铃声数据库默认设置是否已初始化。
 *
 * 通过Preferences查询"RDBInitScanner"标志位，判断铃声数据库的默认铃声是否已完成首次写入。
 * 若标志位为false，则查询preload_config表中是否已存在默认铃声记录：
 *   - 若存在，将"RDBInitScanner"标志位设为true并持久化，表示默认铃声已就绪；
 *   - 若不存在，保持标志位不变，等待后续扫描流程写入默认铃声。
 * 该方法在OnStart中调用，用于确保首次开机铃声数据就绪。
 */
void RingtoneDataShareExtension::CheckRingtoneDbDefaultSettings()
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
    auto rdbStore = RingtoneRdbStore::GetInstance();
    CHECK_AND_RETURN_LOG(rdbStore != nullptr, "rdbstore is nullptr");
    auto rawRdb = rdbStore->GetRaw();
    CHECK_AND_RETURN_LOG(rawRdb != nullptr, "rawRdb is nullptr");
    string sql = "SELECT " + PRELOAD_CONFIG_COLUMN_DISPLAY_NAME + " FROM " +
        PRELOAD_CONFIG_TABLE + " WHERE " + PRELOAD_CONFIG_COLUMN_RING_TONE_TYPE + " = " + PRELOAD_RINGTONE_TYPE;
    auto resultSet = rawRdb->QuerySql(sql);
    CHECK_AND_RETURN_LOG(resultSet != nullptr, "resultSet is nullptr");
    if (resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        resultSet->Close();
        RINGTONE_ERR_LOG("Query Preload failed, no resultSet");
        return;
    }
    string displayName = GetStringVal(PRELOAD_CONFIG_COLUMN_DISPLAY_NAME, resultSet);
    if (!displayName.empty()) {
        RINGTONE_INFO_LOG("The default ringtone has been set. ringtone=%{public}s", displayName.c_str());
        prefs->PutInt(RINGTONE_RDB_SCANNER_FLAG_KEY, RINGTONE_RDB_SCANNER_FLAG_KEY_TRUE);
        prefs->FlushSync();
    }
    resultSet->Close();
    return;
}

/**
 * @brief 扩展能力启动回调。
 *
 * 在Extension生命周期启动时由框架调用，执行以下初始化流程：
 * 1. 调用基类OnStart完成基础初始化；
 * 2. 获取应用上下文，若获取失败则终止应用进程；
 * 3. 初始化RingtoneDataManager单例，若失败则终止应用进程；
 * 4. 设置DataManager的Owner为当前Extension实例；
 * 5. 初始化DFX（设计质量）管理器；
 * 6. 检查铃声数据库默认设置（CheckRingtoneDbDefaultSettings）；
 * 7. 执行铃声扫描（RingtoneScanner）；
 * 8. 若旧版自定义铃声目录存在，则迁移旧路径数据到新路径（UpdataRdbPathData）；
 * 9. 同步铃声资源的多语言信息。
 *
 * @param want 启动意图，包含启动信息。
 */
void RingtoneDataShareExtension::OnStart(const AAFwk::Want &want)
{
    RINGTONE_INFO_LOG("Ringtone OnStart begin.");
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

    CheckRingtoneDbDefaultSettings();
    RingtoneScanner();
    
    if (RingtoneFileUtils::IsFileExists(OLD_RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH)) {
        UpdataRdbPathData();
    }

    RingtoneLanguageManager::GetInstance()->SyncAssetLanguage();
    RINGTONE_INFO_LOG("Ringtone OnStart end.");
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

/**
 * @brief 扩展能力连接回调。
 *
 * 当客户端首次连接铃声数据共享Extension时由框架调用，执行以下流程：
 * 1. 校验调用方身份，仅允许系统应用或原生SA应用连接，非系统应用返回nullptr；
 * 2. 校验不支持跨端分布式场景（Want中deviceId不为空时返回nullptr）；
 * 3. 创建RingtoneDataShareStubImpl远程对象，用于客户端IPC通信；
 * 4. 返回远程对象的IRemoteObject引用。
 *
 * @param want 连接意图，包含设备ID等连接信息。
 * @return 返回远程通信对象的IRemoteObject指针；若校验失败或内存不足则返回nullptr。
 */
sptr<IRemoteObject> RingtoneDataShareExtension::OnConnect(const AAFwk::Want &want)
{
    if (!RingtonePermissionUtils::IsSystemApp() && IPCSkeleton::GetCallingUid() != 0
        && !RingtonePermissionUtils::IsNativeSAApp()) {
        RINGTONE_ERR_LOG("RingtoneLibrary should only be called by system applications!");
        return nullptr;
    }
    // 不支持跨端分布式场景
    if (!want.GetDeviceId().empty()) {
        RINGTONE_ERR_LOG("RingtoneLibrary does not support distributed scenario!");
        return nullptr;
    }
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
        err = (RingtonePermissionUtils::CheckCallerPermission(PERM_WRITE_RINGTONE) ? E_SUCCESS : E_PERMISSION_DENIED);
    }

    return err;
}

/**
 * @brief 从URI中解析有效的数据库表名。
 *
 * 解析逻辑分两步：
 * 1. 若URI中包含代理字符串（RINGTONE_URI_PROXY_STRING），则提取最后一个'/'到代理字符串之间的部分作为表名；
 * 2. 若不含代理字符串，则在预定义的URI到表名的映射表（VALID_URI_TO_TABLE）中查找匹配项。
 * 支持的URI路径包括：SIM卡设置、铃声、振动、触觉反馈2音。
 *
 * @param uri 数据共享操作的URI。
 * @param tab 输出参数，解析得到的数据库表名。
 * @return 返回Media::E_OK表示解析成功，E_INVALID_URI表示URI无效。
 */
static int32_t GetValidUriTab(const Uri &uri, string &tab)
{
    string uriStr = uri.ToString();
    auto proxyStringPos = uriStr.find(RINGTONE_URI_PROXY_STRING);
    auto lastSlash = uriStr.find_last_of('/');
    if (lastSlash != std::string::npos &&
        proxyStringPos != std::string::npos && proxyStringPos > lastSlash) {
        auto tablePos = lastSlash + 1;
        tab = uriStr.substr(tablePos, proxyStringPos - tablePos);
        // 白名单校验：提取的表名必须是已知的合法表名，防止任意表名注入
        bool validTab = false;
        for (const auto &pair : VALID_URI_TO_TABLE) {
            if (tab == pair.second) {
                validTab = true;
                break;
            }
        }
        if (!validTab) {
            uriStr.erase(std::remove_if(uriStr.begin(), uriStr.end(),
                [](char c){ return c == '\r' || c == '\n'; }), uriStr.end());
            RINGTONE_ERR_LOG("Invalid table name extracted from proxy URI, tab=%{public}s, uri=%{public}s",
                tab.c_str(), uriStr.c_str());
            return E_INVALID_URI;
        }
        return Media::E_OK;
    }

    for (const auto &pair : VALID_URI_TO_TABLE) {
        if (uriStr.find(pair.first) != std::string::npos) {
            tab = pair.second;
            return Media::E_OK;
        }
    }

    uriStr.erase(std::remove_if(uriStr.begin(), uriStr.end(),
        [](char c){ return c == '\r' || c == '\n'; }), uriStr.end());
    RINGTONE_INFO_LOG("INVALID uri=%{public}s", uriStr.c_str());
    return E_INVALID_URI;
}

/**
 * @brief 过滤URI字符串中的换行控制字符，防止日志注入。
 *
 * 移除字符串中的 \r 和 \n 字符，避免恶意构造的URI在日志输出时
 * 产生伪造的日志条目。
 *
 * @param uriStr 待过滤的URI字符串。
 * @return 过滤后的安全字符串。
 */
static std::string SanitizeUriForLog(const std::string &uriStr)
{
    std::string sanitized = uriStr;
    sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(),
        [](char c){ return c == '\r' || c == '\n'; }), sanitized.end());
    return sanitized;
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

/**
 * @brief 迁移旧路径铃声数据到新路径。
 *
 * 当检测到旧版自定义铃声目录（/storage/media/local/files/Ringtone）仍然存在时，
 * 执行以下迁移流程：
 * 1. 查询数据库中所有路径包含旧路径前缀的铃声记录；
 * 2. 逐条将铃声文件从旧路径复制到新路径（/data/storage/el2/base/files/Ringtone）；
 * 3. 更新数据库中对应记录的DATA字段为新路径；
 * 4. 若所有文件均迁移成功，删除旧版铃声目录。
 * 该方法在OnStart中条件触发调用，用于兼容旧版本升级场景。
 */
void RingtoneDataShareExtension::UpdataRdbPathData()
{
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
    bool bRemoveRingtoneFolder = true;
    while (ringtoneAsset != nullptr) {
        int toneId = ringtoneAsset->GetId();
        std::string oldPath = ringtoneAsset->GetPath();
        std::string newPath = oldPath;
        size_t start_pos = 0;
        if ((start_pos = newPath.find("/storage/media/local/files/Ringtone/")) != std::string::npos) {
            newPath.replace(start_pos, std::string("/storage/media/local/files/Ringtone").length(),
                "/data/storage/el2/base/files/Ringtone");
        }
        if (RingtoneFileUtils::CopyFileUtil(oldPath, newPath) == false) {
            RINGTONE_ERR_LOG("copy file fail, src: %{public}s, dest: %{public}s", oldPath.c_str(),
                newPath.c_str());
            bRemoveRingtoneFolder = false;
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
    if (bRemoveRingtoneFolder) {
        RingtoneFileUtils::RemoveRingtoneFolder(OLD_RINGTONE_CUSTOMIZED_BASE_RINGTONE_PATH);
    }
    return;
}

/**
 * @brief 向数据库插入一条铃声数据记录。
 *
 * 执行流程：
 * 1. 打印待插入的字段值到日志（用于调试）；
 * 2. 解析URI获取目标数据库表名；
 * 3. 若目标表为Haptic2ToneFiles表，拒绝插入操作（该表不支持插入）；
 * 4. 校验写操作权限（PERM_WRITE_RINGTONE）；
 * 5. 委托RingtoneDataManager执行插入操作。
 *
 * @param uri 数据共享操作的URI，指示数据路径。
 * @param value 待插入的数据键值对，包含各字段的值。
 * @return 成功时返回插入记录的行ID（>=0），失败时返回错误码（<0）。
 */
int RingtoneDataShareExtension::Insert(const Uri &uri, const DataShareValuesBucket &value)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s", SanitizeUriForLog(uri.ToString()).c_str());

    DumpDataShareValueBucket(g_ringToneTableFields, value);

    string tab("");
    int err = GetValidUriTab(uri, tab);
    if (err != Media::E_OK) {
        return err;
    }

    if (tab == HAPTIC_2_TONE_TABLE) {
        RINGTONE_ERR_LOG("Haptic2ToneFiles table does not support insert operation");
        return Media::E_INVALID_URI;
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

/**
 * @brief 更新数据库中的铃声数据记录。
 *
 * 执行流程：
 * 1. 解析URI获取目标数据库表名；
 * 2. 若目标表为Haptic2ToneFiles表，拒绝更新操作（该表不支持更新）；
 * 3. 校验操作权限（读权限，因更新仅需系统应用身份）；
 * 4. 委托RingtoneDataManager执行更新操作。
 *
 * @param uri 数据共享操作的URI，指示数据路径。
 * @param predicates 过滤条件，指定需要更新的记录范围。
 * @param value 待更新的数据键值对，包含需要修改的字段新值。
 * @return 成功时返回更新的记录数（>=0），失败时返回错误码（<0）。
 */
int RingtoneDataShareExtension::Update(const Uri &uri, const DataSharePredicates &predicates,
    const DataShareValuesBucket &value)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s", SanitizeUriForLog(uri.ToString()).c_str());
    RINGTONE_DEBUG_LOG("WhereClause=%{public}s", predicates.GetWhereClause().c_str());

    string tab("");
    int err = GetValidUriTab(uri, tab);
    if (err != Media::E_OK) {
        return err;
    }

    if (tab == HAPTIC_2_TONE_TABLE) {
        RINGTONE_ERR_LOG("Haptic2ToneFiles table does not support update operation");
        return Media::E_INVALID_URI;
    }

    RingtoneDataCommand cmd(uri, tab, RingtoneOperationType::UPDATE);
    err = CheckRingtonePerm(cmd, false);
    if (err < 0) {
        RINGTONE_ERR_LOG("Check Update-permission failed, errCode: %{public}d", err);
        return err;
    }

    return RingtoneDataManager::GetInstance()->Update(cmd, value, predicates);
}

/**
 * @brief 从数据库中删除铃声数据记录。
 *
 * 执行流程：
 * 1. 解析URI获取目标数据库表名；
 * 2. 若目标表为Haptic2ToneFiles表，拒绝删除操作（该表不支持删除）；
 * 3. 校验写操作权限（PERM_WRITE_RINGTONE）；
 * 4. 委托RingtoneDataManager执行删除操作。
 *
 * @param uri 数据共享操作的URI，指示数据路径。
 * @param predicates 过滤条件，指定需要删除的记录范围。
 * @return 成功时返回删除的记录数（>=0），失败时返回错误码（<0）。
 */
int RingtoneDataShareExtension::Delete(const Uri &uri, const DataSharePredicates &predicates)
{
    string tab("");
    int err = GetValidUriTab(uri, tab);
    if (err != Media::E_OK) {
        return err;
    }
    RINGTONE_WARN_LOG("entry, uri=%{public}s", SanitizeUriForLog(uri.ToString()).c_str());

    if (tab == HAPTIC_2_TONE_TABLE) {
        RINGTONE_ERR_LOG("Haptic2ToneFiles table does not support delete operation");
        return Media::E_INVALID_URI;
    }

    RingtoneDataCommand cmd(uri, tab, RingtoneOperationType::DELETE);
    err = CheckRingtonePerm(cmd, true);
    if (err < 0) {
        RINGTONE_ERR_LOG("Check Delete-permission failed, errCode: %{public}d", err);
        return err;
    }

    return RingtoneDataManager::GetInstance()->Delete(cmd, predicates);
}

/**
 * @brief 查询数据库中的铃声数据记录。
 *
 * 执行流程：
 * 1. 解析URI获取目标数据库表名；
 * 2. 校验读操作权限（仅需系统应用身份）；
 * 3. 委托RingtoneDataManager执行查询操作；
 * 4. 将原生ResultSet包装为DataShareResultSet返回给调用方；
 * 5. 通过businessError输出参数返回业务错误码。
 *
 * @param uri 数据共享操作的URI，指示数据路径。
 * @param predicates 过滤条件，指定查询范围和排序等。
 * @param columns 需要查询的列名列表。若为空则查询所有列。
 * @param businessError 输出参数，用于返回业务错误码和错误信息。
 * @return 成功时返回查询结果集，失败时返回nullptr。
 */
shared_ptr<DataShareResultSet> RingtoneDataShareExtension::Query(const Uri &uri,
    const DataSharePredicates &predicates, vector<string> &columns, DatashareBusinessError &businessError)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s", SanitizeUriForLog(uri.ToString()).c_str());

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

/**
 * @brief 以指定模式打开铃声文件，返回文件描述符。
 *
 * 执行流程：
 * 1. 解析URI获取目标数据库表名；
 * 2. 将打开模式统一转为小写，判断是否为写模式（包含w/rw/wt/wa/rwt/rwa等）；
 * 3. 根据模式类型校验相应权限（写模式需PERM_WRITE_RINGTONE权限）；
 * 4. 委托RingtoneDataManager打开文件。
 *
 * @param uri 数据共享操作的URI，指示文件路径。
 * @param mode 文件打开模式，如"r"（只读）、"w"（只写）、"rw"（读写）、"wt"（写截断）、"wa"（追加写）、"rwt"（读写截断）等。
 * @return 成功时返回文件描述符（>=0），失败时返回错误码（<0）。
 */
int RingtoneDataShareExtension::OpenFile(const Uri &uri, const string &mode)
{
    RINGTONE_DEBUG_LOG("entry, uri=%{public}s, mode=%{public}s",
        SanitizeUriForLog(uri.ToString()).c_str(), mode.c_str());

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

/**
 * @brief 检查当前用户是否已在系统参数中注册，若未注册则追加注册。
 *
 * 通过系统参数"ringtone.scanner.userId"获取已注册的用户ID列表（空格分隔），
 * 判断当前活跃用户是否在其中。若存在则返回true，表示当前用户已注册；
 * 若不存在，则将当前用户ID追加到列表中并写回系统参数，返回false。
 * 该方法在RingtoneScanner中使用，用于多用户场景下判断是否需要为当前用户执行首次扫描。
 *
 * @return true表示当前用户已存在于已注册列表中，false表示当前用户是新注册的（首次扫描）。
 */
bool RingtoneDataShareExtension::CheckCurrentUser()
{
    RINGTONE_INFO_LOG("CheckCurrentUser Start.");
    char paramValue[RINGTONEPARA_SIZE] = {0};
    GetParameter(RINGTONE_PARAMETER_SCANNER_USERID_KEY, "", paramValue, RINGTONEPARA_SIZE);
    std::string ids(paramValue);
    RINGTONE_WARN_LOG("GetParameter end, paramValue: %{private}s .", ids.c_str());
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

/**
 * @brief 执行铃声扫描流程。
 *
 * 在OnStart中调用，负责扫描铃声目录并将铃声文件信息写入数据库。流程如下：
 * 1. 通过RingtoneFileUtils确保铃声目录可访问；
 * 2. 调用CheckCurrentUser检查当前用户是否已注册，同时管理系统参数；
 * 3. 读取"ringtone.scanner.first"系统参数判断是否为首次启动：
 *    - 若当前用户为新用户且参数为"true"，则将其设为"false"，表示首次扫描已触发；
 * 4. 当参数为"false"时，启动RingtoneScannerManager执行非首次扫描（增量扫描）；
 *    若参数仍为"true"（即首次启动且当前用户已注册），则不触发扫描，
 *    等待其他初始化流程完成后由后续机制触发。
 */
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
    RINGTONE_WARN_LOG("GetParameter end, paramValue: %{public}s .", parameter.c_str());
    if (strcmp(paramValue, RINGTONE_PARAMETER_SCANNER_FIRST_FALSE) == 0) {
        RingtoneScannerManager::GetInstance()->Start(false);
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
