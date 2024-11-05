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

#include <dualfwk_conf_parser.h>

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
DualFwkConfParser::DualFwkConfParser(const std::string &path)
    : version_(0), path_(path)
{
}

int32_t DualFwkConfParser::Parse()
{
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path_.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        RINGTONE_ERR_LOG("failed to read xml file");
        return E_ERR;
    }

    auto rootNode = xmlDocGetRootElement(docPtr.get());
    if (rootNode == nullptr) {
        RINGTONE_ERR_LOG("failed to read root node");
        return E_ERR;
    }

    if (!xmlStrcmp(rootNode->name, BAD_CAST"settings")) {
        xmlChar* xmlVersion = xmlGetProp(rootNode, BAD_CAST"version");
        try {
            version_ = std::stoi((const char *)xmlVersion);
            RINGTONE_INFO_LOG("xml verison=%{public}d", version_);
        } catch (const std::invalid_argument& e) {
            RINGTONE_INFO_LOG("invalid argument: %{public}s", e.what());
        } catch (const std::out_of_range& e) {
            RINGTONE_INFO_LOG("out of range: %{public}s", e.what());
        }

        if (xmlVersion != nullptr) {
            version_ = std::stoi((const char *)xmlVersion);
            RINGTONE_INFO_LOG("xml verison=%{public}d", version_);
            xmlFree(xmlVersion);
        } else {
            RINGTONE_INFO_LOG("xml version is null");
        }
    } else {
        RINGTONE_ERR_LOG("root node name is not matched");
        return E_ERR;
    }

    for (auto node = rootNode->children; node != nullptr; node = node->next) {
        if (!xmlStrcmp(node->name, BAD_CAST"setting")) {
            ParseConf(node);
        } else {
            RINGTONE_INFO_LOG("bad node name: %{public}s", node->name);
        }
    }

    return E_SUCCESS;
}

int32_t DualFwkConfParser::ParseConf(xmlNodePtr node)
{
    auto conf = std::make_unique<DualFwkConfRow>();
    xmlChar* xmlAttrVal = xmlGetProp(node, BAD_CAST"id");

    try {
        int num = std::stoi((const char *)xmlAttrVal);
        conf->id = num;
    } catch (const std::invalid_argument& e) {
        conf->id = -1;
        RINGTONE_INFO_LOG("invalid argument: %{public}s", e.what());
    } catch (const std::out_of_range& e) {
        conf->id = -1;
        RINGTONE_INFO_LOG("out of range: %{public}s", e.what());
    }
    auto strVal = reinterpret_cast<char*>(xmlGetProp(node, BAD_CAST"name"));
    if (strVal != nullptr) {
        conf->name = std::string(strVal);
    }

    strVal = reinterpret_cast<char*>(xmlGetProp(node, BAD_CAST"value"));
    if (strVal != nullptr) {
        conf->value = std::string(strVal);
    }

    strVal = reinterpret_cast<char*>(xmlGetProp(node, BAD_CAST"package"));
    if (strVal != nullptr) {
        conf->package = std::string(strVal);
    }

    strVal = reinterpret_cast<char*>(xmlGetProp(node, BAD_CAST"defaultValue"));
    if (strVal != nullptr) {
        conf->defaultValue = std::string(strVal);
    }

    strVal = reinterpret_cast<char*>(xmlGetProp(node, BAD_CAST"defaultSysSet"));
    if (strVal != nullptr) {
        conf->defaultSysSet = std::string(strVal);
    }

    strVal = reinterpret_cast<char*>(xmlGetProp(node, BAD_CAST"preserve_in_restore"));
    if (strVal != nullptr) {
        conf->preserveInRestore = (std::string(strVal) == "true" ? true : false);
    }

    dualFwkConfs_.push_back(std::move(conf));

    return E_SUCCESS;
}

void DualFwkConfParser::ConfTraval(std::function<void (std::unique_ptr<DualFwkConfRow> &)> func)
{
    RINGTONE_INFO_LOG("dualfwk confs num: %{public}zu", dualFwkConfs_.size());
    for (size_t i = 0; i < dualFwkConfs_.size(); i++) {
        func(dualFwkConfs_[i]);
    }
}
} // namespace Media
} // namespace OHOS
