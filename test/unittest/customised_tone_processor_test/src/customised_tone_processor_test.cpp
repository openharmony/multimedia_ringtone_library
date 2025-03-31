/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "customised_tone_processor_test.h"
#include "ability_context_impl.h"
#include "datashare_ext_ability.h"
#include "iservice_registry.h"
#include "ringtone_restore_napi.h"
#include "ringtone_restore_factory.h"
#include "ringtone_restore_type.h"
#include "ringtone_rdb_callbacks.h"

#define private public
#define protected public
#include "customised_tone_processor.h"
#include "dualfwk_conf_loader.h"
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

unique_ptr<CustomisedToneProcessor> g_customisedToneProcess = nullptr;

void CustomisedToneProcessorTest::SetUpTestCase(void)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    EXPECT_NE(abilityContextImpl, nullptr);
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> rUniStore  = RingtoneRdbStore::GetInstance(abilityContextImpl);
    EXPECT_NE(rUniStore, nullptr);
    int32_t ret = rUniStore->Init();
    EXPECT_EQ(ret, E_OK);
    g_customisedToneProcess = make_unique<CustomisedToneProcessor>();
    EXPECT_NE(g_customisedToneProcess, nullptr);

    const string subPath = "/data/local/tmp/test";
    EXPECT_EQ(RingtoneFileUtils::CreateDirectory(subPath), true);
    EXPECT_EQ(RingtoneFileUtils::CreateFile(TEST_BACKUP_DATA), E_SUCCESS);
}

void CustomisedToneProcessorTest::TearDownTestCase(void)
{
    g_customisedToneProcess = nullptr;
    system("rm -rf /data/local/tmp/test");
}

void CustomisedToneProcessorTest::SetUp() {}

void CustomisedToneProcessorTest::TearDown() {}

HWTEST_F(CustomisedToneProcessorTest, customised_tone_processor_001, TestSize.Level0)
{
    DualFwkConf dualFwkConf;
    int32_t ret = g_customisedToneProcess->GetCustomisedAudioPath(dualFwkConf);
    EXPECT_EQ(ret, E_OK);

    std::string ringtonePath = "";
    std::string ret1 = g_customisedToneProcess->ConvertCustomisedAudioPath(ringtonePath);
    EXPECT_EQ(ret1, "");
    ringtonePath = "/Music";
    ret1 = g_customisedToneProcess->ConvertCustomisedAudioPath(ringtonePath);
    EXPECT_EQ(ret1, "");
    ringtonePath = "/storage/emulated/0";
    ret1 = g_customisedToneProcess->ConvertCustomisedAudioPath(ringtonePath);
    EXPECT_EQ(ret1, "");
}

HWTEST_F(CustomisedToneProcessorTest, customised_tone_processor_002, TestSize.Level0)
{
    std::map<std::string, std::shared_ptr<FileInfo>> infoMap;
    g_customisedToneProcess->BuildFileInfos();
    int32_t ret = g_customisedToneProcess->QueryFileMgrForFileInfo(infoMap);
    EXPECT_EQ(ret, E_SUCCESS);
    std::string dualFilePath = "";
    int32_t toneType = 2;
    int32_t ringtoneType = 2;
    int32_t shotToneType = 2;
    std::vector<FileInfo> fileInfos;
    ret = g_customisedToneProcess->BuildFileInfo(dualFilePath, toneType, ringtoneType, shotToneType, fileInfos);
    EXPECT_EQ(ret, E_FAIL);
    dualFilePath = "/Music";
    ret = g_customisedToneProcess->BuildFileInfo(dualFilePath, toneType, ringtoneType, shotToneType, fileInfos);
    EXPECT_EQ(ret, E_FAIL);
}
} // namespace Media
} // namespace OHOS