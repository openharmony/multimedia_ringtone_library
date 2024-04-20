/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ringtone_scanner_db_test.h"

#include <string>

#include "rdb_helper.h"
#include "ringtone_data_manager.h"
#include "ringtone_errno.h"
#define private public
#include "ringtone_scanner_db.h"
#undef private
#include "ringtone_unittest_utils.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
class ConfigTestOpenCall : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
    static const string CREATE_TABLE_TEST;
};

const string ConfigTestOpenCall::CREATE_TABLE_TEST = string("CREATE TABLE IF NOT EXISTS test ") +
    "(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, age INTEGER, salary REAL, blobType BLOB)";

int ConfigTestOpenCall::OnCreate(NativeRdb::RdbStore &store)
{
    return store.ExecuteSql(CREATE_TABLE_TEST);
}

int ConfigTestOpenCall::OnUpgrade(NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    return 0;
}

void RingtoneScannerDbTest::SetUpTestCase()
{
    RingtoneUnitTestUtils::Init();
}

void RingtoneScannerDbTest::TearDownTestCase()
{
    auto dataManager = RingtoneDataManager::GetInstance();
    EXPECT_NE(dataManager, nullptr);
    dataManager->ClearRingtoneDataMgr();
}

// SetUp:Execute before each test case
void RingtoneScannerDbTest::SetUp() {}

void RingtoneScannerDbTest::TearDown(void) {}

HWTEST_F(RingtoneScannerDbTest, scannerDb_InsertMetadata_test_001, TestSize.Level0)
{
    RingtoneScannerDb ringtoneScannerDb;
    RingtoneMetadata metadata;
    string tableName;
    int32_t ret = ringtoneScannerDb.InsertMetadata(metadata, tableName);
    EXPECT_EQ((ret > 0), true);
    struct stat statInfo;
    metadata.SetData(ROOT_MEDIA_DIR);
    metadata.SetDisplayName(RingtoneScannerUtils::GetFileNameFromUri(ROOT_MEDIA_DIR));
    metadata.SetTitle(RingtoneScannerUtils::GetFileTitle(metadata.GetDisplayName()));
    metadata.SetMediaType(static_cast<MediaType>(MEDIA_TYPE_AUDIO));
    metadata.SetSize(statInfo.st_size);
    metadata.SetDateModified(statInfo.st_mtime);
    ret = ringtoneScannerDb.InsertMetadata(metadata, tableName);
    EXPECT_EQ((ret > 0), true);
}

HWTEST_F(RingtoneScannerDbTest, scannerDb_UpdateMetadata_test_001, TestSize.Level0)
{
    RingtoneScannerDb ringtoneScannerDb;
    RingtoneMetadata metadata;
    string tableName;
    int32_t ret = ringtoneScannerDb.UpdateMetadata(metadata, tableName);
    EXPECT_EQ(ret, E_DB_FAIL);

    ret = ringtoneScannerDb.InsertMetadata(metadata, tableName);

    int32_t albumId = ret;
    struct stat statInfo;
    metadata.SetData(ROOT_MEDIA_DIR);
    metadata.SetDisplayName(RingtoneScannerUtils::GetFileNameFromUri(ROOT_MEDIA_DIR));
    metadata.SetTitle(RingtoneScannerUtils::GetFileTitle(metadata.GetDisplayName()));
    metadata.SetMediaType(static_cast<MediaType>(MEDIA_TYPE_AUDIO));
    metadata.SetSize(statInfo.st_size);
    metadata.SetDateModified(statInfo.st_mtime);
    metadata.SetToneId(albumId);
    ret = ringtoneScannerDb.UpdateMetadata(metadata, tableName);
    EXPECT_EQ((ret > 0), true);
}
} // namespace Media
} // namespace OHOS
