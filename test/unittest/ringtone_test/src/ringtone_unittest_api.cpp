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

#include "ringtone_unittest_api.h"

#include "datashare_helper.h"
#include "get_self_permissions.h"
#include "iservice_registry.h"
#include "ringtone_asset.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_fetch_result.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_scanner_utils.h"
#include "ringtone_tracer.h"
#include "ringtone_type.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;
using namespace OHOS::DataShare;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace Media {
constexpr int STORAGE_MANAGER_ID = 5003;
std::shared_ptr<DataShare::DataShareHelper> g_dsHelper = nullptr;
const string SELECTION = RINGTONE_COLUMN_TONE_ID + " <> ? LIMIT 1, 3 ";
const string ZERO = "0";

void RingtoneUnitApiTest::SetUpTestCase()
{
    uint64_t tokenId = 0;
    auto ret = RingtonePermissionUtilsUnitTest::SetHapPermission("com.ohos.ringtonelibrary.ringtonelibrarydata",
        tokenId, false);
    EXPECT_EQ(ret, E_SUCCESS);

    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(saManager != nullptr);

    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_ID);
    ASSERT_TRUE(remoteObj != nullptr);

    g_dsHelper = DataShare::DataShareHelper::Creator(remoteObj, RINGTONE_URI);
    ASSERT_TRUE(g_dsHelper != nullptr);
}

void RingtoneUnitApiTest::TearDownTestCase()
{
    RINGTONE_ERR_LOG("TearDownTestCase start");
    if (g_dsHelper != nullptr) {
        g_dsHelper->Release();
    }
    RINGTONE_INFO_LOG("TearDownTestCase end");
}

void RingtoneUnitApiTest::SetUp(void) {}

void RingtoneUnitApiTest::TearDown(void)
{
    system("rm -rf /storage/media/100/local/data/com.ohos.ringtonelibrary.ringtonelibrarydata/*");
}

HWTEST_F(RingtoneUnitApiTest, medialib_ringtoneRead_fail_test_001, TestSize.Level0)
{
    Uri uri(RINGTONE_PATH_URI);
    DataSharePredicates predicates;
    vector<string> selectionArgs = { ZERO };
    predicates.SetWhereClause(SELECTION);
    predicates.SetWhereArgs(selectionArgs);
    vector<string> columns {
        RINGTONE_COLUMN_TONE_ID,
        RINGTONE_COLUMN_DATA,
        RINGTONE_COLUMN_SIZE,
        RINGTONE_COLUMN_DISPLAY_NAME,
        RINGTONE_COLUMN_TITLE,
        RINGTONE_COLUMN_MEDIA_TYPE,
        RINGTONE_COLUMN_TONE_TYPE,
        RINGTONE_COLUMN_MIME_TYPE,
    };

    RingtoneTracer tracer;
    auto result = g_dsHelper->Query(uri, predicates, columns);
    EXPECT_EQ(result, nullptr);
    tracer.Finish();
}
} // namespace Media
} // namespace OHOS
