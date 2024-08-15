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

napi_value RingtoneRestoreNapi::Init(napi_env env, napi_value exports)
{
    RINGTONE_INFO_LOG("Init");
    napi_property_descriptor ringtone_restore_properties[] = {
        DECLARE_NAPI_FUNCTION("startRestore", JSStartRestore)
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
            block->resultSet = E_FAIL;
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

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    if (loop == nullptr) {
        RINGTONE_ERR_LOG("Failed to new uv_loop");
        return result;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        RINGTONE_ERR_LOG("Failed to new uv_work");
        return result;
    }
    int32_t resultSet = E_OK;
    napi_deferred nativeDeferred = nullptr;
    napi_create_promise(env, &nativeDeferred, &result);
    RestoreBlock *block = new (std::nothrow) RestoreBlock {
        env, sceneCode, baseBackupPath, resultSet, nativeDeferred };
    if (block == nullptr) {
        RINGTONE_ERR_LOG("Failed to new block");
        delete work;
        return result;
    }
    work->data = reinterpret_cast<void *>(block);
    UvQueueWork(loop, work);
    RINGTONE_INFO_LOG("JSStartRestore end");
    return result;
}
} // namespace Media
} // namespace OHOS