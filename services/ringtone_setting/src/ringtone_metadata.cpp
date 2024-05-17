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

#include "ringtone_metadata.h"

namespace OHOS {
namespace Media {
using namespace std;
RingtoneMetadata::RingtoneMetadata()
    : toneId_(TONE_ID_DEFAULT),
    data_(DATA_DEFAULT),
    size_(SIZE_DEFAULT),
    displayName_(DISPLAY_NAME_DEFAULT),
    title_(TITLE_DEFAULT),
    mediaType_(METADATA_MEDIA_TYPE_DEFAULT),
    toneType_(TONE_TYPE_DEFAULT),
    mimeType_(MIME_TYPE_DEFAULT),
    sourceType_(SOURCE_TYPE_DEFAULT),
    dateAdded_(DATE_ADDED_DEFAULT),
    dateModified_(DATE_MODIFIED_DEFAULT),
    dateTaken_(DATE_TAKEN_DEFAULT),
    duration_(DURATION_DEFAULT),
    shotToneType_(SHOT_TONE_TYPE_DEFAULT),
    shotToneSourceType_(SHOT_TONE_SOURCE_TYPE_DEFAULT),
    notificationToneType_(NOTIFICATION_TONE_TYPE_DEFAULT),
    notificationToneSourceType_(NOTIFICATION_TONE_SOURCE_TYPE_DEFAULT),
    ringToneType_(RING_TONE_TYPE_DEFAULT),
    ringToneSourceType_(RING_TONE_SOURCE_TYPE_DEFAULT),
    alarmToneType_(ALARM_TONE_TYPE_DEFAULT),
    alarmToneSourceType_(ALARM_TONE_SOURCE_TYPE_DEFAULT)
{
    Init();
}

void RingtoneMetadata::Init()
{
    memberFuncMap_[RINGTONE_COLUMN_TONE_ID] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetToneId);
    memberFuncMap_[RINGTONE_COLUMN_DATA] = make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &RingtoneMetadata::SetData);
    memberFuncMap_[RINGTONE_COLUMN_SIZE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &RingtoneMetadata::SetSize);
    memberFuncMap_[RINGTONE_COLUMN_DISPLAY_NAME] = make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &RingtoneMetadata::SetDisplayName);
    memberFuncMap_[RINGTONE_COLUMN_TITLE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &RingtoneMetadata::SetTitle);
    memberFuncMap_[RINGTONE_COLUMN_MEDIA_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetMediaType);
    memberFuncMap_[RINGTONE_COLUMN_TONE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetToneType);
    memberFuncMap_[RINGTONE_COLUMN_MIME_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_STRING,
        &RingtoneMetadata::SetMimeType);
    memberFuncMap_[RINGTONE_COLUMN_SOURCE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetSourceType);
    memberFuncMap_[RINGTONE_COLUMN_DATE_ADDED] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &RingtoneMetadata::SetDateAdded);
    memberFuncMap_[RINGTONE_COLUMN_DATE_MODIFIED] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &RingtoneMetadata::SetDateModified);
    memberFuncMap_[RINGTONE_COLUMN_DATE_TAKEN] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT64,
        &RingtoneMetadata::SetDateTaken);
    memberFuncMap_[RINGTONE_COLUMN_DURATION] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetDuration);
    memberFuncMap_[RINGTONE_COLUMN_SHOT_TONE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetShotToneType);
    memberFuncMap_[RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetShotToneSourceType);
    memberFuncMap_[RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetNotificationToneType);
    memberFuncMap_[RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE] =
        make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32, &RingtoneMetadata::SetNotificationToneSourceType);
    memberFuncMap_[RINGTONE_COLUMN_RING_TONE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetRingToneType);
    memberFuncMap_[RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetRingToneSourceType);
    memberFuncMap_[RINGTONE_COLUMN_ALARM_TONE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetAlarmToneType);
    memberFuncMap_[RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE] = make_pair(RingtoneResultSetDataType::DATA_TYPE_INT32,
        &RingtoneMetadata::SetAlarmToneSourceType);
}

void RingtoneMetadata::SetToneId(const VariantData &toneId)
{
    toneId_ = std::get<int32_t>(toneId);
}

int32_t RingtoneMetadata::GetToneId() const
{
    return toneId_;
}

void RingtoneMetadata::SetData(const VariantData &data)
{
    data_ = std::get<std::string>(data);
}

const std::string &RingtoneMetadata::GetData() const
{
    return data_;
}

void RingtoneMetadata::SetSize(const VariantData &size)
{
    size_ = std::get<int64_t>(size);
}

int64_t RingtoneMetadata::GetSize() const
{
    return size_;
}

void RingtoneMetadata::SetDisplayName(const VariantData &displayName)
{
    displayName_ = std::get<std::string>(displayName);
}

const std::string &RingtoneMetadata::GetDisplayName() const
{
    return displayName_;
}

void RingtoneMetadata::SetTitle(const VariantData &title)
{
    title_ = std::get<std::string>(title);
}

const std::string &RingtoneMetadata::GetTitle() const
{
    return title_;
}

void RingtoneMetadata::SetMediaType(const VariantData &mediaType)
{
    mediaType_ = std::get<int32_t>(mediaType);
}

int32_t RingtoneMetadata::GetMediaType() const
{
    return mediaType_;
}

