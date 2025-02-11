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

#ifndef OHOS_RINGTONE_ERRNO_H
#define OHOS_RINGTONE_ERRNO_H

#include <stdint.h>
#include <errno.h>

namespace OHOS {
namespace Media {
#define RINGTONE_ERR(offset, errCode) (-((offset) + (errCode)))

// common error code
// linux standard ERROR { 0, 200 }
constexpr int32_t E_OK = 0;
constexpr int32_t E_SUCCESS = 0;

constexpr int32_t E_ERR = -1;
constexpr int32_t E_PERMISSION_DENIED = -EACCES;
constexpr int32_t E_NO_SUCH_FILE      = -ENOENT;
constexpr int32_t E_FILE_EXIST        = -EEXIST;
constexpr int32_t E_NO_MEMORY         = -ENOMEM;
constexpr int32_t E_NO_SPACE          = -ENOSPC;
constexpr int32_t E_CHECK_SYSTEMAPP_FAIL = 202;

// ringtone inner common err { 200, 1999 }
constexpr int32_t E_COMMON_OFFSET           = 200;
constexpr int32_t E_COMMON_START            = RINGTONE_ERR(E_COMMON_OFFSET, 0);
constexpr int32_t E_FAIL                    = RINGTONE_ERR(E_COMMON_OFFSET, 0);
constexpr int32_t E_FILE_OPER_FAIL          = RINGTONE_ERR(E_COMMON_OFFSET, 1);
constexpr int32_t E_HAS_FS_ERROR            = RINGTONE_ERR(E_COMMON_OFFSET, 3);
constexpr int32_t E_CHECK_DIR_FAIL          = RINGTONE_ERR(E_COMMON_OFFSET, 4);
constexpr int32_t E_MODIFY_DATA_FAIL        = RINGTONE_ERR(E_COMMON_OFFSET, 5);
constexpr int32_t E_INVALID_VALUES          = RINGTONE_ERR(E_COMMON_OFFSET, 6);
constexpr int32_t E_INVALID_URI             = RINGTONE_ERR(E_COMMON_OFFSET, 7);
constexpr int32_t E_INVALID_FILEID          = RINGTONE_ERR(E_COMMON_OFFSET, 8);
constexpr int32_t E_INVALID_PATH            = RINGTONE_ERR(E_COMMON_OFFSET, 9);
constexpr int32_t E_VIOLATION_PARAMETERS    = RINGTONE_ERR(E_COMMON_OFFSET, 10);
constexpr int32_t E_RENAME_DIR_FAIL         = RINGTONE_ERR(E_COMMON_OFFSET, 11);
constexpr int32_t E_GET_VALUEBUCKET_FAIL    = RINGTONE_ERR(E_COMMON_OFFSET, 12);
constexpr int32_t E_GET_ASSET_FAIL          = RINGTONE_ERR(E_COMMON_OFFSET, 13);
constexpr int32_t E_GET_HASH_FAIL           = RINGTONE_ERR(E_COMMON_OFFSET, 14);
constexpr int32_t E_GET_CLIENTBUNDLE_FAIL   = RINGTONE_ERR(E_COMMON_OFFSET, 15);
constexpr int32_t E_EXIST_IN_DB             = RINGTONE_ERR(E_COMMON_OFFSET, 16);
constexpr int32_t E_INVALID_MODE            = RINGTONE_ERR(E_COMMON_OFFSET, 17);
constexpr int32_t E_INVALID_BUNDLENAME      = RINGTONE_ERR(E_COMMON_OFFSET, 18);
constexpr int32_t E_RENAME_FILE_FAIL        = RINGTONE_ERR(E_COMMON_OFFSET, 19);
constexpr int32_t E_DB_FAIL                 = RINGTONE_ERR(E_COMMON_OFFSET, 20);
constexpr int32_t E_DELETE_DENIED           = RINGTONE_ERR(E_COMMON_OFFSET, 21);
constexpr int32_t E_HAS_DB_ERROR            = RINGTONE_ERR(E_COMMON_OFFSET, 22);
constexpr int32_t E_INVALID_ARGUMENTS       = RINGTONE_ERR(E_COMMON_OFFSET, 23);
constexpr int32_t E_SQL_CHECK_FAIL          = RINGTONE_ERR(E_COMMON_OFFSET, 24);
constexpr int32_t E_IS_PENDING_ERROR        = RINGTONE_ERR(E_COMMON_OFFSET, 25);
constexpr int32_t E_NEED_FURTHER_CHECK      = RINGTONE_ERR(E_COMMON_OFFSET, 26);
constexpr int32_t E_CHECK_NATIVE_SA_FAIL    = RINGTONE_ERR(E_COMMON_OFFSET, 27);
constexpr int32_t E_INVALID_TIMESTAMP       = RINGTONE_ERR(E_COMMON_OFFSET, 28);
constexpr int32_t E_GRANT_URI_PERM_FAIL     = RINGTONE_ERR(E_COMMON_OFFSET, 29);
constexpr int32_t E_IS_IN_COMMIT            = RINGTONE_ERR(E_COMMON_OFFSET, 30);
constexpr int32_t E_IS_IN_REVERT            = RINGTONE_ERR(E_COMMON_OFFSET, 31);
constexpr int32_t E_GET_PRAMS_FAIL          = RINGTONE_ERR(E_COMMON_OFFSET, 32);
constexpr int32_t E_DATASHARE_END           = RINGTONE_ERR(E_COMMON_OFFSET, 33);
constexpr int32_t E_COMMON_END              = RINGTONE_ERR(E_COMMON_OFFSET, 1799);

// ringtone directory and file type control { 2000, 2099 }
constexpr int32_t E_DIR_CTRL_OFFSET                             = 2000;
constexpr int32_t E_CHECK_EXTENSION_FAIL                        = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 0);
constexpr int32_t E_DELETE_DIR_FAIL                             = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 1);
constexpr int32_t E_CHECK_RINGTONE_FAIL                         = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 2);
constexpr int32_t E_CHECK_ROOT_DIR_FAIL                         = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 3);
constexpr int32_t E_CHECK_RINGTONETYPE_MATCH_EXTENSION_FAIL     = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 4);
constexpr int32_t E_FILE_NAME_INVALID                           = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 5);
constexpr int32_t E_ALBUM_OPER_ERR                              = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 6);
constexpr int32_t E_DIR_OPER_ERR                                = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 7);
constexpr int32_t E_SAME_PATH                                   = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 8);
constexpr int32_t E_DIR_CHECK_DIR_FAIL                          = RINGTONE_ERR(E_DIR_CTRL_OFFSET, 9);

