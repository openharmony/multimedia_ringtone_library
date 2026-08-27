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

#define MLOG_TAG "ringtone_language"

#include "ringtone_language_manager.h"

#include "parameter.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"
#include "ringtone_type.h"
#include "ringtone_file_utils.h"
#ifdef USE_CONFIG_POLICY
#include "config_policy_utils.h"
#endif

#include <cstring>
#include <libxml/tree.h>
#include <libxml/parser.h>

namespace OHOS {
namespace Media {
using namespace OHOS::NativeRdb;
using namespace std;

const char *LANGUAGE_KEY = "persist.global.language";
const char *DEFAULT_LANGUAGE_KEY = "const.global.language";
const string CHINESE_ABBREVIATION = "zh-Hans";
const string ENGLISH_ABBREVIATION = "en-Latn-US";
const int32_t SYSPARA_SIZE = 64;
const int32_t SYSINIT_TYPE = 1;
const int32_t STANDARDVIBRATION = 1;
const int32_t UNKNOWN_INDEX = -1;
#ifdef USE_CONFIG_POLICY
static constexpr char RINGTONE_MULTILINGUAL_FILE_PATH[] =
    "etc/resource/media/audio/ringtone_list_language.xml";
static constexpr char VIBRATION_MULTILINGUAL_FILE_PATH[] =
    "etc/resource/media/haptics/vibration_list_language.xml";
#else
static constexpr char RINGTONE_MULTILINGUAL_FILE_PATH[] =
    "/system/variant/phone/base/etc/resource/media/audio/ringtone_list_language.xml";
static constexpr char VIBRATION_MULTILINGUAL_FILE_PATH[] =
    "/system/variant/phone/base/etc/resource/media/haptics/vibration_list_language.xml";
#endif

shared_ptr<RingtoneLanguageManager> RingtoneLanguageManager::instance_ = nullptr;
mutex RingtoneLanguageManager::mutex_;

RingtoneLanguageManager::RingtoneLanguageManager(void)
{
}

RingtoneLanguageManager::~RingtoneLanguageManager(void)
{
}

shared_ptr<RingtoneLanguageManager> RingtoneLanguageManager::GetInstance()
{
    if (instance_ == nullptr) {
        lock_guard<mutex> lock(mutex_);

        if (instance_ == nullptr) {
            instance_ = make_shared<RingtoneLanguageManager>();
        }
    }
    return instance_;
}

/**
 * @brief 同步铃声和振动资源的显示语言。
 *
 * 执行流程：
 * 1. 通过GetSystemLanguage()获取当前系统语言设置；
 * 2. 将语言统一映射为中文缩写（zh-Hans）或英文缩写（en-Latn-US），
 *    以zh-Hans前缀匹配的归为中文，其余归为英文；
 * 3. 调用UpdateRingtoneLanguage()更新铃声的多语言显示名；
 * 4. 调用UpdateVibrationLanguage()更新振动的多语言显示名。
 *
 * 当系统语言获取失败时直接返回，不做任何更新。
 */
void RingtoneLanguageManager::SyncAssetLanguage()
{
    RINGTONE_INFO_LOG("SyncAssetLanguage start.");
    systemLanguage_ = GetSystemLanguage();
    if (systemLanguage_.empty()) {
        RINGTONE_ERR_LOG("Failed to get system language");
        return;
    }
    RINGTONE_INFO_LOG("system language is %{public}s", systemLanguage_.c_str());
    if (strncmp(systemLanguage_.c_str(), CHINESE_ABBREVIATION.c_str(), CHINESE_ABBREVIATION.size()) == 0) {
        systemLanguage_ = CHINESE_ABBREVIATION;
    } else {
        systemLanguage_ = ENGLISH_ABBREVIATION;
    }
    RINGTONE_INFO_LOG("need changed system language is %{public}s", systemLanguage_.c_str());
    UpdateRingtoneLanguage();
    UpdateVibrationLanguage();
    RINGTONE_INFO_LOG("SyncAssetLanguage end.");
}

/**
 * @brief 获取当前系统语言设置。
 *
 * 依次尝试从两个系统参数读取语言：
 * 1. "persist.global.language"（用户持久化语言设置）；
 * 2. 若读取失败，回退到"const.global.language"（默认语言设置）。
 *
 * @return 系统语言字符串，读取失败时返回空字符串。
 */
string RingtoneLanguageManager::GetSystemLanguage()
{
    char param[SYSPARA_SIZE] = {0};
    int status = GetParameter(LANGUAGE_KEY, "", param, SYSPARA_SIZE);
    if (status > 0) {
        return param;
    }
    status = GetParameter(DEFAULT_LANGUAGE_KEY, "", param, SYSPARA_SIZE);
    if (status > 0) {
        return param;
    }
    return "";
}

/**
 * @brief 更新铃声资源的多语言显示名。
 *
 * 执行流程：
 * 1. 调用CheckLanguageTypeByRingtone()查询需要语言同步的铃声记录
 *    （display_language_type与当前系统语言不一致或为空的预置铃声）；
 * 2. 若无待同步记录则直接返回；
 * 3. 通过配置策略（USE_CONFIG_POLICY）或固定路径获取多语言XML资源文件；
 * 4. 调用ReadMultilingualResources()解析XML中的翻译映射；
 * 5. 调用ChangeLanguageDataToRingtone()将翻译后的显示名写入数据库。
 *
 * 任何中间步骤失败都会提前返回，不执行后续操作。
 */
void RingtoneLanguageManager::UpdateRingtoneLanguage()
{
    RINGTONE_INFO_LOG("UpdateRingtonLanguage start.");
    int32_t rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    if (CheckLanguageTypeByRingtone(rowCount, resultSet) != E_OK) {
        return;
    }
    RINGTONE_WARN_LOG("%{public}d ring tones need to be sync", rowCount);
    if (rowCount == 0) {
        return;
    }
#ifdef USE_CONFIG_POLICY
    char buf[MAX_PATH_LEN] = {0};
    char *path = GetOneCfgFile(RINGTONE_MULTILINGUAL_FILE_PATH, buf, MAX_PATH_LEN);
    if (path == nullptr || *path == '\0') {
        RINGTONE_ERR_LOG("GetOneCfgFile for %{public}s failed.", RINGTONE_MULTILINGUAL_FILE_PATH);
        return;
    }
#else
    const char *path = RINGTONE_MULTILINGUAL_FILE_PATH;
#endif

    if (!ReadMultilingualResources(path, RINGTONE_FILE)) {
        return;
    }
    ChangeLanguageDataToRingtone(rowCount, resultSet);
    RINGTONE_INFO_LOG("UpdateRingtonLanguage end.");
}

/**
 * @brief 更新振动资源的多语言显示名。
 *
 * 执行流程：
 * 1. 调用CheckLanguageTypeByVibration()查询需要语言同步的振动记录
 *    （display_language与当前系统语言不一致或为空的标准预置振动）；
 * 2. 若无待同步记录则直接返回；
 * 3. 通过配置策略（USE_CONFIG_POLICY）或固定路径获取振动多语言XML资源文件；
 * 4. 调用ReadMultilingualResources()解析XML中的翻译映射；
 * 5. 调用ChangeLanguageDataToVibration()将翻译后的显示名写入数据库。
 *
 * 任何中间步骤失败都会提前返回，不执行后续操作。
 */
void RingtoneLanguageManager::UpdateVibrationLanguage()
{
    RINGTONE_INFO_LOG("UpdateVibrationLanguage start.");
    int32_t rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    if (CheckLanguageTypeByVibration(rowCount, resultSet) != E_OK) {
        return;
    }
    RINGTONE_INFO_LOG("%{public}d vibration need to be sync", rowCount);
    if (rowCount == 0) {
        return;
    }
#ifdef USE_CONFIG_POLICY
    char buf[MAX_PATH_LEN] = {0};
    char *path = GetOneCfgFile(VIBRATION_MULTILINGUAL_FILE_PATH, buf, MAX_PATH_LEN);
    if (path == nullptr || *path == '\0') {
        RINGTONE_ERR_LOG("GetOneCfgFile for %{public}s failed.", VIBRATION_MULTILINGUAL_FILE_PATH);
        return;
    }
#else
    const char *path = VIBRATION_MULTILINGUAL_FILE_PATH;
#endif

    if (!ReadMultilingualResources(path, VIBRATION_FILE)) {
        return;
    }
    ChangeLanguageDataToVibration(rowCount, resultSet);
    RINGTONE_INFO_LOG("UpdateVibrationLanguage end.");
}

/**
 * @brief 查询需要语言同步的预置铃声记录。
 *
 * 查询条件：
 * - source_type = 1（系统预置铃声）；
 * - display_language_type不等于当前系统语言，或display_language_type为NULL。
 *
 * 查询结果包含tone_id和data两列，用于后续遍历更新显示名和语言标记。
 *
 * @param rowCount 输出参数，满足条件的记录行数。
 * @param resultSet 输出参数，查询结果集。
 * @return E_OK表示查询成功，E_RDB表示RDB实例获取失败或查询失败。
 */
int32_t RingtoneLanguageManager::CheckLanguageTypeByRingtone(int32_t &rowCount,
    shared_ptr<ResultSet> &resultSet)
{
    vector<string> columns = {
        RINGTONE_COLUMN_TONE_ID,
        RINGTONE_COLUMN_DATA
    };

    auto rdbStore = RingtoneRdbStore::GetInstance();
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("failed to get rdb");
        return E_RDB;
    }
    auto rawRdb = rdbStore->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("failed to get raw rdb");
        return E_RDB;
    }

    AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
    absRdbPredicates.EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, SYSINIT_TYPE);
    absRdbPredicates.And();
    absRdbPredicates.BeginWrap();
    absRdbPredicates.NotEqualTo(RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE, systemLanguage_);
    absRdbPredicates.Or();
    absRdbPredicates.IsNull(RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE);
    absRdbPredicates.EndWrap();
    resultSet = rawRdb->Query(absRdbPredicates, columns);
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("failed to query rdb");
        return E_RDB;
    }

    int32_t ret = resultSet->GetRowCount(rowCount);
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get resultSet row count");
        return E_RDB;
    }
    return E_OK;
}

void RingtoneLanguageManager::ChangeLanguageDataToRingtone(int32_t rowCount,
    const std::shared_ptr<ResultSet> &resultSet)
{
    auto rdbStore = RingtoneRdbStore::GetInstance();
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("failed to get rdb");
        return;
    }
    auto rawRdb = rdbStore->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("failed to get raw rdb");
        return;
    }

    map<string, int> fieldIndex = {
        { RINGTONE_COLUMN_TONE_ID, UNKNOWN_INDEX },
        { RINGTONE_COLUMN_DATA, UNKNOWN_INDEX }
    };
    if (GetFieldIndex(resultSet, fieldIndex) != E_OK) {
        return;
    }

    for (int i = 0; i < rowCount; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            RINGTONE_ERR_LOG("failed to goto row : %{public}d", i);
            return;
        }

        ValuesBucket values;
        int ringtoneId;
        if (SetValuesFromResultSet(resultSet, fieldIndex, values, ringtoneId, RINGTONE_FILE) == E_OK) {
            AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
            absRdbPredicates.EqualTo(RINGTONE_COLUMN_TONE_ID, ringtoneId);
            int32_t changedRows;
            int32_t result = rawRdb->Update(changedRows, values, absRdbPredicates);
            if (result != E_OK || changedRows <= 0) {
                RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, changedRows);
                return;
            }
        }
    }
}

