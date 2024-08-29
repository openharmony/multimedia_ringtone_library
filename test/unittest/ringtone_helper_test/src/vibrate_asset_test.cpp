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

#include "vibrate_asset_test.h"
#include "vibrate_asset.h"
#include "ringtone_errno.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
shared_ptr<VibrateAsset> g_vibrateAsset;

void VibrateAssetTest::SetUpTestCase()
{
    g_vibrateAsset = std::make_shared<VibrateAsset>();
    EXPECT_NE(g_vibrateAsset, nullptr);
}

void VibrateAssetTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void VibrateAssetTest::SetUp() {}

void VibrateAssetTest::TearDown(void) {}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetId_test_001, TestSize.Level0)
{
    auto assetId = g_vibrateAsset->GetId();
    EXPECT_EQ(assetId, 0);
    const int32_t id = 1;
    g_vibrateAsset->SetId(id);
    assetId = g_vibrateAsset->GetId();
    EXPECT_EQ(assetId, id);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetSize_test_001, TestSize.Level0)
{
    auto assetSize = g_vibrateAsset->GetSize();
    EXPECT_EQ(assetSize, 0);
    const int64_t size = 3267; // 3267 is vibrate column size
    g_vibrateAsset->SetSize(size);
    assetSize = g_vibrateAsset->GetSize();
    EXPECT_EQ(assetSize, size);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetPath_test_001, TestSize.Level0)
{
    auto assetPath = g_vibrateAsset->GetPath();
    EXPECT_EQ(assetPath, "");
    const string path = "data/tmp";
    g_vibrateAsset->SetPath(path);
    assetPath = g_vibrateAsset->GetPath();
    EXPECT_EQ(assetPath, path);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetDisplayName_test_001, TestSize.Level0)
{
    auto assetdisplayName = g_vibrateAsset->GetDisplayName();
    EXPECT_EQ(assetdisplayName, "");
    const string displayName = "vibrateAsset_GetDisplayName_test_001.ogg";
    g_vibrateAsset->SetDisplayName(displayName);
    assetdisplayName = g_vibrateAsset->GetDisplayName();
    EXPECT_EQ(assetdisplayName, displayName);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetTitle_test_001, TestSize.Level0)
{
    const string title = "vibrateAsset_GetTitle_test_001";
    g_vibrateAsset->SetTitle(title);
    auto assetTitle = g_vibrateAsset->GetTitle();
    EXPECT_EQ(assetTitle, title);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetDisplayLanguage_test_001, TestSize.Level0)
{
    const string displayLanguage = "ch-zn";
    g_vibrateAsset->SetDisplayLanguage(displayLanguage);
    auto assetLanguage = g_vibrateAsset->GetDisplayLanguage();
    EXPECT_EQ(assetLanguage, displayLanguage);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetVibrateType_test_001, TestSize.Level0)
{
    const int32_t type = 1;
    g_vibrateAsset->SetVibrateType(type);
    auto assetType = g_vibrateAsset->GetVibrateType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetSourceType_test_001, TestSize.Level0)
{
    int32_t sourceType = 1;
    g_vibrateAsset->SetSourceType(sourceType);
    auto assetSourceType = g_vibrateAsset->GetSourceType();
    EXPECT_EQ(assetSourceType, sourceType);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetDateAdded_test_001, TestSize.Level0)
{
    const int64_t dateAdded = 1559276457;
    g_vibrateAsset->SetDateAdded(dateAdded);
    auto assetDateAdded = g_vibrateAsset->GetDateAdded();
    EXPECT_EQ(assetDateAdded, dateAdded);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetDateModified_test_001, TestSize.Level0)
{
    const int64_t dateModified = 1559276475;
    g_vibrateAsset->SetDateModified(dateModified);
    auto assetDateModified = g_vibrateAsset->GetDateModified();
    EXPECT_EQ(assetDateModified, dateModified);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetDateTaken_test_001, TestSize.Level0)
{
    const int64_t dataTaken = 1559277457;
    g_vibrateAsset->SetDateTaken(dataTaken);
    auto assetDataTaken = g_vibrateAsset->GetDateTaken();
    EXPECT_EQ(assetDataTaken, dataTaken);
}

HWTEST_F(VibrateAssetTest, vibrateAsset_GetMemberValue_test_001, TestSize.Level0)
{
    variant<int32_t, int64_t, string, double> memberValue;
    const string name = "vibrateAsset_GetMemberValue_test_001";
    auto assetMemberValue = g_vibrateAsset->GetMemberValue(name);
    EXPECT_EQ(assetMemberValue, memberValue);
}
} // namespace Media
} // namespace OHOS
