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

#include "ringtone_datashare_extension_test.h"

#include "ability_handler.h"
#include "ability_info.h"
#include "datashare_ext_ability.h"
#include "get_self_permissions.h"
#include "ohos_application.h"
#define private public
#include "ringtone_datashare_stub_impl.h"
#undef private
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_type.h"
#include "runtime.h"


using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {

void RingtoneDataShareExtensionTest::SetUpTestCase()
{
    vector<string> perms;
    perms.push_back("ohos.permission.WRITE_RINGTONE");

    uint64_t tokenId = 0;
    RingtonePermissionUtilsUnitTest::SetAccessTokenPermission("RingtoneDataManagerUnitTest", perms, tokenId);
    ASSERT_TRUE(tokenId != 0);
}
void RingtoneDataShareExtensionTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneDataShareExtensionTest::SetUp() {}

void RingtoneDataShareExtensionTest::TearDown() {}

HWTEST_F(RingtoneDataShareExtensionTest, dataShareExtension_OnConnect_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AbilityRuntime::RingtoneDataShareExtension *mediaDataShare;
    AbilityRuntime::DataShareExtAbility::Create(runtime);
    mediaDataShare = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    ASSERT_TRUE(mediaDataShare != nullptr);

    std::shared_ptr<AppExecFwk::AbilityLocalRecord> record = nullptr;
    const std::shared_ptr<AppExecFwk::OHOSApplication> application = nullptr;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = nullptr;
    const sptr<IRemoteObject> token = nullptr;
    mediaDataShare->Init(record, application, handler, token);

    std::shared_ptr<AbilityRuntime::AbilityInfo> info = std::make_shared<AbilityRuntime::AbilityInfo>();
    info->name = "start";
    napi_env env = nullptr;
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(mediaDataShare);
    sptr<DataShare::RingtoneDataShareStubImpl> remoteObject =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> recordTwo =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token);
    mediaDataShare->Init(recordTwo, application, handler, token);
    std::shared_ptr<AppExecFwk::OHOSApplication> applicationTwo = std::make_shared<AppExecFwk::OHOSApplication>();
    mediaDataShare->Init(recordTwo, applicationTwo, handler, token);
    std::shared_ptr<AbilityRuntime::EventRunner> runner = nullptr;
    std::shared_ptr<AppExecFwk::AbilityHandler> handlerTwo = make_shared<AppExecFwk::AbilityHandler>(runner);
    mediaDataShare->Init(recordTwo, applicationTwo, handlerTwo, token);
    const sptr<IRemoteObject> tokenTwo = remoteObject->AsObject();
    mediaDataShare->Init(recordTwo, applicationTwo, handlerTwo, tokenTwo);
    AAFwk::Want want;
    auto ret = mediaDataShare->OnConnect(want);
    EXPECT_EQ((ret != nullptr), true);
    mediaDataShare->OnStart(want);
    mediaDataShare->OnStop();
    ret = mediaDataShare->OnConnect(want);
    EXPECT_EQ((ret != nullptr), true);
}

HWTEST_F(RingtoneDataShareExtensionTest, dataShareExtension_Insert_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AbilityRuntime::RingtoneDataShareExtension *mediaDataShare;
    mediaDataShare = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    EXPECT_EQ(mediaDataShare != nullptr, true);

    Uri uri(RINGTONE_PATH_URI);
    AbilityRuntime::DataShareValuesBucket values;
    auto ret = mediaDataShare->Insert(uri, values);
    EXPECT_EQ(ret, E_COMMON_START);

    Uri uriError(RINGTONE_URI);
    ret = mediaDataShare->Insert(uriError, values);
    EXPECT_EQ(ret, E_INVALID_URI);
}

HWTEST_F(RingtoneDataShareExtensionTest, dataShareExtension_Update_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AbilityRuntime::RingtoneDataShareExtension *mediaDataShare;
    mediaDataShare = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    EXPECT_EQ(mediaDataShare != nullptr, true);

    Uri uri(RINGTONE_PATH_URI);
    const AbilityRuntime::DataSharePredicates predicates;
    AbilityRuntime::DataShareValuesBucket values;
    auto ret = mediaDataShare->Update(uri, predicates, values);
    EXPECT_EQ(ret, E_COMMON_START);

    Uri uriError(RINGTONE_URI);
    ret = mediaDataShare->Update(uriError, predicates, values);
    EXPECT_EQ(ret, E_INVALID_URI);
}