void RingtoneMetadata::SetToneType(const VariantData &toneType)
{
    toneType_ = std::get<int32_t>(toneType);
}

int32_t RingtoneMetadata::GetToneType() const
{
    return toneType_;
}

void RingtoneMetadata::SetMimeType(const VariantData &mimeType)
{
    mimeType_ = std::get<std::string>(mimeType);
}

const std::string &RingtoneMetadata::GetMimeType() const
{
    return mimeType_;
}

void RingtoneMetadata::SetSourceType(const VariantData &sourceType)
{
    sourceType_ = std::get<int32_t>(sourceType);
}

int32_t RingtoneMetadata::GetSourceType() const
{
    return sourceType_;
}

void RingtoneMetadata::SetDateAdded(const VariantData &dateAdded)
{
    dateAdded_ = std::get<int64_t>(dateAdded);
}

int64_t RingtoneMetadata::GetDateAdded() const
{
    return dateAdded_;
}

void RingtoneMetadata::SetDateModified(const VariantData &dateModified)
{
    dateModified_ = std::get<int64_t>(dateModified);
}

int64_t RingtoneMetadata::GetDateModified() const
{
    return dateModified_;
}

void RingtoneMetadata::SetDateTaken(const VariantData &dateTaken)
{
    dateTaken_ = std::get<int64_t>(dateTaken);
}

int64_t RingtoneMetadata::GetDateTaken() const
{
    return dateTaken_;
}

void RingtoneMetadata::SetDuration(const VariantData &duration)
{
    duration_ = std::get<int32_t>(duration);
}

int32_t RingtoneMetadata::GetDuration() const
{
    return duration_;
}

void RingtoneMetadata::SetShotToneType(const VariantData &shotToneType)
{
    shotToneType_ = std::get<int32_t>(shotToneType);
}

int32_t RingtoneMetadata::GetShotToneType() const
{
    return shotToneType_;
}

void RingtoneMetadata::SetShotToneSourceType(const VariantData &shotToneSourceType)
{
    shotToneSourceType_ = std::get<int32_t>(shotToneSourceType);
}

int32_t RingtoneMetadata::GetShotToneSourceType() const
{
    return shotToneSourceType_;
}

void RingtoneMetadata::SetNotificationToneType(const VariantData &notificationToneType)
{
    notificationToneType_ = std::get<int32_t>(notificationToneType);
}

int32_t RingtoneMetadata::GetNotificationToneType() const
{
    return notificationToneType_;
}

void RingtoneMetadata::SetNotificationToneSourceType(const VariantData &notificationToneSourceType)
{
    notificationToneSourceType_ = std::get<int32_t>(notificationToneSourceType);
}

int32_t RingtoneMetadata::GetNotificationToneSourceType() const
{
    return notificationToneSourceType_;
}

void RingtoneMetadata::SetRingToneType(const VariantData &ringToneType)
{
    ringToneType_ = std::get<int32_t>(ringToneType);
}

int32_t RingtoneMetadata::GetRingToneType() const
{
    return ringToneType_;
}

void RingtoneMetadata::SetRingToneSourceType(const VariantData &ringToneSourceType)
{
    ringToneSourceType_ = std::get<int32_t>(ringToneSourceType);
}

int32_t RingtoneMetadata::GetRingToneSourceType() const
{
    return ringToneSourceType_;
}

void RingtoneMetadata::SetAlarmToneType(const VariantData &alarmToneType)
{
    alarmToneType_ = std::get<int32_t>(alarmToneType);
}

int32_t RingtoneMetadata::GetAlarmToneType() const
{
    return alarmToneType_;
}

void RingtoneMetadata::SetAlarmToneSourceType(const VariantData &alarmToneSourceType)
{
    alarmToneSourceType_ = std::get<int32_t>(alarmToneSourceType);
}

int32_t RingtoneMetadata::GetAlarmToneSourceType() const
{
    return alarmToneSourceType_;
}

string RingtoneMetadata::ToString() const
{
    string str = "ringtone metadata:{ toneId_=" + to_string(toneId_) +
                "; data_=" + data_ +
                "; size_=" + to_string(size_) +
                "; displayName_=" + displayName_ +
                "; title_=" + title_ +
                "; mediaType_=" + to_string(mediaType_) +
                "; toneType_=" + to_string(toneType_) +
                "; mimeType_=" + mimeType_ +
                "; sourceType_=" + to_string(sourceType_) +
                "; dateAdded_=" + to_string(dateAdded_) +
                "; dateModified_=" + to_string(dateModified_) +
                "; dateTaken_=" + to_string(dateTaken_) +
                "; duration_=" + to_string(duration_) +
                "; shotToneType_=" + to_string(shotToneType_) +
                "; shotToneSourceType_=" + to_string(shotToneSourceType_) +
                "; notificationToneType_=" + to_string(notificationToneType_) +
                "; notificationToneSourceType_=" + to_string(notificationToneSourceType_) +
                "; ringToneType_=" + to_string(ringToneType_) +
                "; ringToneSourceType_=" + to_string(ringToneSourceType_) +
                "; alarmToneType_=" + to_string(alarmToneType_) +
                "; alarmToneSourceType_=" + to_string(alarmToneSourceType_) + "}";
    return str;
}
} // namespace Media
} // namespace OHOS
