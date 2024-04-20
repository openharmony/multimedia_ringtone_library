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

#include "ringtone_scanner_unit_test.h"

#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_scanner_manager.h"
#include "ringtone_scanner_utils.h"
#include "ringtone_unittest_utils.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace {
    shared_ptr<RingtoneScannerManager> ringtoneScannerManager = nullptr;
} // namespace


void RingtoneScannerUnitTest::SetUpTestCase()
{
    RingtoneUnitTestUtils::Init();

    ringtoneScannerManager = RingtoneScannerManager::GetInstance();
}

void RingtoneScannerUnitTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneScannerUnitTest::SetUp()
{
    RingtoneUnitTestUtils::CleanTestFiles();
    RingtoneUnitTestUtils::InitRootDirs();
}

void RingtoneScannerUnitTest::TearDown(void)
{
    RingtoneUnitTestUtils::CleanTestFiles();
}

HWTEST_F(RingtoneScannerUnitTest,  scannerManager_ScanDir_test_001, TestSize.Level0)
{
    if (ringtoneScannerManager == nullptr) {
        RINGTONE_ERR_LOG("RingtoneScannerManager invalid");
        exit(1);
    }

    auto scannerCallback = make_shared<TestRingtoneScannerCallback>();
    int result = ringtoneScannerManager->ScanDir(ROOT_MEDIA_DIR, scannerCallback);
    EXPECT_EQ(result, E_OK);

    if (result == 0) {
        RingtoneUnitTestUtils::WaitForCallback(scannerCallback);
        EXPECT_EQ(scannerCallback->status_, E_OK);
    }
}

HWTEST_F(RingtoneScannerUnitTest,  scannerManager_ScanOgg_test_001, TestSize.Level0)
{
    if (ringtoneScannerManager == nullptr) {
        RINGTONE_ERR_LOG("ringtoneScannerManager invalid");
        exit(1);
    }

    const string path = ROOT_MEDIA_DIR + "Ringtone/Scanner_ringtone.ogg";
    EXPECT_EQ(RingtoneUnitTestUtils::CreateFileFS(path), true);

    auto scannerCallback = make_shared<TestRingtoneScannerCallback>();
    int result = ringtoneScannerManager->ScanFile(path, scannerCallback);
    EXPECT_EQ(result, E_OK);

    if (result == 0) {
        RingtoneUnitTestUtils::WaitForCallback(scannerCallback);
        EXPECT_EQ(scannerCallback->status_, E_OK);
    }
}

HWTEST_F(RingtoneScannerUnitTest, scannerManager_ScanHiddenFile_Test_001, TestSize.Level0)
{
    if (ringtoneScannerManager == nullptr) {
        RINGTONE_ERR_LOG("RingtoneScannerManager invalid");
        exit(1);
    }

    const string path = ROOT_MEDIA_DIR + "Ringtone/.HiddenFile";
    EXPECT_EQ(RingtoneUnitTestUtils::CreateFileFS(path), true);

    auto scannerCallback = make_shared<TestRingtoneScannerCallback>();
    int result = ringtoneScannerManager->ScanFile(path, scannerCallback);
    EXPECT_EQ(result, E_OK);

    if (result == 0) {
        RingtoneUnitTestUtils::WaitForCallback(scannerCallback);
        EXPECT_EQ(scannerCallback->status_, E_FILE_HIDDEN);
    }
}

HWTEST_F(RingtoneScannerUnitTest,  scannerManager_ScanDir_CanonicalPathtest_001, TestSize.Level0)
{
    if (ringtoneScannerManager == nullptr) {
        RINGTONE_ERR_LOG("RingtoneScannerManager invalid");
        exit(1);
    }

    const string path = ROOT_MEDIA_DIR + "../files";

    auto scannerCallback = make_shared<TestRingtoneScannerCallback>();
    int result = ringtoneScannerManager->ScanDir(path, scannerCallback);
    EXPECT_EQ(result, E_OK);

    if (result == 0) {
        RingtoneUnitTestUtils::WaitForCallback(scannerCallback);
        EXPECT_EQ(scannerCallback->status_, E_OK);
    }
}

HWTEST_F(RingtoneScannerUnitTest,  scannerManager_ScanFile_CanonicalPathtest_001, TestSize.Level0)
{
    if (ringtoneScannerManager == nullptr) {
        RINGTONE_ERR_LOG("RingtoneScannerManager invalid");
        exit(1);
    }

    const string path = ROOT_MEDIA_DIR + "../files/Ringtone/Canonical1.ogg";
    EXPECT_EQ(RingtoneUnitTestUtils::CreateFileFS(path), true);

    auto scannerCallback = make_shared<TestRingtoneScannerCallback>();
    int result = ringtoneScannerManager->ScanFile(path, scannerCallback);
    EXPECT_EQ(result, E_OK);

    if (result == 0) {
        RingtoneUnitTestUtils::WaitForCallback(scannerCallback);
        EXPECT_EQ(scannerCallback->status_, E_OK);
    }
}
} // namespace Media
} // namespace OHOS