HWTEST_F(RingtoneDataShareExtensionTest, dataShareExtension_Delete_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AbilityRuntime::RingtoneDataShareExtension *mediaDataShare;
    mediaDataShare = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    EXPECT_EQ(mediaDataShare != nullptr, true);

    Uri uri(RINGTONE_PATH_URI);
    const AbilityRuntime::DataSharePredicates predicates;
    auto ret = mediaDataShare->Delete(uri, predicates);
    EXPECT_EQ(ret, E_COMMON_START);

    Uri uriError(RINGTONE_URI);
    ret = mediaDataShare->Delete(uriError, predicates);
    EXPECT_EQ(ret, E_INVALID_URI);
}

HWTEST_F(RingtoneDataShareExtensionTest, dataShareExtension_Query_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AbilityRuntime::RingtoneDataShareExtension *mediaDataShare;
    mediaDataShare = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    EXPECT_EQ(mediaDataShare != nullptr, true);

    Uri uri(RINGTONE_PATH_URI);
    vector<string> columns;
    const AbilityRuntime::DataSharePredicates predicates;
    AbilityRuntime::DatashareBusinessError businessError;
    auto ret = mediaDataShare->Query(uri, predicates, columns, businessError);
    EXPECT_EQ(ret, nullptr);

    Uri uriError(RINGTONE_URI);
    ret = mediaDataShare->Query(uriError, predicates, columns, businessError);
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(RingtoneDataShareExtensionTest, dataShareExtension_Query_Vibrate_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AbilityRuntime::RingtoneDataShareExtension *mediaDataShare;
    mediaDataShare = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    EXPECT_EQ(mediaDataShare != nullptr, true);

    Uri uri(VIBRATE_PATH_URI);
    vector<string> columns;
    const AbilityRuntime::DataSharePredicates predicates;
    AbilityRuntime::DatashareBusinessError businessError;
    auto ret = mediaDataShare->Query(uri, predicates, columns, businessError);
    EXPECT_EQ(ret, nullptr);

    Uri uriError(VIBRATE_URI_PATH);
    ret = mediaDataShare->Query(uriError, predicates, columns, businessError);
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(RingtoneDataShareExtensionTest, dataShareExtension_OpenFile_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AbilityRuntime::RingtoneDataShareExtension *mediaDataShare;
    mediaDataShare = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    EXPECT_EQ(mediaDataShare != nullptr, true);

    Uri uri(RINGTONE_PATH_URI);
    auto ret = mediaDataShare->OpenFile(uri, RINGTONE_FILEMODE_READONLY);
    EXPECT_EQ(ret, E_INVALID_VALUES);

    Uri uriError(RINGTONE_URI);
    ret = mediaDataShare->OpenFile(uriError, RINGTONE_FILEMODE_READONLY);
    EXPECT_EQ(ret, E_INVALID_URI);
}

HWTEST_F(RingtoneDataShareExtensionTest, dataShareExtension_DumpDataShareValueBucket_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;
    AbilityRuntime::RingtoneDataShareExtension *mediaDataShare;
    mediaDataShare = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    EXPECT_EQ(mediaDataShare != nullptr, true);

    const string ringtoneUintType = "uint_type";
    const string ringtoneBoolType = "bool_type";
    const string ringtoneDoubleType = "double_type";
    const string ringtoneVectorType = "vector_type";
    const int64_t addedTime = 1559276453;
    const double doubleSize = 155.9276457;
    std::vector<string> tabFields;
    std::vector<uint8_t> ringtoneVec;
    ringtoneVec.push_back(1);
    tabFields.push_back(ringtoneUintType);
    tabFields.push_back(RINGTONE_COLUMN_DATE_ADDED);
    tabFields.push_back(RINGTONE_COLUMN_MIME_TYPE);
    tabFields.push_back(ringtoneBoolType);
    tabFields.push_back(ringtoneDoubleType);
    tabFields.push_back(ringtoneVectorType);
    AbilityRuntime::DataShareValuesBucket value;
    value.Put(ringtoneUintType, static_cast<uint8_t>(1));
    value.Put(RINGTONE_COLUMN_DATE_ADDED, static_cast<int64_t>(addedTime));
    value.Put(RINGTONE_COLUMN_MIME_TYPE, static_cast<string>(RINGTONE_CONTAINER_TYPE_OGG));
    value.Put(ringtoneBoolType, static_cast<bool>(true));
    value.Put(ringtoneDoubleType, static_cast<double>(doubleSize));
    value.Put(ringtoneVectorType, static_cast<std::vector<uint8_t>>(ringtoneVec));
    mediaDataShare->DumpDataShareValueBucket(tabFields, value);
    EXPECT_EQ(mediaDataShare != nullptr, true);
}
} // namespace Media
} // namespace OHOS
