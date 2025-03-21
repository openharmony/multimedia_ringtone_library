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

#include "vibrate_metadata.h"

namespace OHOS {
namespace Media {
using namespace std;
VibrateMetadata::VibrateMetadata()
    : vibrateId_(VIBRATE_ID_DEFAULT),
    data_(DATA_DEFAULT),
    size_(SIZE_DEFAULT),
    displayName_(DISPLAY_NAME_DEFAULT),
    title_(TITLE_DEFAULT),
    displayLanguage_(DISPLAY_LANGUAGE_DEFAULT),
    vibrateType_(VIBRATE_TYPE_DEFAULT),
    sourceType_(SOURCE_TYPE_DEFAULT),
    dateAdded_(DATE_ADDED_DEFAULT),
    dateModified_(DATE_MODIFIED_DEFAULT),
    dateTaken_(DATE_TAKEN_DEFAULT),
    vibratePlayMode_(VIBRATE_PLAYMODE_NONE),
    scannerFlag_(SCANNER_FLAG_DEFAULT)
{
    Init();
}

void VibrateMetadata::Init()
{
    memberFuncMap_[VIBRATE_COLUMN_VIBRATE_ID] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &VibrateMetadata::SetVibrateId);
    memberFuncMap_[VIBRATE_COLUMN_DATA] = make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &VibrateMetadata::SetData);
    memberFuncMap_[VIBRATE_COLUMN_SIZE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &VibrateMetadata::SetSize);
    memberFuncMap_[VIBRATE_COLUMN_DISPLAY_NAME] = make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &VibrateMetadata::SetDisplayName);
    memberFuncMap_[VIBRATE_COLUMN_TITLE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &VibrateMetadata::SetTitle);
    memberFuncMap_[VIBRATE_COLUMN_DISPLAY_LANGUAGE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &VibrateMetadata::SetDisplayLanguage);
    memberFuncMap_[VIBRATE_COLUMN_VIBRATE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &VibrateMetadata::SetVibrateType);
    memberFuncMap_[VIBRATE_COLUMN_SOURCE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &VibrateMetadata::SetSourceType);
    memberFuncMap_[VIBRATE_COLUMN_DATE_ADDED] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &VibrateMetadata::SetDateAdded);
    memberFuncMap_[VIBRATE_COLUMN_DATE_MODIFIED] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &VibrateMetadata::SetDateModified);
    memberFuncMap_[VIBRATE_COLUMN_DATE_TAKEN] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &VibrateMetadata::SetDateTaken);
    memberFuncMap_[VIBRATE_COLUMN_PLAY_MODE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &VibrateMetadata::SetPlayMode);
    memberFuncMap_[VIBRATE_COLUMN_SCANNER_FLAG] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &VibrateMetadata::SetScannerFlag);
}

void VibrateMetadata::SetVibrateId(const VariantData &vibrateId)
{
    vibrateId_ = std::get<int32_t>(vibrateId);
}

int32_t VibrateMetadata::GetVibrateId() const
{
    return vibrateId_;
}

void VibrateMetadata::SetData(const VariantData &data)
{
    data_ = std::get<std::string>(data);
}

const std::string &VibrateMetadata::GetData() const
{
    return data_;
}

void VibrateMetadata::SetSize(const VariantData &size)
{
    size_ = std::get<int64_t>(size);
}

int64_t VibrateMetadata::GetSize() const
{
    return size_;
}

void VibrateMetadata::SetDisplayName(const VariantData &displayName)
{
    displayName_ = std::get<std::string>(displayName);
}

const std::string &VibrateMetadata::GetDisplayName() const
{
    return displayName_;
}

void VibrateMetadata::SetTitle(const VariantData &title)
{
    title_ = std::get<std::string>(title);
}

const std::string &VibrateMetadata::GetTitle() const
{
    return title_;
}

void VibrateMetadata::SetDisplayLanguage(const VariantData &displayLanguage)
{
    displayLanguage_ = std::get<std::string>(displayLanguage);
}

const std::string &VibrateMetadata::GetDisplayLanguage() const
{
    return displayLanguage_;
}

void VibrateMetadata::SetVibrateType(const VariantData &vibrateType)
{
    vibrateType_ = std::get<int32_t>(vibrateType);
}

int32_t VibrateMetadata::GetVibrateType() const
{
    return vibrateType_;
}

void VibrateMetadata::SetSourceType(const VariantData &sourceType)
{
    sourceType_ = std::get<int32_t>(sourceType);
}

int32_t VibrateMetadata::GetSourceType() const
{
    return sourceType_;
}

void VibrateMetadata::SetDateAdded(const VariantData &dateAdded)
{
    dateAdded_ = std::get<int64_t>(dateAdded);
}

int64_t VibrateMetadata::GetDateAdded() const
{
    return dateAdded_;
}

void VibrateMetadata::SetDateModified(const VariantData &dateModified)
{
    dateModified_ = std::get<int64_t>(dateModified);
}

int64_t VibrateMetadata::GetDateModified() const
{
    return dateModified_;
}

void VibrateMetadata::SetDateTaken(const VariantData &dateTaken)
{
    dateTaken_ = std::get<int64_t>(dateTaken);
}

int64_t VibrateMetadata::GetDateTaken() const
{
    return dateTaken_;
}

void VibrateMetadata::SetPlayMode(const VariantData &playMode)
{
    vibratePlayMode_ = std::get<int32_t>(playMode);
}

int32_t VibrateMetadata::GetPlayMode() const
{
    return vibratePlayMode_;
}

void VibrateMetadata::SetScannerFlag(const VariantData &scannerFlag)
{
    scannerFlag_ = std::get<int32_t>(scannerFlag);
}

int32_t VibrateMetadata::GetScannerFlag() const
{
    return scannerFlag_;
}

string VibrateMetadata::ToString() const
{
    string str = "vibrate metadata:{ vibrateId_=" + to_string(vibrateId_) +
                "; data_=" + data_ +
                "; size_=" + to_string(size_) +
                "; displayName_=" + displayName_ +
                "; title_=" + title_ +
                "; displayLanguage_=" + displayLanguage_ +
                "; vibrateType_=" + to_string(vibrateType_) +
                "; sourceType_=" + to_string(sourceType_) +
                "; dateAdded_=" + to_string(dateAdded_) +
                "; dateModified_=" + to_string(dateModified_) +
                "; dateTaken_=" + to_string(dateTaken_) +
                "; scannerFlag_=" + to_string(scannerFlag_) + "}";
    return str;
}
} // namespace Media
} // namespace OHOS
