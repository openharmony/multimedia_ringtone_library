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
#include "ringtone_log.h"
#include "ringtone_errno.h"
#include "ringtone_db_const.h"
#include "application_context.h"
#include "js_native_api.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#define MLOG_TAG "Common"

namespace OHOS {
namespace Media {
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

static int32_t CheckPermission(void)
{
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        RINGTONE_ERR_LOG("CheckPermission context nullptr");
        return E_FAIL;
    }
    std::string bundleName = context->GetBundleName();
    if (bundleName.compare(BUNDLE_NAME) != 0) {
        RINGTONE_ERR_LOG("bundle name is invalid: %{public}s", bundleName.c_str());
        return E_FAIL;
    }
    RINGTONE_INFO_LOG("CheckPermission success");
    return E_OK;
}

napi_value RingtoneRestoreNapi::JSStartRestore(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    RINGTONE_INFO_LOG("JSStartRestore start");
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
    int32_t scenceCode = GetIntFromParams(env, argv, 0);
    RINGTONE_INFO_LOG("scenceCode: %{public}d", scenceCode);
    RINGTONE_INFO_LOG("JSStartRestore end");
    return result;
}
} // namespace Media
} // namespace OHOS