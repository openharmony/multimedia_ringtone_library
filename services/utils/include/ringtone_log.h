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

#ifndef OHOS_RINGTONE_LOG_H
#define OHOS_RINGTONE_LOG_H

#ifndef MLOG_TAG
#define MLOG_TAG "Common"
#endif

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B76

#undef LOG_TAG
#define LOG_TAG "RingtoneLibrary"

#ifndef LOG_LABEL
#define LOG_LABEL { LOG_CORE, LOG_DOMAIN, LOG_TAG }
#endif

#include "hilog/log.h"

#define RINGTONE_HILOG(op, fmt, args...) \
    do {                                  \
        op(LOG_LABEL, MLOG_TAG ":{%{public}s:%{public}d} " fmt, __FUNCTION__, __LINE__, ##args);  \
    } while (0)

#define RINGTONE_DEBUG_LOG(fmt, ...) HILOG_IMPL(LOG_CORE, LOG_DEBUG, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)
#define RINGTONE_ERR_LOG(fmt, ...) HILOG_IMPL(LOG_CORE, LOG_ERROR, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)
#define RINGTONE_WARN_LOG(fmt, ...) HILOG_IMPL(LOG_CORE, LOG_WARN, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)
#define RINGTONE_INFO_LOG(fmt, ...) HILOG_IMPL(LOG_CORE, LOG_INFO, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)
#define RINGTONE_FATAL_LOG(fmt, ...) HILOG_IMPL(LOG_CORE, LOG_FATAL, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)

#define CHECK_AND_RETURN_RET_LOG(cond, ret, fmt, ...)  \
    do {                                               \
        if (!(cond)) {                                 \
            RINGTONE_ERR_LOG(fmt, ##__VA_ARGS__);         \
            return ret;                                \
        }                                              \
    } while (0)

#define CHECK_AND_RETURN_LOG(cond, fmt, ...)           \
    do {                                               \
        if (!(cond)) {                                 \
            RINGTONE_ERR_LOG(fmt, ##__VA_ARGS__);         \
            return;                                    \
        }                                              \
    } while (0)

#define CHECK_AND_PRINT_LOG(cond, fmt, ...)            \
    do {                                               \
        if (!(cond)) {                                 \
            RINGTONE_ERR_LOG(fmt, ##__VA_ARGS__);         \
        }                                              \
    } while (0)

#define CHECK_AND_RETURN_RET(cond, ret)  \
    do {                                               \
        if (!(cond)) {                                 \
            return ret;                                \
        }                                              \
    } while (0)

#endif // OHOS_RINGTONE_LOG_H