// ringtone recycle, trash { 2100, 2199 }
constexpr int32_t E_TRASH_OFFSET            = 2100;
constexpr int32_t E_RECYCLE_FILE_IS_NULL    = RINGTONE_ERR(E_TRASH_OFFSET, 0);
constexpr int32_t E_IS_RECYCLED             = RINGTONE_ERR(E_TRASH_OFFSET, 1);
constexpr int32_t E_RECYCLE_DIR_FAIL        = RINGTONE_ERR(E_TRASH_OFFSET, 2);
constexpr int32_t E_CREATE_TRASHDIR_FAIL    = RINGTONE_ERR(E_TRASH_OFFSET, 3);
constexpr int32_t E_MAKE_HASHNAME_FAIL      = RINGTONE_ERR(E_TRASH_OFFSET, 4);
constexpr int32_t E_GET_EXTENSION_FAIL      = RINGTONE_ERR(E_TRASH_OFFSET, 5);

// ringtone distributed { 2200, 2299 }
constexpr int32_t E_DISTRIBUTED_OFFSET      = 2200;
constexpr int32_t E_DEVICE_OPER_ERR         = RINGTONE_ERR(E_DISTRIBUTED_OFFSET, 0);

// ringtone scanner { 2400, 2499 }
constexpr int32_t E_SCANNER_OFFSET  = 2400;
constexpr int32_t E_FILE_HIDDEN     = RINGTONE_ERR(E_SCANNER_OFFSET, 0);
constexpr int32_t E_DIR_HIDDEN      = RINGTONE_ERR(E_SCANNER_OFFSET, 1);
constexpr int32_t E_RDB             = RINGTONE_ERR(E_SCANNER_OFFSET, 2);
constexpr int32_t E_DATA            = RINGTONE_ERR(E_SCANNER_OFFSET, 3);
constexpr int32_t E_SYSCALL         = RINGTONE_ERR(E_SCANNER_OFFSET, 4);
constexpr int32_t E_SCANNED         = RINGTONE_ERR(E_SCANNER_OFFSET, 5);
constexpr int32_t E_AVMETADATA      = RINGTONE_ERR(E_SCANNER_OFFSET, 6);
constexpr int32_t E_IMAGE           = RINGTONE_ERR(E_SCANNER_OFFSET, 7);
constexpr int32_t E_NO_RESULT       = RINGTONE_ERR(E_SCANNER_OFFSET, 8);
constexpr int32_t E_STOP            = RINGTONE_ERR(E_SCANNER_OFFSET, 9);
constexpr int32_t E_IS_PENDING      = RINGTONE_ERR(E_SCANNER_OFFSET, 10);

