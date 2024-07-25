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

#include "ringtone_dualfw_restore_test.h"
#include "datashare_ext_ability.h"
#include "iservice_registry.h"
#include "medialibrary_db_const.h"
#include "file_asset.h"
#include "fetch_result.h"
#include "ringtone_restore_napi.h"
#include "ringtone_restore_factory.h"
#include "ringtone_restore_type.h"
#include "ringtone_rdb_callbacks.h"

#define private public
#define protected public
#include "ringtone_dualfw_restore.h"
#include "dualfw_conf_loader.h"
#undef protected
#undef private
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "datashare_helper.h"
using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const string TEST_BACKUP_PATH = "/data/test/backup";

unique_ptr<RingtoneDualfwRestore> restoreDualfwService = nullptr;

void RingtoneDualfwRestoreTest::SetUpTestCase(void)
{
    restoreDualfwService = std::make_unique<RingtoneDualfwRestore>();
    int32_t res = restoreDualfwService->Init(TEST_BACKUP_PATH);
    (void) res;
}

void RingtoneDualfwRestoreTest::TearDownTestCase(void)
{
    restoreDualfwService = nullptr;
}

void RingtoneDualfwRestoreTest::SetUp() {}

void RingtoneDualfwRestoreTest::TearDown() {}

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0001 start");
    EXPECT_NE(restoreDualfwService->dualfwSetting_, nullptr);
    EXPECT_NE(restoreDualfwService->mediaDataShare_, nullptr);
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0001 end");
}

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_0002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0002 start");
    
    DualfwConfLoader confLoader;
    if (confLoader.Init() != E_OK) {
        RINGTONE_ERR_LOG("Failed to initialize DualfwConfLoader.");
        return;
    }
    auto confVal = confLoader.GetConf("SETTINGSDATA_CreatedTime");
    std::cout << "conf value of SETTINGSDATA_CreatedTime = " << confVal << std::endl;
    DualFwConf conf;
    confLoader.Load(conf, RESTORE_SCENE_TYPE_DUAL_CLONE);
    confLoader.ShowConf(conf);
    confLoader.Load(conf, RESTORE_SCENE_TYPE_DUAL_UPGRADE);
    confLoader.ShowConf(conf);
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0002 end");
}

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_0003, TestSize.Level0)
{
    NativeRdb::RdbStoreConfig config(RINGTONE_LIBRARY_DB_NAME);
    config.SetPath("/data/app/el2/100/database/com.ohos.ringtonelibrary.ringtonelibrarydata/rdb/"
        + RINGTONE_LIBRARY_DB_NAME);
    config.SetBundleName("xx");
    config.SetReadConSize(10);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S3);

    int32_t err;
    RingtoneDataCallBack cb;
    auto rdbStore = NativeRdb::RdbHelper::GetRdbStore(config, RINGTONE_RDB_VERSION, cb, err);
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromMedia;
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromRingtone;
    auto restore = std::make_unique<RingtoneDualfwRestoreClone>();
    restore->Init("/");
    restore->QueryMediaLibForFileInfo({"sound.m4a", "common.mp3", "cc"}, resultFromMedia, TOOL_QUERY_AUDIO);
    restore->QueryRingToneDbForFileInfo(rdbStore, {"Creek.ogg", "Dawn.ogg", "Flourish.ogg"}, resultFromRingtone);
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0003 end");
}
} // namespace Media
} // namespace OHOS