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

#ifndef RING_MOCK_HAPTIC_AUDIO_METADATA_H
#define RING_MOCK_HAPTIC_AUDIO_METADATA_H

#include <string>
#include <unordered_map>
#include <variant>
#include "ringtone_db_const.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {

#define EXPORT __attribute__ ((visibility ("default")))

class RingMockHapticAudioMetadata {
public:
    EXPORT RingMockHapticAudioMetadata();
    EXPORT ~RingMockHapticAudioMetadata();
    using VariantData = std::variant<int32_t, std::string, int64_t, double>;

    EXPORT int32_t GetId() const;
    EXPORT std::string GetData() const;
    EXPORT int64_t GetSize() const;
    EXPORT std::string GetDisplayName() const;
    EXPORT std::string GetTitle() const;
    EXPORT int32_t GetRingMockHapticAudioType() const;
    EXPORT int32_t GetSourceType() const;
    EXPORT int64_t GetDateAdded() const;
    EXPORT int64_t GetDateModified() const;
    EXPORT int32_t GetPlayMode() const;
    EXPORT int32_t GetScannerFlag() const;

    EXPORT void SetId(const VariantData &id);
    EXPORT void SetData(const VariantData &data);
    EXPORT void SetSize(const VariantData &size);
    EXPORT void SetDisplayName(const VariantData &displayName);
    EXPORT void SetTitle(const VariantData &title);
    EXPORT void SetRingMockHapticAudioType(const VariantData &type);
    EXPORT void SetSourceType(const VariantData &type);
    EXPORT void SetDateAdded(const VariantData &date);
    EXPORT void SetDateModified(const VariantData &date);
    EXPORT void SetPlayMode(const VariantData &mode);
    EXPORT void SetScannerFlag(const VariantData &flag);

    EXPORT void Init();

    using RingMockHapticAudioMetadataFnPtr = void (RingMockHapticAudioMetadata::*)(const VariantData &);
    std::unordered_map<std::string, std::pair<RingtoneResultSetDataType,
        RingMockHapticAudioMetadataFnPtr>> memberFuncMap_;

private:
    int32_t id_ = 0;
    std::string data_;
    int64_t size_ = 0;
    std::string displayName_;
    std::string title_;
    int32_t ringMockHapticAudioType_ = 0;
    int32_t sourceType_ = 0;
    int64_t dateAdded_ = 0;
    int64_t dateModified_ = 0;
    int32_t playMode_ = 0;
    int32_t scannerFlag_ = 0;
};

} // namespace Media
} // namespace OHOS

#endif // RING_MOCK_HAPTIC_AUDIO_METADATA_H