/**
 * @brief 获取结果集中指定字段的列索引。
 *
 * 遍历fieldIndex映射中的每个字段名，通过ResultSet的GetColumnIndex
 * 获取该字段在结果集中的列索引号，更新到fieldIndex的value中。
 *
 * @param resultSet 数据库查询结果集。
 * @param fieldIndex 字段名到列索引的映射（输入为字段名，输出填充列索引）。
 * @return E_OK表示全部字段索引获取成功，E_RDB表示任一字段获取失败。
 */
int32_t RingtoneLanguageManager::GetFieldIndex(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
    std::map<std::string, int> &fieldIndex)
{
    for (auto& field : fieldIndex) {
        if (resultSet->GetColumnIndex(field.first, field.second) != E_OK) {
            RINGTONE_ERR_LOG("failed to get field index");
            return E_RDB;
        }
    }
    return E_OK;
}

/**
 * @brief 从结果集当前行构建语言更新的ValuesBucket。
 *
 * 根据资源文件类型（铃声/振动）选择对应的列名和翻译映射：
 * 1. 从结果集读取记录ID（tone_id或vibrate_id）和文件路径（data）；
 * 2. 将display_language_type/display_language设置为当前系统语言；
 * 3. 从文件路径中提取文件基本名（不含目录和扩展名），
 *    在翻译映射中查找对应的本地化显示名（title）；
 * 4. 若找到翻译则设置title字段，否则仅设置语言标记。
 *
 * @param resultSet 数据库查询结果集，已定位到目标行。
 * @param fieldIndex 字段名到列索引的映射。
 * @param values 输出参数，构建的ValuesBucket用于Update操作。
 * @param indexId 输出参数，当前记录的ID值。
 * @param resourceFileType 资源文件类型（RINGTONE_FILE或VIBRATION_FILE）。
 * @return E_OK表示成功，E_RDB表示读取结果集失败。
 */
