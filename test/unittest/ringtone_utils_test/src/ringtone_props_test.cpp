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

#include "ringtone_props_test.h"

#include <algorithm>

#include "ability_context_impl.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#define private public
#include "ringtone_props.h"
#undef private
#include "ringtone_rdbstore.h"


using std::string;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const string PATH = "ringtone/audio";
const string DEFAULT_STR = "";

void RingtonePropsTest::SetUpTestCase() {}
void RingtonePropsTest::TearDownTestCase() {}
void RingtonePropsTest::SetUp() {}
void RingtonePropsTest::TearDown(void) {}

HWTEST_F(RingtonePropsTest, ringtoneProps_GetProp_Test_001, TestSize.Level0)
{
    shared_ptr<NativeRdb::RdbStore> rdb = nullptr;
    RingtoneProps props(rdb);
    auto ret = props.Init();
    ASSERT_EQ(ret, E_HAS_DB_ERROR);
    const string name = "test_name";
    const string defaultVal = "false";
    auto retStr = props.GetProp(name, defaultVal);
    ASSERT_EQ(retStr, defaultVal);
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);
    auto rawRdb = uniStore->GetRaw();
    RingtoneProps propsObj(rawRdb);
    ret = propsObj.Init();
    ASSERT_EQ(ret, E_OK);
    retStr = propsObj.GetProp(name, defaultVal);
    ASSERT_EQ(retStr, defaultVal);
}

HWTEST_F(RingtonePropsTest, ringtoneProps_GetProp_Test_002, TestSize.Level0)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    auto ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);

    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    const string name = "test_name";
    NativeRdb::ValuesBucket values;
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    const string data = ROOT_TONE_PRELOAD_PATH_NOAH_PATH + "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    values.Put(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, static_cast<int>(1));
    cmd.SetValueBucket(values);
    int64_t rowId = E_HAS_DB_ERROR;
    ret = uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);

    auto rawRdb = uniStore->GetRaw();
    rawRdb = uniStore->GetRaw();
    RingtoneProps props(rawRdb);
    ret = props.Init();
    ASSERT_EQ(ret, E_OK);
    const string defaultVal = "false";
    auto retStr = props.GetProp(name, defaultVal);
    ASSERT_EQ(retStr, defaultVal);
}

HWTEST_F(RingtonePropsTest, ringtoneProps_GetProp_Test_003, TestSize.Level0)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    auto ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);

    auto rawRdb = uniStore->GetRaw();
    rawRdb = uniStore->GetRaw();
    RingtoneProps props(rawRdb);
    ret = props.Init();
    ASSERT_EQ(ret, E_OK);
    const string name = "test_name";
    const string defaultVal = "false";
    uniStore->Stop();
    auto retStr = props.GetProp(name, defaultVal);
    ASSERT_EQ(retStr, defaultVal);
}

HWTEST_F(RingtonePropsTest, ringtoneProps_SetProp_Test_001, TestSize.Level0)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);
    auto rawRdb = uniStore->GetRaw();
    RingtoneProps propsObj(rawRdb);
    ret = propsObj.Init();
    ASSERT_EQ(ret, E_OK);
    const string name = "test_name";
    const string defaultVal = "false";
    auto retStr = propsObj.SetProp(name, defaultVal);
    ASSERT_EQ(retStr, true);
}

HWTEST_F(RingtonePropsTest, ringtoneProps_SetProp_Test_002, TestSize.Level0)
{
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);

    Uri uri(RINGTONE_PATH_URI);
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::INSERT);
    const string name = "test_name";
    NativeRdb::ValuesBucket values;
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, name);
    const string data = ROOT_TONE_PRELOAD_PATH_NOAH_PATH + "rdbStore_Insert_test_001";
    values.PutString(RINGTONE_COLUMN_DATA, data);
    const string title = "insert test";
    values.PutString(RINGTONE_COLUMN_TITLE, title);
    values.Put(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, static_cast<int>(1));
    cmd.SetValueBucket(values);
    uniStore->Init();
    int64_t rowId = E_HAS_DB_ERROR;
    ret = uniStore->Insert(cmd, rowId);
    EXPECT_EQ(ret, E_OK);

    auto rawRdb = uniStore->GetRaw();
    rawRdb = uniStore->GetRaw();
    RingtoneProps props(rawRdb);
    ret = props.Init();
    ASSERT_EQ(ret, E_OK);
    const string defaultVal = "false";
    auto retStr = props.SetProp(name, defaultVal);
    ASSERT_EQ(retStr, true);
}

HWTEST_F(RingtonePropsTest, ringtoneProps_SetProp_Test_003, TestSize.Level0)
{
    shared_ptr<NativeRdb::RdbStore> rdb = nullptr;
    RingtoneProps props(rdb);
    const string name = "test_name";
    const string defaultVal = "false";
    auto retStr = props.SetProp(name, defaultVal);
    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    int32_t ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);
    auto rawRdb = uniStore->GetRaw();
    RingtoneProps propsObj(rawRdb);
    ret = propsObj.Init();
    ASSERT_EQ(ret, E_OK);
    uniStore->Stop();
    retStr = propsObj.SetProp(name, defaultVal);
    ASSERT_EQ(retStr, true);
}

HWTEST_F(RingtonePropsTest, ringtoneProps_GetPropFromResultSet_Test_001, TestSize.Level0)
{
    shared_ptr<NativeRdb::RdbStore> rdb = nullptr;
    RingtoneProps props(rdb);
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    string propVal = "false";
    auto ret = props.GetPropFromResultSet(resultSet, propVal);
    ASSERT_EQ(ret, E_INVALID_ARGUMENTS);

    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    auto abilityContextImpl = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    shared_ptr<RingtoneUnistore> uniStore = RingtoneRdbStore::GetInstance(abilityContextImpl);
    ret = uniStore->Init();
    EXPECT_EQ(ret, E_OK);
    auto rawRdb = uniStore->GetRaw();
    const string name = "test_name";
    auto queryResultSet = rawRdb->QuerySql(name);
    EXPECT_NE(queryResultSet, nullptr);
    ret = props.GetPropFromResultSet(queryResultSet, propVal);
    ASSERT_EQ(ret, E_HAS_DB_ERROR);
}
} // namespace Media
} // namespace OHOS
