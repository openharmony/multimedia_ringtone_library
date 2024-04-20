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

#include "ringtone_file_utils_test.h"

#include <algorithm>

#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"

using std::string;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const string PATH = "ringtone/audio";
const string DEFAULT_STR = "";

void RingtoneFileUtilsTest::SetUpTestCase() {}
void RingtoneFileUtilsTest::TearDownTestCase() {}
void RingtoneFileUtilsTest::SetUp()
{
    system("mkdir /data/local/tmp/test");
}
void RingtoneFileUtilsTest::TearDown(void)
{
    system("rm -rf /data/local/tmp/test");
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_SplitByChar_Test_001, TestSize.Level0)
{
    const char SLASH_CHAR = '/';
    auto str = RingtoneFileUtils::SplitByChar(PATH, SLASH_CHAR);
    ASSERT_EQ(str, "audio");
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_SplitByChar_Test_002, TestSize.Level0)
{
    const char star = '*';
    auto str = RingtoneFileUtils::SplitByChar(PATH, star);
    ASSERT_EQ(str, DEFAULT_STR);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_GetExtensionFromPath_Test_001, TestSize.Level0)
{
    const string path = "ringtone/audio.mp3";
    const string extension = "mp3";
    auto str = RingtoneFileUtils::GetExtensionFromPath(path);
    ASSERT_EQ(str, extension);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_GetExtensionFromPath_Test_002, TestSize.Level0)
{
    auto str = RingtoneFileUtils::GetExtensionFromPath(PATH);
    ASSERT_EQ(str, DEFAULT_STR);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_OpenFile_Test_001, TestSize.Level0)
{
    const string path = "/storage/media/100";
    auto ret = RingtoneFileUtils::OpenFile(path, RINGTONE_FILEMODE_WRITEONLY);
    ASSERT_EQ(ret, E_INVALID_PATH);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_MilliSecond_Test_001, TestSize.Level0)
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    int64_t MilliSecondTime = RingtoneFileUtils::Timespec2Millisecond(t);
    ASSERT_EQ(MilliSecondTime, RingtoneFileUtils::UTCTimeMilliSeconds());
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_StartsWith_Test_001, TestSize.Level0)
{
    const std::string prefix = "audio";
    auto str = RingtoneFileUtils::StartsWith(PATH, prefix);
    ASSERT_EQ(str, false);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_StartsWith_Test_002, TestSize.Level0)
{
    const std::string prefix = "ringtone/audio";
    auto str = RingtoneFileUtils::StartsWith(PATH, prefix);
    ASSERT_EQ(str, true);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_IsFileExists_Test_001, TestSize.Level0)
{
    const string filePath = "/data/test/isfileexists_001";
    EXPECT_EQ(RingtoneFileUtils::IsFileExists(filePath), false);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_CreateFile_Test_001, TestSize.Level0)
{
    const string filePath = "/data/local/tmp/test/createfile_001.ogg";
    EXPECT_EQ(RingtoneFileUtils::CreateFile(filePath), E_SUCCESS);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_CreateFile_Test_002, TestSize.Level0)
{
    const string filePath = DEFAULT_STR;
    EXPECT_EQ(RingtoneFileUtils::CreateFile(filePath), E_VIOLATION_PARAMETERS);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_CreateFile_Test_003, TestSize.Level0)
{
    const string filePath = "/data/local/tmp/test/createfile_003.ogg";
    EXPECT_EQ(RingtoneFileUtils::CreateFile(filePath), E_SUCCESS);
    EXPECT_EQ(RingtoneFileUtils::CreateFile(filePath), E_FILE_EXIST);
}

HWTEST_F(RingtoneFileUtilsTest, ringtoneFileUtils_CreateFile_Test_004, TestSize.Level0)
{
    const string filePath = "/data/local/tmp/test/test/test/test/createfile_004.ogg";
    EXPECT_EQ(RingtoneFileUtils::CreateFile(filePath), E_ERR);
}
} // namespace Media
} // namespace OHOS
