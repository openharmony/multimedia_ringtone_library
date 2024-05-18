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

#include "ringtone_asset_test.h"

#define private public
#include "ringtone_asset.h"
#undef private
#include "ringtone_errno.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
shared_ptr<RingtoneAsset> g_ringtoneAsset;

void RingtoneAssetTest::SetUpTestCase()
{
    g_ringtoneAsset = std::make_shared<RingtoneAsset>();
    EXPECT_NE(g_ringtoneAsset, nullptr);
}

void RingtoneAssetTest::TearDownTestCase()
{
}

// SetUp:Execute before each test case
void RingtoneAssetTest::SetUp() {}

void RingtoneAssetTest::TearDown(void) {}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetOpenStatus_test_001, TestSize.Level0)
{
    const int toneType = 2;
    int ret = g_ringtoneAsset->GetOpenStatus(toneType);
    EXPECT_EQ(ret, E_INVALID_VALUES);
    g_ringtoneAsset->RemoveOpenStatus(toneType);
    int32_t openStatus = 1;
    g_ringtoneAsset->SetOpenStatus(toneType, openStatus);
    ret = g_ringtoneAsset->GetOpenStatus(toneType);
    EXPECT_EQ(ret, openStatus);
    g_ringtoneAsset->SetOpenStatus(toneType, openStatus);
    g_ringtoneAsset->RemoveOpenStatus(toneType);
    ret = g_ringtoneAsset->GetOpenStatus(toneType);
    EXPECT_EQ(ret, E_INVALID_VALUES);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetId_test_001, TestSize.Level0)
{
    auto assetId = g_ringtoneAsset->GetId();
    EXPECT_EQ(assetId, 0);
    const int32_t id = 1;
    g_ringtoneAsset->SetId(id);
    assetId = g_ringtoneAsset->GetId();
    EXPECT_EQ(assetId, id);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetSize_test_001, TestSize.Level0)
{
    auto assetSize = g_ringtoneAsset->GetSize();
    EXPECT_EQ(assetSize, 0);
    const int64_t size = 3267; // 3267 is ringtone column size
    g_ringtoneAsset->SetSize(size);
    assetSize = g_ringtoneAsset->GetSize();
    EXPECT_EQ(assetSize, size);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetPath_test_001, TestSize.Level0)
{
    auto assetPath = g_ringtoneAsset->GetPath();
    EXPECT_EQ(assetPath, "");
    const string path = "data/tmp";
    g_ringtoneAsset->SetPath(path);
    assetPath = g_ringtoneAsset->GetPath();
    EXPECT_EQ(assetPath, path);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetDisplayName_test_001, TestSize.Level0)
{
    auto assetdisplayName = g_ringtoneAsset->GetDisplayName();
    EXPECT_EQ(assetdisplayName, "");
    const string displayName = "ringtoneAsset_GetDisplayName_test_001.ogg";
    g_ringtoneAsset->SetDisplayName(displayName);
    assetdisplayName = g_ringtoneAsset->GetDisplayName();
    EXPECT_EQ(assetdisplayName, displayName);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetTitle_test_001, TestSize.Level0)
{
    const string title = "ringtoneAsset_GetTitle_test_001";
    g_ringtoneAsset->SetTitle(title);
    auto assetTitle = g_ringtoneAsset->GetTitle();
    EXPECT_EQ(assetTitle, title);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetMimeType_test_001, TestSize.Level0)
{
    const string mimeType = "ogg";
    g_ringtoneAsset->SetMimeType(mimeType);
    auto assetMimeType = g_ringtoneAsset->GetMimeType();
    EXPECT_EQ(assetMimeType, mimeType);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetSourceType_test_001, TestSize.Level0)
{
    int32_t sourceType = 1;
    g_ringtoneAsset->SetSourceType(sourceType);
    auto assetSourceType = g_ringtoneAsset->GetSourceType();
    EXPECT_EQ(assetSourceType, sourceType);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetDateAdded_test_001, TestSize.Level0)
{
    const int64_t dateAdded = 1559276457;
    g_ringtoneAsset->SetDateAdded(dateAdded);
    auto assetDateAdded = g_ringtoneAsset->GetDateAdded();
    EXPECT_EQ(assetDateAdded, dateAdded);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetDateModified_test_001, TestSize.Level0)
{
    const int64_t dateModified = 1559276475;
    g_ringtoneAsset->SetDateModified(dateModified);
    auto assetDateModified = g_ringtoneAsset->GetDateModified();
    EXPECT_EQ(assetDateModified, dateModified);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetDateTaken_test_001, TestSize.Level0)
{
    const int64_t dataTaken = 1559277457;
    g_ringtoneAsset->SetDateTaken(dataTaken);
    auto assetDataTaken = g_ringtoneAsset->GetDateTaken();
    EXPECT_EQ(assetDataTaken, dataTaken);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetDuration_test_001, TestSize.Level0)
{
    const int32_t duration = 117;
    g_ringtoneAsset->SetDuration(duration);
    auto assetDuration = g_ringtoneAsset->GetDuration();
    EXPECT_EQ(assetDuration, duration);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetShottoneType_test_001, TestSize.Level0)
{
    const int32_t type = 1;
    g_ringtoneAsset->SetShottoneType(type);
    auto assetType = g_ringtoneAsset->GetShottoneType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetShottoneSourceType_test_001, TestSize.Level0)
{
    const int32_t type = 0;
    g_ringtoneAsset->SetShottoneSourceType(type);
    auto assetType = g_ringtoneAsset->GetShottoneSourceType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetNotificationtoneType_test_001, TestSize.Level0)
{
    const int32_t type = 1;
    g_ringtoneAsset->SetNotificationtoneType(type);
    auto assetType = g_ringtoneAsset->GetNotificationtoneType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetNotificationtoneSourceType_test_001, TestSize.Level0)
{
    const int32_t type = 0;
    g_ringtoneAsset->SetNotificationtoneSourceType(type);
    auto assetType = g_ringtoneAsset->GetNotificationtoneSourceType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetRingtoneType_test_001, TestSize.Level0)
{
    const int32_t type = 1;
    g_ringtoneAsset->SetRingtoneType(type);
    auto assetType = g_ringtoneAsset->GetRingtoneType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetRingtoneSourceType_test_001, TestSize.Level0)
{
    const int32_t type = 1;
    g_ringtoneAsset->SetRingtoneSourceType(type);
    auto assetType = g_ringtoneAsset->GetRingtoneSourceType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetAlarmtoneType_test_001, TestSize.Level0)
{
    const int32_t type = 0;
    g_ringtoneAsset->SetAlarmtoneType(type);
    auto assetType = g_ringtoneAsset->GetAlarmtoneType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetAlarmtoneSourceType_test_001, TestSize.Level0)
{
    const int32_t type = 1;
    g_ringtoneAsset->SetAlarmtoneSourceType(type);
    auto assetType = g_ringtoneAsset->GetAlarmtoneSourceType();
    EXPECT_EQ(assetType, type);
}

HWTEST_F(RingtoneAssetTest, ringtoneAsset_GetMemberValue_test_001, TestSize.Level0)
{
    variant<int32_t, int64_t, string, double> memberValue;
    const string name = "ringtoneAsset_GetMemberValue_test_001";
    auto assetMemberValue = g_ringtoneAsset->GetMemberValue(name);
    EXPECT_EQ(assetMemberValue, memberValue);
}
} // namespace Media
} // namespace OHOS