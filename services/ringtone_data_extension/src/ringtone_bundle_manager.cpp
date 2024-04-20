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

#define MLOG_TAG "RingtoneBundleManager"

#include "ringtone_bundle_manager.h"

#include "ipc_skeleton.h"
#include "permission_utils.h"

using namespace std;

namespace OHOS {
namespace Media {
const int CAPACITY = 50;
shared_ptr<RingtoneBundleManager> RingtoneBundleManager::instance_ = nullptr;
mutex RingtoneBundleManager::mutex_;
shared_ptr<RingtoneBundleManager> RingtoneBundleManager::GetInstance()
{
    if (instance_ == nullptr) {
        lock_guard<mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = make_shared<RingtoneBundleManager>();
        }
    }
    return instance_;
}

void RingtoneBundleManager::GetBundleNameByUid(const int32_t uid, string &bundleName)
{
    RingtonePermissionUtils::GetClientBundle(uid, bundleName);
    if (bundleName.empty()) {
        return;
    }

    auto it = cacheMap_.find(uid);
    if (it != cacheMap_.end()) {
        cacheList_.erase(it->second);
    }
    cacheList_.push_front(make_pair(uid, bundleName));
    cacheMap_[uid] = cacheList_.begin();
    if (cacheMap_.size() > CAPACITY) {
        int32_t deleteKey = cacheList_.back().first;
        cacheMap_.erase(deleteKey);
        cacheList_.pop_back();
    }
}

string RingtoneBundleManager::GetClientBundleName()
{
    lock_guard<mutex> lock(uninstallMutex_);
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto iter = cacheMap_.find(uid);
    if (iter == cacheMap_.end()) {
        string bundleName;
        GetBundleNameByUid(uid, bundleName);
        return bundleName;
    }
    cacheList_.splice(cacheList_.begin(), cacheList_, iter->second);
    return iter->second->second;
}

void RingtoneBundleManager::Clear()
{
    lock_guard<mutex> lock(uninstallMutex_);
    cacheList_.clear();
    cacheMap_.clear();
}
} // Ringtone
} // OHOS
