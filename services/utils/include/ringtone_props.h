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

#ifndef RINGTONE_PROPS_H
#define RINGTONE_PROPS_H

#include "rdb_sql_utils.h"
#include "rdb_store.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
using namespace std;
using namespace OHOS;
class RingtoneProps {
public:
    EXPORT RingtoneProps(shared_ptr<NativeRdb::RdbStore> rdb);
    EXPORT ~RingtoneProps() = default;

    EXPORT int32_t Init();
    EXPORT string GetProp(const string &propName, const string &defaultVal);
    EXPORT bool SetProp(const string &propName, const string &propVal);
private:
    EXPORT int32_t GetPropFromResultSet(shared_ptr<NativeRdb::ResultSet> resultSet, string &propVal);
    shared_ptr<NativeRdb::RdbStore> store_;
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_PROPS_H
