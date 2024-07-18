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
#include "datashare_ext_ability.h"
#include "iservice_registry.h"
#include "medialibrary_db_const.h"
#include "file_asset.h"
#include "fetch_result.h"
#include "ringtone_restore_napi.h"
#include "ringtone_restore_factory.h"
#include "ringtone_restore_type.h"

#define private public
#define protected public
#include "ringtone_dualfw_restore.h"
#include "dualfw_conf_loader.h"
#undef protected
#undef private
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "datashare_helper.h"
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
    (void) res;
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

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_0002, TestSize.Level0)
{
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0002 start");
    
    DualfwConfLoader confLoader;
    if (confLoader.Init() != E_OK) {
        RINGTONE_ERR_LOG("Failed to initialize DualfwConfLoader.");
        return;
    }
    auto confVal = confLoader.GetConf("SETTINGSDATA_CreatedTime");
    std::cout << "conf value of SETTINGSDATA_CreatedTime = " << confVal << std::endl;
    DualFwConf conf;
    confLoader.Load(conf, RESTORE_SCENE_TYPE_DUAL_CLONE);
    confLoader.ShowConf(conf);
    confLoader.Load(conf, RESTORE_SCENE_TYPE_DUAL_UPGRADE);
    confLoader.ShowConf(conf);
    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0002 end");
}

static void MediaUriAppendKeyValue(string &uri, const string &key, const string &value)
{
    string uriKey = key + '=';
    if (uri.find(uriKey) != string::npos) {
        return;
    }

    char queryMark = (uri.find('?') == string::npos) ? '?' : '&';
    string append = queryMark + key + '=' + value;

    size_t posJ = uri.find('#');
    if (posJ == string::npos) {
        uri += append;
    } else {
        uri.insert(posJ, append);
    }
}


static int32_t checkURI(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper, const std::string & testUri)
{
    const std::string MEDIA_DATA_DB_NAME = "display_name";
    const string KEY_API_VERSION = "API_VERSION";
    std::vector<string> columns = {};
    DataShare::DataSharePredicates predicates;

    std::string queryFileUri = testUri;
    std::cout << queryFileUri << std::endl;
    MediaUriAppendKeyValue(queryFileUri, KEY_API_VERSION, to_string(MEDIA_API_VERSION_V10));
    std::cout << queryFileUri << std::endl;
    std::shared_ptr<DataShare::DataShareResultSet> resultSet = nullptr;
    Uri uri(queryFileUri);
    resultSet = dataShareHelper->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        std::cout << "empty resultSet\n";
        return 0;
    }
    int32_t numRows = 0;
    resultSet->GetRowCount(numRows);
    std::cout << numRows << " records found." << std::endl;
    if (numRows == 0) {
        std::cout << "no record." << std::endl;
        return 0;
    }

    std::unique_ptr<FetchResult<FileAsset>> fetchFileResult =
        std::make_unique<FetchResult<FileAsset>>(move(resultSet));
    auto cnt = fetchFileResult->GetCount();
    for (int i = 0; i < cnt; i++) {
        auto fileAssetPtr = fetchFileResult->GetNextObject(); // GetFirstObject
        std::cout << fileAssetPtr->GetPath()<< ":" <<  fileAssetPtr->GetDisplayName() <<
            ":" << fileAssetPtr->GetSize() << std::endl;
    }
    return numRows;
}

HWTEST_F(RingtoneDualfwRestoreTest, ringtone_dualfw_restore_test_0003, TestSize.Level0)
{
    constexpr int STORAGE_MANAGER_MANAGER_ID = 5003;
    const std::string MEDIALIBRARY_DATA_URI = "datashare:///media";

    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0003 start");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        RINGTONE_ERR_LOG("CreateFileExtHelper Get system ability mgr failed.");
        return;
    }
    auto remoteObj = saManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remoteObj == nullptr) {
        RINGTONE_ERR_LOG("CreateDataShareHelper GetSystemAbility Service Failed.");
        return;
    }
    auto dataShareHelper = DataShare::DataShareHelper::Creator(remoteObj, MEDIALIBRARY_DATA_URI);
    
    EXPECT_EQ(checkURI(dataShareHelper, URI_QUERY_AUDIO), 0);
    EXPECT_EQ(checkURI(dataShareHelper, UFM_QUERY_AUDIO), 0);
    EXPECT_EQ(checkURI(dataShareHelper, TOOL_QUERY_AUDIO), 2);
    EXPECT_EQ(checkURI(dataShareHelper, "datashare:///media/mediatool_audio_operation/query"), 2);

    auto restore = RingtoneRestoreFactory::CreateObj(RESTORE_SCENE_TYPE_DUAL_CLONE);
    restore->Init("/data/storage/el2/base/.backup/restore");
    restore->StartRestore();

    RINGTONE_INFO_LOG("ringtone_dualfw_restore_test_0003 end");
}
} // namespace Media
} // namespace OHOS