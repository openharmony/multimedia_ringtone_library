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

#include "ringtone_metadata_test.h"

#include <memory>

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
std::shared_ptr<RingtoneMetadata> g_ringtoneMetadata;
void RingtoneMetadataTest::SetUpTestCase()
{
    g_ringtoneMetadata = std::make_shared<RingtoneMetadata>();
    EXPECT_NE(g_ringtoneMetadata, nullptr);
}

void RingtoneMetadataTest::TearDownTestCase()
{
}

// SetUp:Execute before each test case
void RingtoneMetadataTest::SetUp() {}

void RingtoneMetadataTest::TearDown(void) {}

HWTEST_F(RingtoneMetadataTest, metadata_SetToneType_test_001, TestSize.Level0)
{
    const int toneType = 2;
    g_ringtoneMetadata->SetToneType(toneType);
    auto type = g_ringtoneMetadata->GetToneType();
    EXPECT_EQ(toneType, type);
}

HWTEST_F(RingtoneMetadataTest, metadata_SetSourceType_test_001, TestSize.Level0)
{
    const int sourceType = 1;
    g_ringtoneMetadata->SetSourceType(sourceType);
    auto type = g_ringtoneMetadata->GetSourceType();
    EXPECT_EQ(sourceType, type);
}

HWTEST_F(RingtoneMetadataTest, metadata_SetShotToneType_test_001, TestSize.Level0)
{
    const int shotToneType = 2;
    g_ringtoneMetadata->SetShotToneType(shotToneType);
    auto type = g_ringtoneMetadata->GetShotToneType();
    EXPECT_EQ(shotToneType, type);
}

HWTEST_F(RingtoneMetadataTest, metadata_SetShotToneSourceType_test_001, TestSize.Level0)
{
    const int setShotToneSourceType = 0;
    g_ringtoneMetadata->SetShotToneSourceType(setShotToneSourceType);
    auto type = g_ringtoneMetadata->GetShotToneSourceType();
    EXPECT_EQ(setShotToneSourceType, type);
}

HWTEST_F(RingtoneMetadataTest, metadata_SetNotificationToneType_test_001, TestSize.Level0)
{
    const int setNotificationToneType = 1;
    g_ringtoneMetadata->SetNotificationToneType(setNotificationToneType);
    auto type = g_ringtoneMetadata->GetNotificationToneType();
    EXPECT_EQ(setNotificationToneType, type);
}

HWTEST_F(RingtoneMetadataTest, metadata_SetNotificationToneSourceType_test_001, TestSize.Level0)
{
    const int setNotificationToneSourceType = 0;
    g_ringtoneMetadata->SetNotificationToneSourceType(setNotificationToneSourceType);
    auto type = g_ringtoneMetadata->GetNotificationToneSourceType();
    EXPECT_EQ(setNotificationToneSourceType, type);
}

HWTEST_F(RingtoneMetadataTest, metadata_SetRingToneType_test_001, TestSize.Level0)
{
    const int setRingToneType = 2;
    g_ringtoneMetadata->SetRingToneType(setRingToneType);
    auto type = g_ringtoneMetadata->GetRingToneType();
    EXPECT_EQ(setRingToneType, type);
}

HWTEST_F(RingtoneMetadataTest, metadata_SetRingToneSourceType_test_001, TestSize.Level0)
{
    const int setRingToneSourceType = 1;
    g_ringtoneMetadata->SetRingToneSourceType(setRingToneSourceType);
    auto type = g_ringtoneMetadata->GetRingToneSourceType();
    EXPECT_EQ(setRingToneSourceType, type);
}

HWTEST_F(RingtoneMetadataTest, metadata_SetAlarmToneType_test_001, TestSize.Level0)
{
    const int setAlarmToneType = 2;
    g_ringtoneMetadata->SetAlarmToneType(setAlarmToneType);
    auto type = g_ringtoneMetadata->GetAlarmToneType();
    EXPECT_EQ(setAlarmToneType, type);
}

HWTEST_F(RingtoneMetadataTest, metadata_SetAlarmToneSourceType_test_001, TestSize.Level0)
{
    const int setAlarmToneSourceType = 0;
    g_ringtoneMetadata->SetAlarmToneSourceType(setAlarmToneSourceType);
    auto type = g_ringtoneMetadata->GetAlarmToneSourceType();
    EXPECT_EQ(setAlarmToneSourceType, type);
    auto str = g_ringtoneMetadata->ToString();
    EXPECT_NE(str, "");
}
} // namespace Media
} // namespace OHOS