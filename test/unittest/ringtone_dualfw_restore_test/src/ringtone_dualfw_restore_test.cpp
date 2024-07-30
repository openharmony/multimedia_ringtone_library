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
#include "ability_context_impl.h"
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
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"
#include "datashare_helper.h"
using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const string TEST_BACKUP_PATH = "/data/test/backup";
const string TEST_BACKUP_DATA = "/data/local/tmp/test/Adara.ogg";

unique_ptr<RingtoneDualfwRestore> g_restoreDualfwService = nullptr;

void RingtoneDualfwRestoreTest::SetUpTestCase(void)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> rUniStore  = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = rUniStore->Init();
    EXPECT_EQ(ret, E_OK);
    g_restoreDualfwService = std::make_unique<RingtoneDualfwRestore>();
    int32_t res = g_restoreDualfwService->Init(TEST_BACKUP_PATH);
    // (void) res;
    ASSERT_EQ(res, E_FAIL);

    const string subPath = "/data/local/tmp/test";
    EXPECT_EQ(RingtoneFileUtils::CreateDirectory(subPath), true);
    EXPECT_EQ(RingtoneFileUtils::CreateFile(TEST_BACKUP_DATA), E_SUCCESS);
}

void RingtoneDualfwRestoreTest::TearDownTestCase(void)
{
    g_restoreDualfwService = nullptr;
    system("rm -rf /data/local/tmp/test");
}

void RingtoneDualfwRestoreTest::SetUp() {}

void RingtoneDualfwRestoreTest::TearDown() {}

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0001 start");
    EXPECT_NE(g_restoreDualfwService->dualfwSetting_, nullptr);
    EXPECT_NE(g_restoreDualfwService->mediaDataShare_, nullptr);
    g_restoreDualfwService->StartRestore();
    std::unique_ptr<DualFwConfRow> conf = std::make_unique<DualFwConfRow>();
    string confName = "alarm_alert_path";
    const string confValue = TEST_BACKUP_DATA;
    const string isSysSet = "true";
    conf->name = confName;
    conf->defaultSysSet = isSysSet;
    conf->value = confValue;
    auto ret = g_restoreDualfwService->dualfwSetting_->ProcessConfRow(conf);
    EXPECT_EQ(ret, E_SUCCESS);
    confName = "notification_sound";
    conf->name = confName;
    ret = g_restoreDualfwService->dualfwSetting_->ProcessConfRow(conf);
    EXPECT_EQ(ret, E_SUCCESS);
    confName = "notification_sound_set";
    conf->name = confName;
    ret = g_restoreDualfwService->dualfwSetting_->ProcessConfRow(conf);
    EXPECT_EQ(ret, E_SUCCESS);
    g_restoreDualfwService->localRdb_ = RingtoneRdbStore::GetInstance()->GetRaw();
    g_restoreDualfwService->StartRestore();
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

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_0004, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0004 start");
    string backupPath = "";
    int32_t res = g_restoreDualfwService->Init(backupPath);
    ASSERT_EQ(res, E_INVALID_ARGUMENTS);
    backupPath = "/data/test";
    res = g_restoreDualfwService->Init(backupPath);
    ASSERT_EQ(res, E_SUCCESS);

    FileInfo info;
    info.title = TITLE_DEFAULT;
    g_restoreDualfwService->UpdateRestoreFileInfo(info);
    info.data = TEST_BACKUP_DATA;
    bool result = g_restoreDualfwService->OnPrepare(info, TEST_BACKUP_DATA);
    EXPECT_EQ(result, true);
    string data = "restore_OnPrepare_test_0001";
    info.data = data;
    const string destPath = "/restore_Init_test_0001/data";
    result = g_restoreDualfwService->OnPrepare(info, TEST_BACKUP_DATA);
    EXPECT_EQ(result, false);
    data = "createfile_001./data";
    info.data = data;
    result = g_restoreDualfwService->OnPrepare(info, TEST_BACKUP_DATA);
    EXPECT_EQ(result, false);
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0004 end");
}

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_00045, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_00045 start");
    auto obj = std::make_unique<RingtoneDualfwRestoreClone>();
    EXPECT_NE(obj, nullptr);
    obj->LoadDualfwConf();
    string xml = "setting_system";
    auto ret = g_restoreDualfwService->ParseDualfwConf(xml);
    EXPECT_EQ(ret, E_FAIL);
    vector<FileInfo> infos;
    g_restoreDualfwService->OnFinished(infos);
    xml = "/data/test/backup/setting_system.xml";
    ret = g_restoreDualfwService->ParseDualfwConf(xml);
    EXPECT_EQ(ret, E_SUCCESS);
    std::unique_ptr<DualfwConfLoader> dualLoader = std::make_unique<DualfwConfLoader>();
    DualFwConf conf;
    dualLoader->Load(conf, RESTORE_SCENE_TYPE_DUAL_CLONE);
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_00045 end");
}

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_0006, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0006 start");
    auto dualSetting = std::make_unique<DualfwSoundSetting>();
    EXPECT_NE(dualSetting, nullptr);
    DualFwConf conf;
    string message = "Adara.ogg";
    conf.messagePath = message;
    conf.notificationSoundPath = message;
    conf.ringtonePath = message;
    conf.ringtone2Path = message;
    conf.alarmAlertPath = message;
    conf.messageSub1 = message;
    dualSetting->ProcessConf(conf);
    auto unConf = std::make_unique<DualFwConfRow>();
    auto ret = dualSetting->ProcessConfRow(unConf);
    EXPECT_EQ(ret, E_ERR);
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0006 end");
}
} // namespace Media
} // namespace OHOS