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

#include "ringtone_mimetype_utils.h"

#include <fstream>

#include "ringtone_log.h"
#include "ringtone_errno.h"

namespace OHOS {
namespace Media {
using namespace std;
using namespace nlohmann;

using MimeTypeMap = unordered_map<string, vector<string>>;

MimeTypeMap RingtoneMimeTypeUtils::mediaJsonMap_;
const string MIMETYPE_JSON_PATH = "/system/etc/userfilemanager/userfilemanager_mimetypes.json";
const string DEFAULT_MIME_TYPE = "application/octet-stream";

/**
 * The format of the target json file:
 * First floor: Media type string, such as image, video, audio, etc.
 * Second floor: Mime type string
 * Third floor: Extension array.
*/
void RingtoneMimeTypeUtils::CreateMapFromJson()
{
    std::ifstream jFile(MIMETYPE_JSON_PATH);
    if (!jFile.is_open()) {
        RINGTONE_ERR_LOG("Failed to open: %{private}s", MIMETYPE_JSON_PATH.c_str());
        return;
    }
    json firstFloorObjs;
    jFile >> firstFloorObjs;
    for (auto& firstFloorObj : firstFloorObjs.items()) {
        json secondFloorJsons = json::parse(firstFloorObj.value().dump(), nullptr, false);
        if (secondFloorJsons.is_discarded()) {
            continue;
        }
        for (auto &secondFloorJson : secondFloorJsons.items()) {
            json thirdFloorJsons = json::parse(secondFloorJson.value().dump(), nullptr, false);
            // Key: MimeType, Value: Extension array.
            if (!thirdFloorJsons.is_discarded()) {
                mediaJsonMap_.insert(std::pair<string, vector<string>>(secondFloorJson.key(), thirdFloorJsons));
            }
        }
    }
}

int32_t RingtoneMimeTypeUtils::InitMimeTypeMap()
{
    CreateMapFromJson();
    if (mediaJsonMap_.empty()) {
        RINGTONE_ERR_LOG("JsonMap is empty");
        return E_FAIL;
    }
    return E_OK;
}

string RingtoneMimeTypeUtils::GetMimeTypeFromExtension(const string &extension)
{
    return GetMimeTypeFromExtension(extension, mediaJsonMap_);
}

string RingtoneMimeTypeUtils::GetMimeTypeFromExtension(const string &extension,
    const MimeTypeMap &mimeTypeMap)
{
    string tmp = extension;
    transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    for (auto &item : mimeTypeMap) {
        for (auto &ext : item.second) {
            if (ext == tmp) {
                return item.first;
            }
        }
    }
    return DEFAULT_MIME_TYPE;
}

RingtoneMediaType RingtoneMimeTypeUtils::GetMediaTypeFromMimeType(const string &mimeType)
{
    size_t pos = mimeType.find_first_of("/");
    if (pos == string::npos) {
        RINGTONE_ERR_LOG("Invalid mime type: %{public}s", mimeType.c_str());
        return RINGTONE_MEDIA_TYPE_INVALID;
    }
    string prefix = mimeType.substr(0, pos);
    if (prefix == "audio") {
        return RINGTONE_MEDIA_TYPE_AUDIO;
    } else {
        RINGTONE_ERR_LOG("Invalid mime type: %{public}s", mimeType.c_str());
        return RINGTONE_MEDIA_TYPE_INVALID;
    }
}
} // namespace Media
} // namespace OHOS
