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

#ifndef RINGTONE_EXTRACTOR_H
#define RINGTONE_EXTRACTOR_H

#include <sstream>

#include "avmetadatahelper.h"
#include "ringtone_metadata.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class RingtoneMetadataExtractor {
public:
    EXPORT static int32_t Extract(std::unique_ptr<RingtoneMetadata> &data);
    EXPORT static int32_t ExtractAudioMetadata(std::unique_ptr<RingtoneMetadata> &data);

private:
    RingtoneMetadataExtractor() = delete;
    ~RingtoneMetadataExtractor() = delete;

    EXPORT static void FillExtractedMetadata(const std::unordered_map<int32_t, std::string> &metadataMap,
        std::unique_ptr<RingtoneMetadata> &data);
};
} // namespace Media
} // namespace OHOS
#endif /* RINGTONE_EXTRACTOR_H */
