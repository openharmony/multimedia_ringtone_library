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
#define MLOG_TAG "RingtonePrivacyManager"

#include "ringtone_privacy_manager.h"

#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
using namespace std;
using PrivacyRanges = vector<pair<uint32_t, uint32_t>>;
RingtonePrivacyManager::RingtonePrivacyManager(const string &path, const string &mode) : path_(path), mode_(mode)
{}

RingtonePrivacyManager::~RingtonePrivacyManager()
{}

const vector<string> EXIF_SUPPORTED_EXTENSION = {
    RINGTONE_CONTAINER_TYPE_MP3,
    RINGTONE_CONTAINER_TYPE_OGG
};

static bool IsTargetExtension(const string &path)
{
    const string ext = RingtoneFileUtils::GetExtensionFromPath(path);
    bool ret = find(EXIF_SUPPORTED_EXTENSION.begin(), EXIF_SUPPORTED_EXTENSION.end(), ext) !=
        EXIF_SUPPORTED_EXTENSION.end();
    if (!ret) {
        RINGTONE_ERR_LOG("invalid target extension:%{public}s", ext.c_str());
    }
    return ret;
}

/* Caller is responsible to close the returned fd */
static int32_t OpenOriginFd(const string &path, const string &mode)
{
    return RingtoneFileUtils::OpenFile(path, mode);
}

int32_t RingtonePrivacyManager::Open()
{
    if (!IsTargetExtension(path_)) {
        return E_INVALID_PATH;
    }
    return OpenOriginFd(path_, mode_);
}
} // namespace Media
} // namespace OHOS
