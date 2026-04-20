/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "haptic_2_tone_asset_test.h"
#include "haptic_2_tone_asset.h"
#include "ringtone_errno.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
shared_ptr<Haptic2ToneAsset> g_haptic2ToneAsset;

void Haptic2ToneAssetTest::SetUpTestCase()
{
    g_haptic2ToneAsset = std::make_shared<Haptic2ToneAsset>();
    EXPECT_NE(g_haptic2ToneAsset, nullptr);
}

void Haptic2ToneAssetTest::TearDownTestCase() {}

void Haptic2ToneAssetTest::SetUp() {}

void Haptic2ToneAssetTest::TearDown(void) {}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetId_test_001, TestSize.Level0)
{
    auto assetId = g_haptic2ToneAsset->GetId();
    EXPECT_EQ(assetId, 0);
    const int32_t id = 1;
    g_haptic2ToneAsset->SetId(id);
    assetId = g_haptic2ToneAsset->GetId();
    EXPECT_EQ(assetId, id);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetSize_test_001, TestSize.Level0)
{
    auto assetSize = g_haptic2ToneAsset->GetSize();
    EXPECT_EQ(assetSize, 0);
    const int64_t size = 3267;
    g_haptic2ToneAsset->SetSize(size);
    assetSize = g_haptic2ToneAsset->GetSize();
    EXPECT_EQ(assetSize, size);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetPath_test_001, TestSize.Level0)
{
    auto assetPath = g_haptic2ToneAsset->GetPath();
    EXPECT_EQ(assetPath, "");
    const string path = "data/tmp/test.wav";
    g_haptic2ToneAsset->SetPath(path);
    assetPath = g_haptic2ToneAsset->GetPath();
    EXPECT_EQ(assetPath, path);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetDisplayName_test_001, TestSize.Level0)
{
    auto assetdisplayName = g_haptic2ToneAsset->GetDisplayName();
    EXPECT_EQ(assetdisplayName, "");
    const string displayName = "test_audio.wav";
    g_haptic2ToneAsset->SetDisplayName(displayName);
    assetdisplayName = g_haptic2ToneAsset->GetDisplayName();
    EXPECT_EQ(assetdisplayName, displayName);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetTitle_test_001, TestSize.Level0)
{
    const string title = "test_audio";
    g_haptic2ToneAsset->SetTitle(title);
    auto assetTitle = g_haptic2ToneAsset->GetTitle();
    EXPECT_EQ(assetTitle, title);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetHaptic2ToneType_test_001, TestSize.Level0)
{
    const int32_t type = 1;
    g_haptic2ToneAsset->SetHaptic2ToneType(type);
    auto assetType = g_haptic2ToneAsset->GetHaptic2ToneType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetSourceType_test_001, TestSize.Level0)
{
    int32_t sourceType = 1;
    g_haptic2ToneAsset->SetSourceType(sourceType);
    auto assetSourceType = g_haptic2ToneAsset->GetSourceType();
    EXPECT_EQ(assetSourceType, sourceType);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetDateAdded_test_001, TestSize.Level0)
{
    const int64_t dateAdded = 1559276457;
    g_haptic2ToneAsset->SetDateAdded(dateAdded);
    auto assetDateAdded = g_haptic2ToneAsset->GetDateAdded();
    EXPECT_EQ(assetDateAdded, dateAdded);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetDateModified_test_001, TestSize.Level0)
{
    const int64_t dateModified = 1559276475;
    g_haptic2ToneAsset->SetDateModified(dateModified);
    auto assetDateModified = g_haptic2ToneAsset->GetDateModified();
    EXPECT_EQ(assetDateModified, dateModified);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetPlayMode_test_001, TestSize.Level0)
{
    const int32_t playMode = 1;
    g_haptic2ToneAsset->SetPlayMode(playMode);
    auto assetPlayMode = g_haptic2ToneAsset->GetPlayMode();
    EXPECT_EQ(assetPlayMode, playMode);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetScannerFlag_test_001, TestSize.Level0)
{
    const int32_t scannerFlag = 1;
    g_haptic2ToneAsset->SetScannerFlag(scannerFlag);
    auto assetScannerFlag = g_haptic2ToneAsset->GetScannerFlag();
    EXPECT_EQ(assetScannerFlag, scannerFlag);
}

HWTEST_F(Haptic2ToneAssetTest, haptic2ToneAsset_GetMemberValue_test_001, TestSize.Level0)
{
    variant<int32_t, int64_t, string, double> memberValue;
    const string name = "haptic2ToneAsset_GetMemberValue_test_001";
    auto assetMemberValue = g_haptic2ToneAsset->GetMemberValue(name);
    EXPECT_EQ(assetMemberValue, memberValue);
}
} // namespace Media
} // namespace OHOS
