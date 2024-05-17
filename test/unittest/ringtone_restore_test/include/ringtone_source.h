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

#ifndef RINGTONELIBRARY_SOURCE_H
#define RINGTONELIBRARY_SOURCE_H

#include <string>

#include "result_set_utils.h"
#include "rdb_helper.h"

namespace OHOS {
namespace Media {
class RingtoneSource {
public:
    void Init(const std::string &restoreDbPath, const std::string &localDbPath);
    void InitRingtoneDb();
    std::shared_ptr<NativeRdb::RdbStore> restoreRdbPtr_;
    std::shared_ptr<NativeRdb::RdbStore> localRdbPtr_;
};

class RingtoneRestoreRdbOpenCb : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    static const std::string CREATE_RINGTONE_TABLE;
};

class RingtoneLocalRdbOpenCb : public NativeRdb::RdbOpenCallback {
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONELIBRARY_SOURCE_H