int32_t RingtoneLanguageManager::SetValuesFromResultSet(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
    const std::map<std::string, int> &fieldIndex, NativeRdb::ValuesBucket &values, int32_t &indexId,
    ResourceFileType resourceFileType)
{
    string data;
    string idIndexField = resourceFileType == RINGTONE_FILE ? RINGTONE_COLUMN_TONE_ID : VIBRATE_COLUMN_VIBRATE_ID;
    string dataIndexField = resourceFileType == RINGTONE_FILE ? RINGTONE_COLUMN_DATA : VIBRATE_COLUMN_DATA;
    string titleIndexField = resourceFileType == RINGTONE_FILE ? RINGTONE_COLUMN_TITLE : VIBRATE_COLUMN_TITLE;
    string languageIndexField = resourceFileType == RINGTONE_FILE ?
        RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE : VIBRATE_COLUMN_DISPLAY_LANGUAGE;
    auto& translation = resourceFileType == RINGTONE_FILE ? ringtoneTranslate_ : vibrationTranslate_;

    auto idItem = fieldIndex.find(idIndexField);
    if (idItem == fieldIndex.end()) {
        RINGTONE_ERR_LOG("failed to get %{public}s index", idIndexField.c_str());
        return E_RDB;
    }
    if (resultSet->GetInt(idItem->second, indexId) != E_OK) {
        RINGTONE_ERR_LOG("failed to get tone_id value");
        return E_RDB;
    }

    auto dataItem = fieldIndex.find(dataIndexField);
    if (dataItem == fieldIndex.end()) {
        RINGTONE_ERR_LOG("failed to get %{public}s index", dataIndexField.c_str());
        return E_RDB;
    }
    if (resultSet->GetString(dataItem->second, data) != E_OK) {
        RINGTONE_ERR_LOG("failed to get tone_id value");
        return E_RDB;
    }

    values.PutString(languageIndexField, systemLanguage_);
    string realName = RingtoneFileUtils::GetBaseNameFromPath(data);
    auto item = translation[systemLanguage_].find(realName);
    if (item == translation[systemLanguage_].end()) {
        return E_OK;
    }
    string titleName = item->second;
    values.PutString(titleIndexField, titleName);
    return E_OK;
}

/**
 * @brief 查询需要语言同步的预置振动记录。
 *
 * 查询条件：
 * - vibrate_type = 1（标准振动）；
 * - display_language不等于当前系统语言，或display_language为NULL。
 *
 * 查询结果包含vibrate_id和data两列，用于后续遍历更新显示名和语言标记。
 * 与CheckLanguageTypeByRingtone()的区别：振动表按vibrate_type过滤（仅标准振动），
 * 铃声表按source_type过滤（所有预置铃声）。
 *
 * @param rowCount 输出参数，满足条件的记录行数。
 * @param resultSet 输出参数，查询结果集。
 * @return E_OK表示查询成功，E_RDB表示RDB实例获取失败或查询失败。
 */
int32_t RingtoneLanguageManager::CheckLanguageTypeByVibration(int32_t &rowCount,
    std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    vector<string> columns = {
        VIBRATE_COLUMN_VIBRATE_ID,
        VIBRATE_COLUMN_DATA
    };

    auto rdbStore = RingtoneRdbStore::GetInstance();
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("failed to get rdb");
        return E_RDB;
    }
    auto rawRdb = rdbStore->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("failed to get raw rdb");
        return E_RDB;
    }

    AbsRdbPredicates absRdbPredicates(VIBRATE_TABLE);
    absRdbPredicates.EqualTo(VIBRATE_COLUMN_VIBRATE_TYPE, STANDARDVIBRATION);
    absRdbPredicates.And();
    absRdbPredicates.BeginWrap();
    absRdbPredicates.NotEqualTo(VIBRATE_COLUMN_DISPLAY_LANGUAGE, systemLanguage_);
    absRdbPredicates.Or();
    absRdbPredicates.IsNull(VIBRATE_COLUMN_DISPLAY_LANGUAGE);
    absRdbPredicates.EndWrap();
    resultSet = rawRdb->Query(absRdbPredicates, columns);
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("failed to query rdb");
        return E_RDB;
    }

    int32_t ret = resultSet->GetRowCount(rowCount);
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get resultSet row count");
        return E_RDB;
    }
    return E_OK;
}

