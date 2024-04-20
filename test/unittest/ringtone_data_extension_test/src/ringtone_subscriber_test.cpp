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

#include "ringtone_subscriber_test.h"

#include "common_event_support.h"
#define private public
#include "ringtone_subscriber.h"
#undef private

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
void RingtonesublUnitTest::SetUpTestCase() {}

void RingtonesublUnitTest::TearDownTestCase() {}

// SetUp:Execute before each test case
void RingtonesublUnitTest::SetUp() {}

void RingtonesublUnitTest::TearDown(void) {}

HWTEST_F(RingtonesublUnitTest, subscriber_Subscribe_test_001, TestSize.Level0)
{
    bool ret = RingtoneSubscriber::Subscribe();
    EXPECT_EQ(ret, true);
}

HWTEST_F(RingtonesublUnitTest, subscriber_OnReceiveEvent_test_001, TestSize.Level0)
{
    RingtoneSubscriber ringtoneSubscriber;
    EventFwk::CommonEventData eventData;
    string action = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF;
    AAFwk::Want want = eventData.GetWant();
    want.SetAction(action);
    eventData.SetWant(want);
    ringtoneSubscriber.OnReceiveEvent(eventData);
    EXPECT_EQ(want.GetAction(), action);
}

HWTEST_F(RingtonesublUnitTest, subscriber_OnReceiveEvent_test_002, TestSize.Level0)
{
    RingtoneSubscriber ringtoneSubscriber;
    EventFwk::CommonEventData eventData;
    string action = EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED;
    AAFwk::Want want = eventData.GetWant();
    want.SetAction(action);
    eventData.SetWant(want);
    ringtoneSubscriber.OnReceiveEvent(eventData);
    EXPECT_EQ(want.GetAction(), action);
}

HWTEST_F(RingtonesublUnitTest, subscriber_OnReceiveEvent_test_003, TestSize.Level0)
{
    RingtoneSubscriber ringtoneSubscriber;
    EventFwk::CommonEventData eventData;
    string action = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON;
    AAFwk::Want want = eventData.GetWant();
    want.SetAction(action);
    eventData.SetWant(want);
    ringtoneSubscriber.OnReceiveEvent(eventData);
    EXPECT_EQ(want.GetAction(), action);
}

HWTEST_F(RingtonesublUnitTest, subscriber_OnReceiveEvent_test_004, TestSize.Level0)
{
    RingtoneSubscriber ringtoneSubscriber;
    EventFwk::CommonEventData eventData;
    string action = EventFwk::CommonEventSupport::COMMON_EVENT_TIME_CHANGED;
    AAFwk::Want want = eventData.GetWant();
    want.SetAction(action);
    eventData.SetWant(want);
    ringtoneSubscriber.OnReceiveEvent(eventData);
    EXPECT_EQ(want.GetAction(), action);
}

HWTEST_F(RingtonesublUnitTest, subscriber_OnReceiveEvent_test_005, TestSize.Level0)
{
    RingtoneSubscriber ringtoneSubscriber;
    EventFwk::CommonEventData eventData;
    string action = EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED;
    AAFwk::Want want = eventData.GetWant();
    want.SetAction(action);
    eventData.SetWant(want);
    ringtoneSubscriber.OnReceiveEvent(eventData);
    EXPECT_EQ(want.GetAction(), action);
}
} // namespace Media
} // namespace OHOS
