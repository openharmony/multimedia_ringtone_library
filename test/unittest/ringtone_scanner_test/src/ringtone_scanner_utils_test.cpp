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
#define MLOG_TAG "FileExtUnitTest"

#include "ringtone_scanner_utils_test.h"

#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_mimetype_utils.h"
#include "ringtone_scanner_utils.h"
#include "ringtone_unittest_utils.h"
#include "ringtone_type.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
void RingtoneScannerUtilsTest::SetUpTestCase() {}

void RingtoneScannerUtilsTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneScannerUtilsTest::SetUp()
{
    RingtoneUnitTestUtils::CleanTestFiles();
    RingtoneUnitTestUtils::InitRootDirs();
}

void RingtoneScannerUtilsTest::TearDown(void)
{
    RingtoneUnitTestUtils::CleanTestFiles();
}


HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_IsExists_test_001, TestSize.Level0)
{
    bool ret = RingtoneScannerUtils::IsExists(RINGTONE_DEFAULT_STR);
    EXPECT_EQ(ret, false);
    const string path = "ringtonelib_IsExists_test_001";
    ret = RingtoneScannerUtils::IsExists(path);
    EXPECT_EQ(ret, false);
    const string pathSecond = "/data/local/tmp/";
    ret = RingtoneScannerUtils::IsExists(pathSecond);
    EXPECT_EQ(ret, true);
}


HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_GetFileNameFromUri_test_001, TestSize.Level0)
{
    string ret = RingtoneScannerUtils::GetFileNameFromUri(RINGTONE_DEFAULT_STR);
    EXPECT_EQ(ret, "");
    string path = "ringtonelib_GetFileNameFromUri_test_001/test";
    ret = RingtoneScannerUtils::GetFileNameFromUri(path);
    EXPECT_EQ(ret, "test");
    path = "ringtonelib_GetFileNameFromUri_test_001";
    ret = RingtoneScannerUtils::GetFileNameFromUri(path);
    EXPECT_EQ(ret, "");
}

HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_GetFileExtension_test_001, TestSize.Level0)
{
    string ret = RingtoneScannerUtils::GetFileExtension(RINGTONE_DEFAULT_STR);
    EXPECT_EQ(ret, "");
    string path = "ringtonelib_GetFileExtension_test_001.test";
    ret = RingtoneScannerUtils::GetFileExtension(path);
    EXPECT_EQ(ret, "test");
    path = "ringtonelib_GetFileExtension_test_001";
    ret = RingtoneScannerUtils::GetFileExtension(path);
    EXPECT_EQ(ret, "");
}

HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_IsFileHidden_test_001, TestSize.Level0)
{
    bool ret = RingtoneScannerUtils::IsFileHidden(RINGTONE_DEFAULT_STR);
    EXPECT_EQ(ret, false);
    const string path = "ringtonelib_IsFileHidden_test_001/.test";
    ret = RingtoneScannerUtils::IsFileHidden(path);
    EXPECT_EQ(ret, true);
}

HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_GetParentPath_test_001, TestSize.Level0)
{
    string ret = RingtoneScannerUtils::GetParentPath(RINGTONE_DEFAULT_STR);
    EXPECT_EQ(ret, "");
    const string path = "ringtonelib_GetParentPath_test_001/test";
    ret = RingtoneScannerUtils::GetParentPath(path);
    EXPECT_EQ(ret, "ringtonelib_GetParentPath_test_001");
}

HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_GetFileTitle_test_001, TestSize.Level0)
{
    const string path = "ringtonelib_GetFileTitle_test_001";
    string ret = RingtoneScannerUtils::GetFileTitle(path);
    EXPECT_EQ(ret, path);
}

HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_IsDirHidden_test_001, TestSize.Level0)
{
    const string path = "ringtonelib_IsDirHidden_test_001/.test";
    bool ret = RingtoneScannerUtils::IsDirHidden(path);
    EXPECT_EQ(ret, true);
    ret = RingtoneScannerUtils::IsDirHidden(RINGTONE_DEFAULT_STR);
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_IsDirHiddenRecursive_test_001, TestSize.Level0)
{
    const string path = "ringtonelib_IsDirHiddenRecursive_test_001/.test";
    bool ret = RingtoneScannerUtils::IsDirHiddenRecursive(path);
    EXPECT_EQ(ret, true);
    ret = RingtoneScannerUtils::IsDirHiddenRecursive(RINGTONE_DEFAULT_STR);
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_IsDirectory_test_001, TestSize.Level0)
{
    bool ret = RingtoneScannerUtils::IsDirectory(RINGTONE_DEFAULT_STR);
    EXPECT_EQ(ret, false);
    const string path = "ringtonelib_IsDirectory_test_001";
    ret = RingtoneScannerUtils::IsDirectory(path);
    EXPECT_EQ(ret, false);
    const string pathSecond = "/storage/media";
    ret = RingtoneScannerUtils::IsDirectory(pathSecond);
    EXPECT_EQ(ret, true);
}

HWTEST_F(RingtoneScannerUtilsTest, ringtonelib_IsRegularFile_test_001, TestSize.Level0)
{
    bool ret = RingtoneScannerUtils::IsRegularFile(RINGTONE_DEFAULT_STR);
    EXPECT_EQ(ret, false);
    string path = "ringtonelib_IsRegularFile_test_001";
    ret = RingtoneScannerUtils::IsRegularFile(path);
    EXPECT_EQ(ret, false);
    const string pathSecond = "/storage/cloud/files/Ringtone/Scanner_ringtone.ogg";
    EXPECT_EQ(RingtoneUnitTestUtils::CreateFileFS(pathSecond), true);
    ret = RingtoneScannerUtils::IsRegularFile(pathSecond);
    EXPECT_EQ(ret, true);
}
} // namespace Media
} // namespace OHOS
