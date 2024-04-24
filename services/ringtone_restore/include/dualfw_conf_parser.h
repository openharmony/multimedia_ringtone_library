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

#ifndef DUALFW_CONF_PARSER_H_
#define DUALFW_CONF_PARSER_H_

#include <functional>
#include <string>
#include <vector>

#include <libxml/parser.h>

namespace OHOS {
namespace Media {
struct DualFwConfRow {
    int32_t id;
    std::string name;
    std::string value;
    std::string defaultValue;
    std::string package;
    std::string defaultSysSet;
    bool preserveInRestore;
};

class DualfwConfParser
{
public:
    DualfwConfParser(const std::string &path);
    ~DualfwConfParser() {};
    int32_t Parse();
    void ConfTraval(std::function<void (std::unique_ptr<DualFwConfRow> &)> func);
private:
    int32_t ParseConf(xmlNodePtr node);

    int32_t version_;
    std::string path_;
    std::vector<std::unique_ptr<DualFwConfRow>> DualFwConfs_ = {};
};
} // namespace Media
} // namespace OHOS
#endif // DUALFW_CONF_PARSER_H_
