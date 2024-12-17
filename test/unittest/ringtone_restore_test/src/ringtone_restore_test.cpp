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

#define MLOG_TAG "RingtoneRestoreTest"

#include "ringtone_restore_test.h"

#define private public
#define protected public
#include "ringtone_restore.h"
#include "ringtone_restore_base.h"
#include "ringtone_restore_napi.h"
#include "ringtone_dualfwk_restore.h"
#undef protected
#undef private
#include "ringtone_file_utils.h"
#include "ringtone_source.h"
#include "ringtone_restore_factory.h"
#include "ringtone_unittest_utils.h"
#include "ringtone_rdbstore.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ability_context_impl.h"
#include "datashare_helper.h"
#include "iservice_registry.h"
#include "ringtone_restore_type.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {

void RingtoneRestoreTest::SetUpTestCase(void) {}

void RingtoneRestoreTest::TearDownTestCase(void) {}

void RingtoneRestoreTest::SetUp() {}

void RingtoneRestoreTest::TearDown() {}

} // namespace Media
} // namespace OHOS