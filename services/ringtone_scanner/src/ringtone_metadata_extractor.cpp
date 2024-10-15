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
#define MLOG_TAG "Scanner"

#include "ringtone_metadata_extractor.h"

#include <fcntl.h>
#include <sys/stat.h>

#include "directory_ex.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_errno.h"

namespace OHOS {
namespace Media {
using namespace std;

template <class Type>
static Type stringToNum(const string &str)
{
    std::istringstream iss(str);
    Type num;
    iss >> num;
    return num;
}

static time_t convertTimeStr2TimeStamp(string &timeStr)
{
    struct tm timeinfo;
    strptime(timeStr.c_str(), "%Y-%m-%d %H:%M:%S",  &timeinfo);
    time_t timeStamp = mktime(&timeinfo);
    return timeStamp;
}

void PopulateExtractedAVMetadataOne(const std::unordered_map<int32_t, std::string> &resultMap,
    std::unique_ptr<RingtoneMetadata> &data)
{
    int32_t intTempMeta;

    string strTemp = resultMap.at(Media::AV_KEY_DURATION);
    if (strTemp != "") {
        intTempMeta = stringToNum<int32_t>(strTemp);
        data->SetDuration(intTempMeta);
    }

    strTemp = resultMap.at(Media::AV_KEY_MIME_TYPE);
    if (strTemp != "") {
        data->SetMimeType(strTemp);
    }
}

void PopulateExtractedAVMetadataTwo(const std::unordered_map<int32_t, std::string> &resultMap,
    std::unique_ptr<RingtoneMetadata> &data)
{
    string strTemp = resultMap.at(Media::AV_KEY_DATE_TIME_FORMAT);
    if (strTemp != "") {
        int64_t int64TempMeta = convertTimeStr2TimeStamp(strTemp);
        if (int64TempMeta < 0) {
            data->SetDateTaken(data->GetDateModified() / MSEC_TO_SEC);
        } else {
            data->SetDateTaken(int64TempMeta);
        }
    } else {
        // use modified time as date taken time when date taken not set
        data->SetDateTaken(data->GetDateModified() / MSEC_TO_SEC);
    }

    strTemp = resultMap.at(Media::AV_KEY_TITLE);
    if (!strTemp.empty()) {
        data->SetTitle(strTemp);
    }
}

void RingtoneMetadataExtractor::FillExtractedMetadata(const std::unordered_map<int32_t, std::string> &resultMap,
    std::unique_ptr<RingtoneMetadata> &data)
{
    PopulateExtractedAVMetadataOne(resultMap, data);
    PopulateExtractedAVMetadataTwo(resultMap, data);
}

int32_t RingtoneMetadataExtractor::ExtractAudioMetadata(std::unique_ptr<RingtoneMetadata> &data)
{
    std::shared_ptr<Media::AVMetadataHelper> avMetadataHelper =
        Media::AVMetadataHelperFactory::CreateAVMetadataHelper();
    if (avMetadataHelper == nullptr) {
        RINGTONE_ERR_LOG("AV RingtoneMetadata helper is null");
        return E_AVMETADATA;
    }

    string absFilePath;
    if (!PathToRealPath(data->GetData(), absFilePath)) {
        RINGTONE_ERR_LOG("AV RingtoneMetadata is not real path, file path: %{public}s", data->GetData().c_str());
        return E_AVMETADATA;
    }
    if (absFilePath.empty()) {
        RINGTONE_ERR_LOG("Failed to obtain the canonical path for source path: %{public}s %{public}d",
            absFilePath.c_str(), errno);
        return E_AVMETADATA;
    }

    int32_t fd = open(absFilePath.c_str(), O_RDONLY);
    if (fd <= 0) {
        RINGTONE_ERR_LOG("Open file descriptor failed, errno = %{public}d", errno);
        return E_SYSCALL;
    }
    struct stat64 st;
    if (fstat64(fd, &st) != 0) {
        RINGTONE_ERR_LOG("Get file state failed for the given fd");
        (void)close(fd);
        return E_SYSCALL;
    }

    int32_t err = avMetadataHelper->SetSource(fd, 0, static_cast<int64_t>(st.st_size), Media::AV_META_USAGE_META_ONLY);
    if (err != 0) {
        RINGTONE_ERR_LOG("SetSource failed for the given file descriptor, err = %{public}d", err);
        (void)close(fd);
        return E_AVMETADATA;
    } else {
        std::unordered_map<int32_t, std::string> resultMap = avMetadataHelper->ResolveMetadata();
        if (!resultMap.empty()) {
            FillExtractedMetadata(resultMap, data);
        }
    }

    (void)close(fd);

    return E_OK;
}

int32_t RingtoneMetadataExtractor::Extract(std::unique_ptr<RingtoneMetadata> &data)
{
    return ExtractAudioMetadata(data);
}
} // namespace Media
} // namespace OHOS
