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

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {

void RingtoneDfxTest::SetUpTestCase()
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    auto dfxMgr = DfxManager::GetInstance();
    EXPECT_NE(dfxMgr, nullptr);
    int32_t ret = dfxMgr->Init(abilityContextImpl);
    EXPECT_EQ(ret, E_OK);
}
void RingtoneDfxTest::TearDownTestCase()
{
}

// SetUp:Execute before each test case
void RingtoneDfxTest::SetUp() {}

void RingtoneDfxTest::TearDown(void) {}

HWTEST_F(RingtoneDfxTest, ringtonelib_dfx_manager_test_001, TestSize.Level0)
{
    int64_t ret = DfxManager::GetInstance()->HandleReportXml();
    EXPECT_EQ((ret > 0), true);
    std::string bundleName = "test";
    ret = DfxManager::GetInstance()->RequestTonesCount(SourceType::SOURCE_TYPE_INVALID);
    EXPECT_EQ(ret, 0);
}
} // namespace Media
} // namespace OHOS