/**
 * @brief 将多语言翻译应用到振动数据库记录。
 *
 * 遍历查询结果集中的每一行：
 * 1. 通过GetFieldIndex()获取vibrate_id和data列的索引；
 * 2. 调用SetValuesFromResultSet()根据文件名从振动翻译映射中查找对应显示名，
 *    构建ValuesBucket（包含display_language和title）；
 * 3. 根据vibrate_id更新数据库记录。
 *
 * 与ChangeLanguageDataToRingtone()逻辑对称，区别在于操作VIBRATE表
 * 和使用vibrationTranslate_翻译映射。
 * 任一记录更新失败或行跳转失败时立即终止后续处理。
 *
 * @param rowCount 待更新的记录行数。
 * @param resultSet 包含vibrate_id和data列的查询结果集。
 */
void RingtoneLanguageManager::ChangeLanguageDataToVibration(int32_t rowCount,
    const std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    auto rdbStore = RingtoneRdbStore::GetInstance();
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("failed to get rdb");
        return;
    }
    auto rawRdb = rdbStore->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("failed to get raw rdb");
        return;
    }

    map<string, int> fieldIndex = {
        { VIBRATE_COLUMN_VIBRATE_ID, UNKNOWN_INDEX },
        { VIBRATE_COLUMN_DATA, UNKNOWN_INDEX }
    };
    if (GetFieldIndex(resultSet, fieldIndex) != E_OK) {
        return;
    }

    for (int i = 0; i < rowCount; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            RINGTONE_ERR_LOG("failed to goto row : %{public}d", i);
            return;
        }

        ValuesBucket values;
        int vibrateId;
        if (SetValuesFromResultSet(resultSet, fieldIndex, values, vibrateId, VIBRATION_FILE) == E_OK) {
            AbsRdbPredicates absRdbPredicates(VIBRATE_TABLE);
            absRdbPredicates.EqualTo(VIBRATE_COLUMN_VIBRATE_ID, vibrateId);
            int32_t changedRows;
            int32_t result = rawRdb->Update(changedRows, values, absRdbPredicates);
            if (result != E_OK || changedRows <= 0) {
                RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, changedRows);
                return;
            }
        }
    }
}

/**
 * @brief 读取并解析多语言资源XML文件。
 *
 * 执行流程：
 * 1. 使用libxml2的xmlReadFile()加载XML文件；
 * 2. 获取XML根节点并校验根节点名称：
 *    - 铃声文件：根节点应为"RingtoneList"；
 *    - 振动文件：根节点应为"VibrationList"；
 * 3. 校验通过后清空对应的翻译映射（ringtoneTranslate_或vibrationTranslate_）；
 * 4. 调用ParseMultilingualXml()解析子节点填充翻译映射。
 *
 * XML文件路径通过配置策略（USE_CONFIG_POLICY宏）动态获取或使用固定路径。
 *
 * @param filePath 多语言XML资源文件的完整路径。
 * @param resourceFileType 资源文件类型（RINGTONE_FILE或VIBRATION_FILE）。
 * @return true表示解析成功，false表示文件读取失败或根节点校验不通过。
 */
