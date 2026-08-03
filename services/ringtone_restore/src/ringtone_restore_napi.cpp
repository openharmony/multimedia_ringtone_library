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

#include "ringtone_restore_napi.h"

#include "application_context.h"
#include "js_native_api.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_restore_factory.h"
#include "ringtone_restore_type.h"
#include "ringtone_restore_db_utils.h"

#define MLOG_TAG "Common"

namespace OHOS {
namespace Media {
using namespace std;

using RestoreBlock = struct {
    napi_env env;
    int32_t sceneCode;
    std::string baseBackupPath;
    int32_t resultSet;
    napi_deferred nativeDeferred;
};

using CleanESimBlock = struct {
    napi_env env;
    std::string dbPath;
    int32_t peerSlotNum;
    std::string ringtoneBasePath;
    int32_t resultSet;
    napi_deferred nativeDeferred;
};

napi_value RingtoneRestoreNapi::Init(napi_env env, napi_value exports)
{
    RINGTONE_INFO_LOG("Init");
    napi_property_descriptor ringtone_restore_properties[] = {
        DECLARE_NAPI_FUNCTION("startRestore", JSStartRestore),
        DECLARE_NAPI_FUNCTION("cleanESimData", JSCleanESimData)
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(ringtone_restore_properties) /
        sizeof(ringtone_restore_properties[0]), ringtone_restore_properties));
    return exports;
}

static int32_t GetIntFromParams(napi_env env, const napi_value args[], size_t index)
{
    int32_t result = -1;
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, args[index], &valueType) != napi_ok || valueType != napi_number) {
        // NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        RINGTONE_ERR_LOG("GetIntFromParams invalid parameter");
        return result;
    }
    napi_get_value_int32(env, args[index], &result);
    return result;
}

static std::string GetStringFromParams(napi_env env, const napi_value args[], size_t index)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, args[index], &valueType) != napi_ok || valueType != napi_string) {
        // NapiError::ThrowError(env, JS_ERR_PARAMETER_INVALID);
        return "";
    }

    size_t resultLength;
    napi_get_value_string_utf8(env, args[index], nullptr, 0, &resultLength);
    std::string result(resultLength, '\0');
    napi_get_value_string_utf8(env, args[index], &result[0], resultLength + 1, &resultLength);
    return result;
}

static int32_t CheckPermission(void)
{
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        RINGTONE_ERR_LOG("CheckPermission context nullptr");
        return E_FAIL;
    }
    std::string bundleName = context->GetBundleName();
    if (bundleName.compare(RINGTONE_BUNDLE_NAME) != 0) {
        RINGTONE_ERR_LOG("bundle name is invalid: %{public}s", bundleName.c_str());
        return E_FAIL;
    }
    RINGTONE_INFO_LOG("CheckPermission success");
    return E_OK;
}

static uv_loop_s *GetUvLoop(napi_env env)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    if (loop == nullptr) {
        RINGTONE_ERR_LOG("Failed to new uv_loop");
    }
    return loop;
}

static uv_work_t *CreateUvWorkWithPromise(napi_env env, napi_value &result, napi_deferred &nativeDeferred)
{
    napi_create_promise(env, &nativeDeferred, &result);
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        RINGTONE_ERR_LOG("Failed to new uv_work");
    }
    return work;
}

static int32_t RingtoneRestore(std::unique_ptr<RestoreInterface> &restore, string backupPath)
{
    int32_t ret = E_OK;
    if ((restore != nullptr) && (restore->Init(backupPath)) == Media::E_OK) {
        RINGTONE_INFO_LOG("start restore....");
        ret = restore->StartRestore();
        RINGTONE_INFO_LOG("restore finished");
    } else {
        RINGTONE_ERR_LOG("ringtone-restore failed on init");
        ret = E_HAS_DB_ERROR;
    }

    return ret;
}

