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

#define MLOG_TAG "RingtoneRestoreTest"

#include "ringtone_restore_test.h"

#include "ringtone_source.h"
#include "ringtone_restore_factory.h"
#include "ringtone_log.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const string TEST_BACKUP_PATH = "/data/test/backup";
const string TEST_BACKUP_DB_PATH = TEST_BACKUP_PATH + "/data/storage/el2/database/rdb";

void Init(RingtoneSource &ringtoneSource) {
    RINGTONE_INFO_LOG("start init ringtone database");
    const string ringtoneDbPath = TEST_BACKUP_DB_PATH + "/ringtone_library.db";
    ringtoneSource.Init(ringtoneDbPath);
    auto restore = RingtoneRestoreFactory::CreateObj(RESTORE_SCENE_TYPE_SINGLE_CLONE);
    restore->Init(TEST_BACKUP_PATH);
    restore->StartRestore();
}

void RingtoneRestoreTest::SetUpTestCase(void) {
    RINGTONE_INFO_LOG("Start Init");
    RingtoneSource ringtoneSource;
    Init(ringtoneSource);
}

void RingtoneRestoreTest::TearDownTestCase(void) {
    RINGTONE_INFO_LOG("TearDownTestCase");
}

void RingtoneRestoreTest::SetUp() {}

void RingtoneRestoreTest::TearDown() {}

HWTEST_F(RingtoneRestoreTest, ringtone_restore_test_init, TestSize.Level0) {
    RINGTONE_INFO_LOG("ringtone_restore_test_init start");
    auto restore = RingtoneRestoreFactory::CreateObj(RESTORE_SCENE_TYPE_SINGLE_CLONE);
    EXPECT_NE(restore, nullptr);
    RINGTONE_INFO_LOG("ringtone_restore_test_init end");
}
} // namespace Media
} // namespace OHOS