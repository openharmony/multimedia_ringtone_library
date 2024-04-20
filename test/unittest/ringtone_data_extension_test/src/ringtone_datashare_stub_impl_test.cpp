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

#include "ringtone_datashare_stub_impl_test.h"

#define private public
#include "ringtone_datashare_stub_impl.h"
#undef private
#include "runtime.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {

void RingtoneDataShareStubImplUnitTest::SetUpTestCase() {}
void RingtoneDataShareStubImplUnitTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneDataShareStubImplUnitTest::SetUp() {}

void RingtoneDataShareStubImplUnitTest::TearDown(void) {}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_GetOwner_test_001, TestSize.Level0)
{
    napi_env env = nullptr;
    shared_ptr<DataShare::RingtoneDataShareExtension> extension;
    DataShare::RingtoneDataShareStubImpl ringtoneDataShareStubImpl(extension, env);
    extension = ringtoneDataShareStubImpl.GetOwner();
    EXPECT_EQ(extension, nullptr);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_GetOwner_test_002, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AbilityRuntime::RingtoneDataShareExtension *mediaDataShare;
    mediaDataShare = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    EXPECT_EQ(mediaDataShare != nullptr, true);

    napi_env env = nullptr;
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(mediaDataShare);
    DataShare::RingtoneDataShareStubImpl ringtoneDataShareStubImpl(extension, env);
    extension = ringtoneDataShareStubImpl.GetOwner();
    EXPECT_EQ((extension != nullptr), true);
}
} // namespace Media
} // namespace OHOS
