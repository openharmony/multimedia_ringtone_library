/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "ringtone_dualfw_restore_test.h"

#define private public
#define protected public
#include "ringtone_dualfw_restore.h"
#undef protected
#undef private
#include "ringtone_errno.h"
#include "ringtone_log.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const string TEST_BACKUP_PATH = "/data/test/backup";
unique_ptr<RingtoneDualfwRestore> restoreDualfwService = nullptr;

void RingtoneDualfwRestoreTest::SetUpTestCase(void)
{
    restoreDualfwService = std::make_unique<RingtoneDualfwRestore>();
    int32_t res = restoreDualfwService->Init(TEST_BACKUP_PATH);
    ASSERT_EQ(res, Media::E_OK);
}

void RingtoneDualfwRestoreTest::TearDownTestCase(void)
{
    restoreDualfwService = nullptr;
}

void RingtoneDualfwRestoreTest::SetUp() {}

void RingtoneDualfwRestoreTest::TearDown() {}

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0001 start");
    EXPECT_NE(restoreDualfwService->dualfwSetting_, nullptr);
    EXPECT_NE(restoreDualfwService->mediaDataShare_, nullptr);
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0001 end");
}
} // namespace Media
} // namespace OHOS