bool RingtoneLanguageManager::ReadMultilingualResources(const string &filePath, ResourceFileType resourceFileType)
{
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(filePath.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        RINGTONE_ERR_LOG("failed to read xml file [%{public}s]", filePath.c_str());
        const xmlError *error = xmlGetLastError();
        if (error != nullptr) {
            RINGTONE_ERR_LOG("Error: %{public}s (line %{public}d): %{public}s",
                error->file, error->line, error->message);
            xmlResetLastError();
        }
        return false;
    }

    xmlNodePtr rootNode = xmlDocGetRootElement(docPtr.get());
    if (rootNode == nullptr) {
        RINGTONE_ERR_LOG("failed to read root node");
        return false;
    }
    if (resourceFileType == RINGTONE_FILE) {
        if (xmlStrcmp(rootNode->name, BAD_CAST "RingtoneList") != 0) {
            RINGTONE_ERR_LOG("failed to root node name is not matched");
            return false;
        }
        ringtoneTranslate_.clear();
    } else if (resourceFileType == VIBRATION_FILE) {
        if (xmlStrcmp(rootNode->name, BAD_CAST "VibrationList") != 0) {
            RINGTONE_ERR_LOG("failed to root node name is not matched");
            return false;
        }
        vibrationTranslate_.clear();
    }
    return ParseMultilingualXml(rootNode, resourceFileType);
}

/**
 * @brief 解析多语言XML的节点树，填充翻译映射表。
 *
 * XML结构预期：
 * @code
 * <RingtoneList>  <!-- 或 <VibrationList> -->
 *   <Language type="zh-Hans">
 *     <Ring resource_name="filename" title="显示名"/>  <!-- 或 <vibrtion> -->
 *     ...
 *   </Language>
 *   <Language type="en-Latn-US">
 *     ...
 *   </Language>
 * </RingtoneList>
 * @endcode
 *
 * 解析流程：
 * 1. 遍历根节点的子节点，查找名为"Language"的节点；
 * 2. 从Language节点的"type"属性获取语言标识；
 * 3. 遍历Language的子节点：
 *    - 铃声文件：子节点名必须为"Ring"；
 *    - 振动文件：子节点名必须为"vibrtion"；
 * 4. 从子节点的"resource_name"和"title"属性提取资源文件名和本地化显示名；
 * 5. 将映射关系存入ringtoneTranslate_[language][resourceName]或
 *    vibrationTranslate_[language][resourceName]。
 *
 * @param rootNode XML根节点指针。
 * @param resourceFileType 资源文件类型（RINGTONE_FILE或VIBRATION_FILE）。
 * @return true表示解析成功，false表示子节点名不匹配（严重格式错误）。
 */
bool RingtoneLanguageManager::ParseMultilingualXml(xmlNodePtr &rootNode, ResourceFileType resourceFileType)
{
    for (xmlNodePtr itemNode = rootNode->children; itemNode; itemNode = itemNode->next) {
        if (xmlStrcmp(itemNode->name, BAD_CAST "Language") != 0) {
            continue;
        }

        string language;
        auto xmlLanguage = reinterpret_cast<char*>(xmlGetProp(itemNode, BAD_CAST "type"));
        if (xmlLanguage != nullptr) {
            language = string(xmlLanguage);
            xmlFree(xmlLanguage);
        }

        for (xmlNodePtr childNode = itemNode->children; childNode; childNode = childNode->next) {
            if (resourceFileType == RINGTONE_FILE && xmlStrcmp(childNode->name, BAD_CAST "Ring") != 0) {
                RINGTONE_ERR_LOG("failed to ringtone child node name is not matched");
                return false;
            } else if (resourceFileType == VIBRATION_FILE && xmlStrcmp(childNode->name, BAD_CAST "vibrtion") != 0) {
                RINGTONE_ERR_LOG("failed to vibrate child node name is not matched");
                return false;
            }

            string resourceName;
            auto xmlResourceName = reinterpret_cast<char*>(xmlGetProp(childNode, BAD_CAST "resource_name"));
            if (xmlResourceName) {
                resourceName = string(xmlResourceName);
                xmlFree(xmlResourceName);
            }
            string displayName;
            auto xmlDisplayName = reinterpret_cast<char*>(xmlGetProp(childNode, BAD_CAST "title"));
            if (xmlDisplayName) {
                displayName = string(xmlDisplayName);
                xmlFree(xmlDisplayName);
            }

            if (resourceFileType == RINGTONE_FILE && !resourceName.empty() && !displayName.empty()) {
                ringtoneTranslate_[language][resourceName] = displayName;
            } else if (resourceFileType == VIBRATION_FILE && !resourceName.empty() && !displayName.empty()) {
                vibrationTranslate_[language][resourceName] = displayName;
            }
        }
    }
    return true;
}

} // namespace Media
} // namespace OHOS