// ringtone file extension module error code { 3000, 3099 }
constexpr int32_t E_RINGTONE_FILE_OFFSET        = 3000;
constexpr int32_t E_URI_INVALID                 = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 0);
constexpr int32_t E_DISTIBUTED_URI_NO_SUPPORT   = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 1);
constexpr int32_t E_URI_IS_NOT_ALBUM            = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 2);
constexpr int32_t E_URI_IS_NOT_FILE             = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 3);
constexpr int32_t E_TWO_URI_ARE_THE_SAME        = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 4);
constexpr int32_t E_OPENFILE_INVALID_FLAG       = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 5);
constexpr int32_t E_INVALID_DISPLAY_NAME        = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 6);
constexpr int32_t E_DENIED_MOVE                 = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 7);
constexpr int32_t E_UPDATE_DB_FAIL              = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 8);
constexpr int32_t E_DENIED_RENAME               = RINGTONE_ERR(E_RINGTONE_FILE_OFFSET, 9);

// ringtone on notify module error code { 16000, 16099 }
constexpr int32_t E_RINGTONE_NOTIFY_OFFSET      = 16000;
constexpr int32_t E_ASYNC_WORKER_IS_NULL        = RINGTONE_ERR(E_RINGTONE_NOTIFY_OFFSET, 0);
constexpr int32_t E_NOTIFY_TASK_DATA_IS_NULL    = RINGTONE_ERR(E_RINGTONE_NOTIFY_OFFSET, 1);
constexpr int32_t E_SOLVE_URIS_FAILED           = RINGTONE_ERR(E_RINGTONE_NOTIFY_OFFSET, 2);
constexpr int32_t E_NOTIFY_CHANGE_EXT_FAILED    = RINGTONE_ERR(E_RINGTONE_NOTIFY_OFFSET, 3);
constexpr int32_t E_PARCEL_GET_SIZE_FAILED      = RINGTONE_ERR(E_RINGTONE_NOTIFY_OFFSET, 4);
constexpr int32_t E_GET_ALBUM_ID_FAILED         = RINGTONE_ERR(E_RINGTONE_NOTIFY_OFFSET, 5);
constexpr int32_t E_DO_NOT_NEDD_SEND_NOTIFY     = RINGTONE_ERR(E_RINGTONE_NOTIFY_OFFSET, 6);
} // namespace Media
} // namespace OHOS
#endif // OHOS_RINGTONE_ERRNO_H
