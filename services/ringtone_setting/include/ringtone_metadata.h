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

#ifndef RINGTONE_METADATA_H
#define RINGTONE_METADATA_H

#include "ringtone_db_const.h"
#include "ringtone_type.h"


namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))

class RingtoneMetadata {
public:
    EXPORT RingtoneMetadata();
    EXPORT ~RingtoneMetadata() = default;
    using VariantData = std::variant<int32_t, std::string, int64_t, double>;

    EXPORT void SetToneId(const VariantData &toneId);
    EXPORT int32_t GetToneId() const;

    EXPORT void SetData(const VariantData &data);
    EXPORT const std::string &GetData() const;

    EXPORT void SetSize(const VariantData &size);
    EXPORT int64_t GetSize() const;

    EXPORT void SetDisplayName(const VariantData &displayName);
    EXPORT const std::string &GetDisplayName() const;

    EXPORT void SetTitle(const VariantData &title);
    EXPORT const std::string &GetTitle() const;

    EXPORT void SetMediaType(const VariantData &mediaType);
    EXPORT int32_t GetMediaType() const;

    EXPORT void SetToneType(const VariantData &toneType);
    EXPORT int32_t GetToneType() const;

    EXPORT void SetMimeType(const VariantData &mimeType);
    EXPORT const std::string &GetMimeType() const;

    EXPORT void SetSourceType(const VariantData &toneType);
    EXPORT int32_t GetSourceType() const;

    EXPORT void SetDateAdded(const VariantData &dateAdded);
    EXPORT int64_t GetDateAdded() const;

    EXPORT void SetDateModified(const VariantData &dateModified);
    EXPORT int64_t GetDateModified() const;

    EXPORT void SetDateTaken(const VariantData &dateTaken);
    EXPORT int64_t GetDateTaken() const;

    EXPORT void SetDuration(const VariantData &duration);
    EXPORT int32_t GetDuration() const;

    EXPORT void SetShotToneType(const VariantData &shotToneType);
    EXPORT int32_t GetShotToneType() const;

    EXPORT void SetShotToneSourceType(const VariantData &shotToneSourceType);
    EXPORT int32_t GetShotToneSourceType() const;

    EXPORT void SetNotificationToneType(const VariantData &notificationToneType);
    EXPORT int32_t GetNotificationToneType() const;

    EXPORT void SetNotificationToneSourceType(const VariantData &notificationToneSourceType);
    EXPORT int32_t GetNotificationToneSourceType() const;

    EXPORT void SetRingToneType(const VariantData &ringToneType);
    EXPORT int32_t GetRingToneType() const;

    EXPORT void SetRingToneSourceType(const VariantData &ringToneSourceType);
    EXPORT int32_t GetRingToneSourceType() const;

    EXPORT void SetAlarmToneType(const VariantData &alarmToneType);
    EXPORT int32_t GetAlarmToneType() const;

    EXPORT void SetAlarmToneSourceType(const VariantData &alarmToneSourceType);
    EXPORT int32_t GetAlarmToneSourceType() const;

    EXPORT void SetDefaultSystemtoneType(const VariantData &defaultSystemtoneType);
    EXPORT int32_t GetDefaultSystemtoneType() const;

    EXPORT void Init();

    using RingtoneMetadataFnPtr = void (RingtoneMetadata::*)(const VariantData &);
    std::unordered_map<std::string, std::pair<RingtoneResultSetDataType, RingtoneMetadataFnPtr>> memberFuncMap_;

    EXPORT std::string ToString() const;
private:
    int32_t toneId_;
    std::string data_;
    int64_t size_;
    std::string displayName_;
    std::string title_;
    int32_t mediaType_;
    int32_t toneType_;
    std::string mimeType_;
    int32_t sourceType_;
    int64_t dateAdded_;
    int64_t dateModified_;
    int64_t dateTaken_;
    int32_t duration_;
    int32_t shotToneType_;
    int32_t shotToneSourceType_;
    int32_t notificationToneType_;
    int32_t notificationToneSourceType_;
    int32_t ringToneType_;
    int32_t ringToneSourceType_;
    int32_t alarmToneType_;
    int32_t alarmToneSourceType_;
    int32_t defaultSystemtoneType_;
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_METADATA_H
