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

#include "ringtone_dfx_test.h"

#include <fcntl.h>
#include <iostream>
#include <string>

#include "ability_context_impl.h"
#include "dfx_const.h"
#include "dfx_manager.h"
#include "dfx_worker.h"
#include "preferences.h"
#include "preferences_helper.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
shared_ptr<OHOS::AbilityRuntime::AbilityContextImpl> abilityContextImpl;
void RingtoneDfxTest::SetUpTestCase()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
}
void RingtoneDfxTest::TearDownTestCase()
{
}

// SetUp:Execute before each test case
void RingtoneDfxTest::SetUp() {}

void RingtoneDfxTest::TearDown(void) {}

HWTEST_F(RingtoneDfxTest, ringtonelib_dfx_manager_test_0001, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtonelib_dfx_manager_test_0001 start");
    auto dfxMgr = DfxManager::GetInstance();
    EXPECT_NE(dfxMgr, nullptr);
    int32_t ret = dfxMgr->Init(nullptr);
    EXPECT_NE(ret, E_OK);
    int64_t res = dfxMgr->HandleReportXml();
    EXPECT_NE(res, 0);
    ret = dfxMgr->Init(abilityContextImpl);
    EXPECT_EQ(ret, E_OK);
    sleep(1);
    auto dfxMgrTwo = DfxManager::GetInstance();
    EXPECT_NE(dfxMgrTwo, nullptr);
    ret = dfxMgrTwo->Init(abilityContextImpl);
    EXPECT_EQ(ret, E_OK);
    RINGTONE_INFO_LOG("ringtonelib_dfx_manager_test_0001 end");
}
} // namespace Media
} // namespace OHOS
