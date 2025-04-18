/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "ringtone_scan_executor_test.h"

#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_mimetype_utils.h"
#include "ringtone_scan_executor.h"
#include "ringtone_unittest_utils.h"
#include "ringtone_type.h"
#include <thread>

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
void RingtoneScanExecutorTest::SetUpTestCase() {}

void RingtoneScanExecutorTest::TearDownTestCase() {}

void RingtoneScanExecutorTest::SetUp() {}

void RingtoneScanExecutorTest::TearDown(void) {}


HWTEST_F(RingtoneScanExecutorTest, ringtonelib_Commit_test_001, TestSize.Level0)
{
    shared_ptr<RingtoneScanExecutor> ringtonescanexecutor = make_shared<RingtoneScanExecutor>();
    EXPECT_NE(ringtonescanexecutor, nullptr);
    ringtonescanexecutor->activeThread_ = 10;
    std::shared_ptr<RingtoneScannerObj> scanner;
    ringtonescanexecutor->Start();
    int32_t ret = ringtonescanexecutor->Commit(scanner);
    ringtonescanexecutor->isScanFinished = false;
    std::thread stopThread([ringtonescanexecutor]() {
        ringtonescanexecutor->Stop();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ringtonescanexecutor->stopCond.notify_one();
    stopThread.join();
    EXPECT_EQ(ret, 0);
}
} // namespace Media
} // namespace OHOS
