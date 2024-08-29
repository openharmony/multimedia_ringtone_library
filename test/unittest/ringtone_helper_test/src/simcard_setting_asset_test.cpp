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

#include "simcard_setting_asset_test.h"
#include "ringtone_type.h"
#include "ringtone_db_const.h"
#include "simcard_setting_asset.h"
#include "ringtone_errno.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
shared_ptr<SimcardSettingAsset> g_simcardSettingAsset;

void SimcardSettingAssetTest::SetUpTestCase()
{
    g_simcardSettingAsset = std::make_shared<SimcardSettingAsset>();
    EXPECT_NE(g_simcardSettingAsset, nullptr);
}

void SimcardSettingAssetTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void SimcardSettingAssetTest::SetUp() {}

void SimcardSettingAssetTest::TearDown(void) {}

HWTEST_F(SimcardSettingAssetTest, simcardSettingAsset_GetMode_test_001, TestSize.Level0)
{
    auto mode = g_simcardSettingAsset->GetMode();
    EXPECT_EQ(mode, RING_TONE_TYPE_NOT);
}

HWTEST_F(SimcardSettingAssetTest, simcardSettingAsset_SetMode_test_001, TestSize.Level0)
{
    auto mode = g_simcardSettingAsset->GetMode();
    EXPECT_EQ(mode, RING_TONE_TYPE_NOT);
    const int32_t newMode = RING_TONE_TYPE_SIM_CARD_2;
    g_simcardSettingAsset->SetMode(newMode);
    mode = g_simcardSettingAsset->GetMode();
    EXPECT_EQ(mode, newMode);
}

HWTEST_F(SimcardSettingAssetTest, simcardSettingAsset_SetToneFile_test_001, TestSize.Level0)
{
    auto toneFile = g_simcardSettingAsset->GetToneFile();
    EXPECT_EQ(toneFile, RINGTONE_DEFAULT_STR);
    const std::string newToneFile = ROOT_TONE_PRELOAD_PATH_NOAH_PATH;
    g_simcardSettingAsset->SetToneFile(newToneFile);
    toneFile = g_simcardSettingAsset->GetToneFile();
    EXPECT_EQ(toneFile, newToneFile);
}

HWTEST_F(SimcardSettingAssetTest, simcardSettingAsset_SetVibrateFile_test_001, TestSize.Level0)
{
    auto vibrateFile = g_simcardSettingAsset->GetVibrateFile();
    EXPECT_EQ(vibrateFile, RINGTONE_DEFAULT_STR);
    const std::string newVibrateFile = ROOT_TONE_PRELOAD_PATH_NOAH_PATH;
    g_simcardSettingAsset->SetVibrateFile(newVibrateFile);
    vibrateFile = g_simcardSettingAsset->GetVibrateFile();
    EXPECT_EQ(vibrateFile, newVibrateFile);
}
} // namespace Media
} // namespace OHOS