void RingtoneRestoreNapi::UvQueueWork(uv_loop_s *loop, uv_work_t *work)
{
    if (loop == nullptr) {
        RINGTONE_ERR_LOG("Failed to uv_loop");
        return;
    }
    if (work == nullptr || work->data == nullptr) {
        RINGTONE_ERR_LOG("Failed to uv_work");
        return;
    }
    uv_queue_work(loop, work, [](uv_work_t *work) {
        RestoreBlock *block = reinterpret_cast<RestoreBlock *> (work->data);
        if (block == nullptr) {
            RINGTONE_ERR_LOG("Failed to new block");
            return;
        }
        auto restore = RingtoneRestoreFactory::CreateObj(RestoreSceneType(block->sceneCode));
        if (restore == nullptr) {
            RINGTONE_ERR_LOG("Failed to new restore");
            block->resultSet = E_FAIL;
            return;
        }
        block->resultSet = RingtoneRestore(restore, block->baseBackupPath);
    }, [](uv_work_t *work, int _status) {
        RestoreBlock *block = reinterpret_cast<RestoreBlock *> (work->data);
        if (block == nullptr) {
            RINGTONE_ERR_LOG("Failed to new block");
            delete work;
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(block->env, &scope);
        if (scope == nullptr) {
            RINGTONE_ERR_LOG("Failed to new scope");
            delete block;
            delete work;
            return;
        }
        napi_value restoreExResult = nullptr;
        RINGTONE_INFO_LOG("resultSet, %{public}d", block->resultSet);
        napi_create_int32(block->env, block->resultSet, &restoreExResult);
        napi_resolve_deferred(block->env, block->nativeDeferred, restoreExResult);
        napi_close_handle_scope(block->env, scope);
        delete block;
        delete work;
    });
}

napi_value RingtoneRestoreNapi::JSStartRestore(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    RINGTONE_INFO_LOG("JSStartRestore start");
    napi_get_undefined(env, &result);
    if (CheckPermission() != E_OK) {
        RINGTONE_ERR_LOG("check permission failed");
        return result;
    }

    size_t argc = 2;
    napi_value argv[2] = {0};
    napi_value thisVar = nullptr;

    void *data;
    const int32_t param = 2;
    napi_get_cb_info(env, info, &(argc), argv, &(thisVar), &(data));
    if (argc != param) {
        RINGTONE_ERR_LOG("require 2 parameters");
        return result;
    }
    napi_get_undefined(env, &result);
    int32_t sceneCode = GetIntFromParams(env, argv, 0);
    std::string baseBackupPath = GetStringFromParams(env, argv, 1);
    RINGTONE_INFO_LOG("sceneCode: %{public}d, backupPath: %{private}s", sceneCode, baseBackupPath.c_str());
    napi_deferred nativeDeferred = nullptr;
    uv_work_t *work = CreateUvWorkWithPromise(env, result, nativeDeferred);
    if (work == nullptr) {
        return result;
    }
    RestoreBlock *block = new (std::nothrow) RestoreBlock {
        env, sceneCode, baseBackupPath, E_OK, nativeDeferred };
    if (block == nullptr) {
        RINGTONE_ERR_LOG("Failed to new block");
        delete work;
        return result;
    }
    work->data = reinterpret_cast<void *>(block);
    UvQueueWork(GetUvLoop(env), work);
    RINGTONE_INFO_LOG("JSStartRestore end");
    return result;
}

void RingtoneRestoreNapi::UvQueueWorkCleanESim(uv_loop_s *loop, uv_work_t *work)
{
    if (loop == nullptr) {
        RINGTONE_ERR_LOG("Failed to uv_loop");
        return;
    }
    if (work == nullptr || work->data == nullptr) {
        RINGTONE_ERR_LOG("Failed to uv_work");
        return;
    }
    uv_queue_work(loop, work, [](uv_work_t *work) {
        CleanESimBlock *block = reinterpret_cast<CleanESimBlock *>(work->data);
        if (block == nullptr) {
            RINGTONE_ERR_LOG("Failed to new block");
            return;
        }
        block->resultSet = RingtoneRestoreDbUtils::CleanESimData(
            block->dbPath, block->peerSlotNum, block->ringtoneBasePath);
    }, [](uv_work_t *work, int _status) {
        CleanESimBlock *block = reinterpret_cast<CleanESimBlock *>(work->data);
        if (block == nullptr) {
            RINGTONE_ERR_LOG("Failed to new block");
            delete work;
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(block->env, &scope);
        if (scope == nullptr) {
            RINGTONE_ERR_LOG("Failed to new scope");
            delete block;
            delete work;
            return;
        }
        napi_value cleanResult = nullptr;
        RINGTONE_INFO_LOG("cleanESimData resultSet, %{public}d", block->resultSet);
        napi_create_int32(block->env, block->resultSet, &cleanResult);
        napi_resolve_deferred(block->env, block->nativeDeferred, cleanResult);
        napi_close_handle_scope(block->env, scope);
        delete block;
        delete work;
    });
}

napi_value RingtoneRestoreNapi::JSCleanESimData(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    RINGTONE_INFO_LOG("JSCleanESimData start");
    napi_get_undefined(env, &result);
    if (CheckPermission() != E_OK) {
        RINGTONE_ERR_LOG("check permission failed");
        return result;
    }

    size_t argc = 3;
    napi_value argv[3] = {0};
    napi_value thisVar = nullptr;

    void *data;
    const int32_t param = 3;
    napi_get_cb_info(env, info, &(argc), argv, &(thisVar), &(data));
    if (argc < param - 1) {
        RINGTONE_ERR_LOG("require at least 2 parameters");
        return result;
    }
    napi_get_undefined(env, &result);
    std::string dbPath = GetStringFromParams(env, argv, 0);
    int32_t peerSlotNum = GetIntFromParams(env, argv, 1);
    std::string ringtoneBasePath;
    if (argc >= param) {
        const size_t index = 2;
        ringtoneBasePath = GetStringFromParams(env, argv, index);
    }
    RINGTONE_INFO_LOG("dbPath: %{private}s, peerSlotNum: %{public}d, ringtoneBasePath: %{private}s",
        dbPath.c_str(), peerSlotNum, ringtoneBasePath.c_str());

    napi_deferred nativeDeferred = nullptr;
    uv_work_t *work = CreateUvWorkWithPromise(env, result, nativeDeferred);
    if (work == nullptr) {
        return result;
    }
    CleanESimBlock *block = new (std::nothrow) CleanESimBlock {
        env, dbPath, peerSlotNum, ringtoneBasePath, E_OK, nativeDeferred };
    if (block == nullptr) {
        RINGTONE_ERR_LOG("Failed to new block");
        delete work;
        return result;
    }
    work->data = reinterpret_cast<void *>(block);
    UvQueueWorkCleanESim(GetUvLoop(env), work);
    RINGTONE_INFO_LOG("JSCleanESimData end");
    return result;
}
} // namespace Media
} // namespace OHOS