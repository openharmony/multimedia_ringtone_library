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

#include "ability_handler.h"
#include "ability_info.h"
#include "data_ability_observer_interface.h"
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

void RingtoneDataShareStubImplUnitTest::SetUpTestCase() {}
void RingtoneDataShareStubImplUnitTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtoneDataShareStubImplUnitTest::SetUp() {}

void RingtoneDataShareStubImplUnitTest::TearDown() {}

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

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_GetFileTypes_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    std::string mimeTypeFilter;
    auto ret = ringtoneDataShareStubImpl->GetFileTypes(uri, mimeTypeFilter);
    std::vector<std::string> FileTypesVec;
    EXPECT_EQ(ret, FileTypesVec);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->GetFileTypes(uri, mimeTypeFilter);
    EXPECT_EQ(ret, FileTypesVec);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_OpenFile_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    std::string mode;
    int ret = ringtoneDataShareStubImpl->OpenFile(uri, mode);
    EXPECT_EQ((ret != E_SUCCESS), true);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->OpenFile(uri, mode);
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_OpenRawFile_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    std::string mode;
    int ret = ringtoneDataShareStubImpl->OpenRawFile(uri, mode);
    EXPECT_EQ(ret, E_ERR);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->OpenRawFile(uri, mode);
    EXPECT_EQ(ret, E_ERR);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_Insert_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    AbilityRuntime::DataShareValuesBucket value;
    int ret = ringtoneDataShareStubImpl->Insert(uri, value);
    EXPECT_EQ((ret != E_OK), true);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->Insert(uri, value);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_InsertExt_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    AbilityRuntime::DataShareValuesBucket value;
    std::string result;
    int ret = ringtoneDataShareStubImpl->InsertExt(uri, value, result);
    EXPECT_EQ(ret, E_ERR);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->InsertExt(uri, value, result);
    EXPECT_EQ(ret, E_OK);;
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_Update_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    AbilityRuntime::DataSharePredicates predicates;
    AbilityRuntime::DataShareValuesBucket value;
    int ret = ringtoneDataShareStubImpl->Update(uri, predicates, value);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->Update(uri, predicates, value);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_Delete_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    AbilityRuntime::DataSharePredicates predicates;
    int ret = ringtoneDataShareStubImpl->Delete(uri, predicates);
    EXPECT_EQ((ret != E_OK), true);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->Delete(uri, predicates);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_Query_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    AbilityRuntime::DataSharePredicates predicates;
    std::vector<std::string> columns;
    AbilityRuntime::DatashareBusinessError businessError;
    auto ret = ringtoneDataShareStubImpl->Query(uri, predicates, columns, businessError);
    EXPECT_EQ(ret, nullptr);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->Query(uri, predicates, columns, businessError);
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_BatchInsert_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    std::vector<AbilityRuntime::DataShareValuesBucket> values;
    int ret = ringtoneDataShareStubImpl->BatchInsert(uri, values);
    EXPECT_EQ(ret, E_ERR);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->BatchInsert(uri, values);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_RegisterObserver_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    bool ret = ringtoneDataShareStubImpl->RegisterObserver(uri, dataObserver);
    EXPECT_EQ(ret, true);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->RegisterObserver(uri, dataObserver);
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_UnregisterObserver_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    sptr<AAFwk::IDataAbilityObserver> dataObserver;
    bool ret = ringtoneDataShareStubImpl->UnregisterObserver(uri, dataObserver);
    EXPECT_EQ(ret, true);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->UnregisterObserver(uri, dataObserver);
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_NotifyChange_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    bool ret = ringtoneDataShareStubImpl->NotifyChange(uri);
    EXPECT_EQ(ret, true);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->NotifyChange(uri);
    EXPECT_EQ(ret, false);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_NormalizeUri_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    auto ret = ringtoneDataShareStubImpl->NormalizeUri(uri);
    Uri urivalue("");
    EXPECT_EQ(ret, uri);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->NormalizeUri(uri);
    EXPECT_EQ(ret, urivalue);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_DenormalizeUri_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    auto ret = ringtoneDataShareStubImpl->DenormalizeUri(uri);
    Uri urivalue("");
    EXPECT_EQ(ret, uri);

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->DenormalizeUri(uri);
    EXPECT_EQ(ret, urivalue);
}

HWTEST_F(RingtoneDataShareStubImplUnitTest, dataShareStubImpl_GetType_test_001, TestSize.Level0)
{
    const std::unique_ptr<AbilityRuntime::Runtime> runtime;

    napi_env env = nullptr;
    auto dataShareExtension = AbilityRuntime::RingtoneDataShareExtension::Create(runtime);
    shared_ptr<DataShare::RingtoneDataShareExtension> extension(dataShareExtension);
    sptr<DataShare::RingtoneDataShareStubImpl> ringtoneDataShareStubImpl =
        new DataShare::RingtoneDataShareStubImpl(extension, env);
    Uri uri(RINGTONE_PATH_URI);
    auto ret = ringtoneDataShareStubImpl->GetType(uri);
    EXPECT_EQ(ret, "");

    ringtoneDataShareStubImpl->extension_ = nullptr;
    ret = ringtoneDataShareStubImpl->GetType(uri);
    EXPECT_EQ(ret, "");
}
} // namespace Media
} // namespace OHOS
