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

#ifndef RINGTONE_LANGUAGE_MANAGER_H
#define RINGTONE_LANGUAGE_MANAGER_H

#include "result_set.h"
#include "values_bucket.h"

#include <cstdint>
#include <libxml/tree.h>
#include <memory>
#include <mutex>

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))

enum ResourceFileType {
    RINGTONE_FILE,
};

class RingtoneLanguageManager {
public:
    EXPORT RingtoneLanguageManager();
    EXPORT ~RingtoneLanguageManager();
    EXPORT static std::shared_ptr<RingtoneLanguageManager> GetInstance();

    EXPORT void SyncAssetLanguage();

private:
    EXPORT static std::string GetSystemLanguage();
    EXPORT void UpdateRingtoneLanguage();

    EXPORT int32_t CheckLanguageTypeByRingtone(int32_t &rowCount, std::shared_ptr<NativeRdb::ResultSet> &resultSet);
    EXPORT void ChangeLanguageDataToRingtone(int32_t rowCount, const std::shared_ptr<NativeRdb::ResultSet> &resultSet);
    EXPORT int32_t SetValuesFromResultSet(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
        const std::vector<std::pair<std::string, int>> &fieldIndex,
        NativeRdb::ValuesBucket &values, int32_t &ringtoneId);
    EXPORT static int32_t GetFieldIndex(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
        std::vector<std::pair<std::string, int>> &fieldIndex);

    EXPORT bool ReadMultilingualResources(const std::string &filePath, ResourceFileType resourceFileType);
    EXPORT bool ParseMultilingualXml(xmlNodePtr& parentNode, ResourceFileType resourceFileType);

    std::map<std::string, std::map<std::string, std::string>> ringtoneTranslate_;
    std::string systemLanguage_;
    static std::mutex mutex_;
    static std::shared_ptr<RingtoneLanguageManager> instance_;
};
} // namespace Media
} // namespace OHOS
#endif // RINGTONE_LANGUAGE_MANAGER_H
