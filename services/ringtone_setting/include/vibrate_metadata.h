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

#ifndef VIBRATE_METADATA_H
#define VIBRATE_METADATA_H

#include "ringtone_db_const.h"
#include "ringtone_type.h"
#include "vibrate_type.h"


namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))

class VibrateMetadata {
public:
    EXPORT VibrateMetadata();
    EXPORT ~VibrateMetadata() = default;
    using VariantData = std::variant<int32_t, std::string, int64_t, double>;

    EXPORT void SetVibrateId(const VariantData &vibrateId);
    EXPORT int32_t GetVibrateId() const;

    EXPORT void SetData(const VariantData &data);
    EXPORT const std::string &GetData() const;

    EXPORT void SetSize(const VariantData &size);
    EXPORT int64_t GetSize() const;

    EXPORT void SetDisplayName(const VariantData &displayName);
    EXPORT const std::string &GetDisplayName() const;

    EXPORT void SetTitle(const VariantData &title);
    EXPORT const std::string &GetTitle() const;

    EXPORT void SetDisplayLanguage(const VariantData &displayLanguage);
    EXPORT const std::string &GetDisplayLanguage() const;

    EXPORT void SetVibrateType(const VariantData &vibrateType);
    EXPORT int32_t GetVibrateType() const;

    EXPORT void SetSourceType(const VariantData &sourceType);
    EXPORT int32_t GetSourceType() const;

    EXPORT void SetDateAdded(const VariantData &dateAdded);
    EXPORT int64_t GetDateAdded() const;

    EXPORT void SetDateModified(const VariantData &dateModified);
    EXPORT int64_t GetDateModified() const;

    EXPORT void SetDateTaken(const VariantData &dateTaken);
    EXPORT int64_t GetDateTaken() const;

    EXPORT void SetPlayMode(const VariantData &dateTaken);
    EXPORT int32_t GetPlayMode() const;

    EXPORT void Init();

    using VibrateMetadataFnPtr = void (VibrateMetadata::*)(const VariantData &);
    std::unordered_map<std::string, std::pair<RingtoneResultSetDataType, VibrateMetadataFnPtr>> memberFuncMap_;

    EXPORT std::string ToString() const;
private:
    int32_t vibrateId_;
    std::string data_;
    int64_t size_;
    std::string displayName_;
    std::string title_;
    std::string displayLanguage_;
    int32_t vibrateType_;
    int32_t sourceType_;
    int64_t dateAdded_;
    int64_t dateModified_;
    int64_t dateTaken_;
    int32_t vibratePlayMode_;
};
} // namespace Media
} // namespace OHOS

#endif // VIBRATE_METADATA_H
