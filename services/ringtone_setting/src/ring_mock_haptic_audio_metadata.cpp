/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "ring_mock_haptic_audio_metadata.h"

namespace OHOS {
namespace Media {

RingMockHapticAudioMetadata::RingMockHapticAudioMetadata()
{
    Init();
}

RingMockHapticAudioMetadata::~RingMockHapticAudioMetadata() {}

void RingMockHapticAudioMetadata::Init()
{
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_ID] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingMockHapticAudioMetadata::SetId);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_DATA] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &RingMockHapticAudioMetadata::SetData);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_SIZE] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &RingMockHapticAudioMetadata::SetSize);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_DISPLAY_NAME] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &RingMockHapticAudioMetadata::SetDisplayName);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_TITLE] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &RingMockHapticAudioMetadata::SetTitle);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_HAPTIC_2_TONE_TYPE] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingMockHapticAudioMetadata::SetRingMockHapticAudioType);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_SOURCE_TYPE] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingMockHapticAudioMetadata::SetSourceType);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_DATE_ADDED] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &RingMockHapticAudioMetadata::SetDateAdded);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_DATE_MODIFIED] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &RingMockHapticAudioMetadata::SetDateModified);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_PLAY_MODE] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingMockHapticAudioMetadata::SetPlayMode);
    memberFuncMap_[HAPTIC_2_TONE_COLUMN_SCANNER_FLAG] = std::make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingMockHapticAudioMetadata::SetScannerFlag);
}

int32_t RingMockHapticAudioMetadata::GetId() const
{
    return id_;
}

std::string RingMockHapticAudioMetadata::GetData() const
{
    return data_;
}

int64_t RingMockHapticAudioMetadata::GetSize() const
{
    return size_;
}

std::string RingMockHapticAudioMetadata::GetDisplayName() const
{
    return displayName_;
}

std::string RingMockHapticAudioMetadata::GetTitle() const
{
    return title_;
}

int32_t RingMockHapticAudioMetadata::GetRingMockHapticAudioType() const
{
    return ringMockHapticAudioType_;
}

int32_t RingMockHapticAudioMetadata::GetSourceType() const
{
    return sourceType_;
}

int64_t RingMockHapticAudioMetadata::GetDateAdded() const
{
    return dateAdded_;
}

int64_t RingMockHapticAudioMetadata::GetDateModified() const
{
    return dateModified_;
}

int32_t RingMockHapticAudioMetadata::GetPlayMode() const
{
    return playMode_;
}

int32_t RingMockHapticAudioMetadata::GetScannerFlag() const
{
    return scannerFlag_;
}

void RingMockHapticAudioMetadata::SetId(const VariantData &id)
{
    id_ = std::get<int32_t>(id);
}

void RingMockHapticAudioMetadata::SetData(const VariantData &data)
{
    data_ = std::get<std::string>(data);
}

void RingMockHapticAudioMetadata::SetSize(const VariantData &size)
{
    size_ = std::get<int64_t>(size);
}

void RingMockHapticAudioMetadata::SetDisplayName(const VariantData &displayName)
{
    displayName_ = std::get<std::string>(displayName);
}

void RingMockHapticAudioMetadata::SetTitle(const VariantData &title)
{
    title_ = std::get<std::string>(title);
}

void RingMockHapticAudioMetadata::SetRingMockHapticAudioType(const VariantData &type)
{
    ringMockHapticAudioType_ = std::get<int32_t>(type);
}

void RingMockHapticAudioMetadata::SetSourceType(const VariantData &type)
{
    sourceType_ = std::get<int32_t>(type);
}

void RingMockHapticAudioMetadata::SetDateAdded(const VariantData &date)
{
    dateAdded_ = std::get<int64_t>(date);
}

void RingMockHapticAudioMetadata::SetDateModified(const VariantData &date)
{
    dateModified_ = std::get<int64_t>(date);
}

void RingMockHapticAudioMetadata::SetPlayMode(const VariantData &mode)
{
    playMode_ = std::get<int32_t>(mode);
}

void RingMockHapticAudioMetadata::SetScannerFlag(const VariantData &flag)
{
    scannerFlag_ = std::get<int32_t>(flag);
}

} // namespace Media
} // namespace OHOS
