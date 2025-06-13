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

#include "ringtone_metadata_extractor_test.h"

#include <string>

#include "ability_context_impl.h"
#include "ringtone_data_manager.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#define private public
#include "ringtone_metadata_extractor.h"
#undef private
#include "ringtone_scanner_db.h"
#include "ringtone_unittest_utils.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
void RingtoneMetadataExtractorTest::SetUpTestCase()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    auto ret = RingtoneDataManager::GetInstance()->Init(abilityContextImpl);
    EXPECT_EQ(ret, E_OK);
}

void RingtoneMetadataExtractorTest::TearDownTestCase()
{
    auto dataManager = RingtoneDataManager::GetInstance();
    EXPECT_NE(dataManager, nullptr);
    dataManager->ClearRingtoneDataMgr();
}

// SetUp:Execute before each test case
void RingtoneMetadataExtractorTest::SetUp() {}

void RingtoneMetadataExtractorTest::TearDown(void) {}

HWTEST_F(RingtoneMetadataExtractorTest, metadataExtractor_Extract_test_001, TestSize.Level0)
{
    unique_ptr<RingtoneMetadata> data = make_unique<RingtoneMetadata>();
    unique_ptr<RingtoneScannerDb> ringtoneScannerDb;
    ringtoneScannerDb->GetFileBasicInfo(STORAGE_FILES_DIR, data);
    data->SetMediaType(static_cast<RingtoneMediaType>(RINGTONE_MEDIA_TYPE_AUDIO));
    int32_t ret = RingtoneMetadataExtractor::Extract(data);
    EXPECT_EQ(ret, E_AVMETADATA);
}

HWTEST_F(RingtoneMetadataExtractorTest, metadataExtractor_Extract_test_002, TestSize.Level0)
{
    unique_ptr<RingtoneMetadata> data = make_unique<RingtoneMetadata>();
    unique_ptr<RingtoneScannerDb> ringtoneScannerDb;
    ringtoneScannerDb->GetFileBasicInfo(STORAGE_FILES_DIR, data);
    data->SetMediaType(static_cast<RingtoneMediaType>(RINGTONE_MEDIA_TYPE_INVALID));
    int32_t ret = RingtoneMetadataExtractor::Extract(data);
    EXPECT_EQ(ret, E_AVMETADATA);
}

HWTEST_F(RingtoneMetadataExtractorTest, metadataExtractor_FillExtractedMetadata_test_001, TestSize.Level0)
{
    unique_ptr<RingtoneMetadata> data = make_unique<RingtoneMetadata>();
    unique_ptr<RingtoneScannerDb> ringtoneScannerDb;
    const int modifiedTime = 11;
    ringtoneScannerDb->GetFileBasicInfo(STORAGE_FILES_DIR, data);
    data->SetMediaType(static_cast<RingtoneMediaType>(RINGTONE_MEDIA_TYPE_AUDIO));
    data->SetData(STORAGE_FILES_DIR);
    data->SetDateModified(static_cast<int64_t>(modifiedTime));
    unordered_map<int32_t, std::string> resultMap;
    resultMap = { { Media::AV_KEY_DURATION, "" }, { Media::AV_KEY_DATE_TIME_FORMAT, "" },
        { Media::AV_KEY_MIME_TYPE, "" }, { Media::AV_KEY_TITLE, "" } };
    RingtoneMetadataExtractor::FillExtractedMetadata(resultMap, data);
    EXPECT_EQ(data->GetTitle(), "");
}

HWTEST_F(RingtoneMetadataExtractorTest, metadataExtractor_FillExtractedMetadata_test_002, TestSize.Level0)
{
    unique_ptr<RingtoneMetadata> data = make_unique<RingtoneMetadata>();
    unique_ptr<RingtoneScannerDb> ringtoneScannerDb;
    const int modifiedTime = 13;
    ringtoneScannerDb->GetFileBasicInfo(STORAGE_FILES_DIR, data);
    data->SetMediaType(static_cast<RingtoneMediaType>(RINGTONE_MEDIA_TYPE_AUDIO));
    data->SetData(STORAGE_FILES_DIR);
    data->SetDateModified(static_cast<int64_t>(modifiedTime));
    unordered_map<int32_t, std::string> resultMap;
    resultMap = { { Media::AV_KEY_DURATION, "a" }, { Media::AV_KEY_DATE_TIME_FORMAT, "a" },
        { Media::AV_KEY_MIME_TYPE, "a" }, { Media::AV_KEY_TITLE, "a" } };
    RingtoneMetadataExtractor::FillExtractedMetadata(resultMap, data);
    EXPECT_EQ(data->GetTitle(), "a");
    const string timeKey = "2021-10-1 15:00:00";
    resultMap = { { Media::AV_KEY_DURATION, "a" }, { Media::AV_KEY_DATE_TIME_FORMAT,  timeKey},
        { Media::AV_KEY_MIME_TYPE, "a" }, { Media::AV_KEY_TITLE, "a" } };
    RingtoneMetadataExtractor::FillExtractedMetadata(resultMap, data);
    EXPECT_EQ(data->GetTitle(), "a");
}

HWTEST_F(RingtoneMetadataExtractorTest, metadataExtractor_ExtractAudioMetadata_test_001, TestSize.Level0)
{
    unique_ptr<RingtoneMetadata> data = make_unique<RingtoneMetadata>();
    unique_ptr<RingtoneScannerDb> ringtoneScannerDb;
    ringtoneScannerDb->GetFileBasicInfo(STORAGE_FILES_DIR, data);
    data->SetMediaType(static_cast<RingtoneMediaType>(RINGTONE_MEDIA_TYPE_AUDIO));
    int32_t ret = RingtoneMetadataExtractor::ExtractAudioMetadata(data);
    EXPECT_EQ(ret, E_AVMETADATA);
    data->SetData(STORAGE_FILES_DIR);
    ret = RingtoneMetadataExtractor::ExtractAudioMetadata(data);
    EXPECT_NE(ret, E_OK);
    data->SetData("ExtractAudioMetadata");
    ret = RingtoneMetadataExtractor::ExtractAudioMetadata(data);
    EXPECT_EQ((ret == E_SYSCALL || ret == E_AVMETADATA), true);
}

HWTEST_F(RingtoneMetadataExtractorTest, metadataExtractor_FillExtractedMetadata_test_003, TestSize.Level0)
{
    RINGTONE_INFO_LOG("metadataExtractor_FillExtractedMetadata_test_003 start.");
    unique_ptr<RingtoneMetadata> data = make_unique<RingtoneMetadata>();
    unordered_map<int32_t, std::string> resultMap;
    const string timeKey = "1600-01-01 00:00:00";
    resultMap = { { Media::AV_KEY_DURATION, "-123456" }, { Media::AV_KEY_DATE_TIME_FORMAT,  timeKey},
        { Media::AV_KEY_MIME_TYPE, "a" }, { Media::AV_KEY_TITLE, "a" } };
    RingtoneMetadataExtractor::FillExtractedMetadata(resultMap, data);
    EXPECT_EQ(data->GetTitle(), "a");
    RINGTONE_INFO_LOG("metadataExtractor_FillExtractedMetadata_test_003 end.");
}

} // namespace Media
} // namespace OHOS
