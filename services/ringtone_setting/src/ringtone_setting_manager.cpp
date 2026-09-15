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
#define MLOG_TAG "RingtoneSettingManager"

#include "ringtone_setting_manager.h"

#include "rdb_errno.h"
#include "result_set_utils.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_metadata.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
using namespace std;
static const string RINGTONE_SETTINGS_SELECT_COLUMNS =
    RINGTONE_COLUMN_TONE_ID + ", " + RINGTONE_COLUMN_DATA + ", " + RINGTONE_COLUMN_SHOT_TONE_TYPE + ", " +
    RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + ", " + RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + ", " +
    RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + ", " + RINGTONE_COLUMN_RING_TONE_TYPE + ", " +
    RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + ", " + RINGTONE_COLUMN_ALARM_TONE_TYPE + ", " +
    RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE;

static const string RINGTONE_SETTINGS_WHERECLAUSE_SHOT = RINGTONE_COLUMN_SHOT_TONE_TYPE + " <> " +
    to_string(SHOT_TONE_TYPE_NOT);

static const string RINGTONE_SETTINGS_WHERECLAUSE_ALARM = RINGTONE_COLUMN_ALARM_TONE_TYPE + " <> " +
    to_string(ALARM_TONE_TYPE_NOT);

static const string RINGTONE_SETTINGS_WHERECLAUSE_NOTIFICATION = RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " <> " +
    to_string(NOTIFICATION_TONE_TYPE_NOT);

static const string RINGTONE_SETTINGS_WHERECLAUSE_RINGTONE = RINGTONE_COLUMN_RING_TONE_TYPE + " <> " +
    to_string(RING_TONE_TYPE_NOT);

static const string QUERY_SHOTTONE_SETTINGS_SQL = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS + " FROM "
    + RINGTONE_TABLE + " WHERE " + RINGTONE_SETTINGS_WHERECLAUSE_SHOT;

static const string QUERY_ALARMTONE_SETTINGS_SQL = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS + " FROM "
    + RINGTONE_TABLE + " WHERE " + RINGTONE_SETTINGS_WHERECLAUSE_ALARM;

static const string QUERY_NOTIFICATIONTONE_SETTINGS_SQL = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS +
    " FROM " + RINGTONE_TABLE + " WHERE " + RINGTONE_SETTINGS_WHERECLAUSE_NOTIFICATION;

static const string QUERY_RINGTONE_SETTINGS_SQL = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS + " FROM " +
    RINGTONE_TABLE + " WHERE " + RINGTONE_SETTINGS_WHERECLAUSE_RINGTONE;

static const string QUERY_SETTINGS_BY_PATH = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS + " FROM "
    + RINGTONE_TABLE + " WHERE " + RINGTONE_COLUMN_DATA + " = ";

RingtoneSettingManager::RingtoneSettingManager(std::shared_ptr<NativeRdb::RdbStore> rdb) : ringtoneRdb_(rdb)
{
}

int32_t RingtoneSettingManager::CommitSettingCompare(int32_t settingType, int32_t toneType, int32_t sourceType)
{
    // sourceType只接受PRESET(1)和CUSTOMISED(2)，INVALID(-1)等值会被拒绝
    if ((sourceType != SOURCE_TYPE_PRESET) && (sourceType != SOURCE_TYPE_CUSTOMISED)) {
        return E_INVALID_ARGUMENTS;
    }
    if ((settingType < TONE_SETTING_TYPE_ALARM) || (settingType >= TONE_SETTING_TYPE_MAX)) {
        return E_INVALID_ARGUMENTS;
    }
    // 通知和闹钟的toneType是固定值（非卡位图），必须匹配
    if (((settingType == TONE_SETTING_TYPE_NOTIFICATION) && (toneType != NOTIFICATION_TONE_TYPE)) ||
        ((settingType == TONE_SETTING_TYPE_ALARM) && (toneType != ALARM_TONE_TYPE))) {
        return E_INVALID_ARGUMENTS;
    }
    // 短信和来电的toneType是卡位图，必须至少有一个卡位被设置
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        if (toneType <= SHOT_TONE_TYPE_NOT || !HasAnyCardSet(toneType)) {
            return E_INVALID_ARGUMENTS;
        }
    }
    if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        if (toneType <= RING_TONE_TYPE_NOT || !HasAnyCardSet(toneType)) {
            return E_INVALID_ARGUMENTS;
        }
    }
    return E_OK;
}

int32_t RingtoneSettingManager::TryMergeExistingSetting(const string &tonePath, int32_t settingType,
    int32_t toneType, int32_t sourceType)
{
    auto range = settings_.equal_range(tonePath);
    for (auto it = range.first; it != range.second; ++it) {
        // 完全重复：settingType + toneType + sourceType 三者均相同
        if ((settingType == it->second.settingType) && (toneType == it->second.toneType) &&
            (sourceType == it->second.sourceType)) {
            RINGTONE_INFO_LOG("warning: this setting is existing, tone file:%{public}s", tonePath.c_str());
            return E_FAIL;
        } else if ((settingType == it->second.settingType) && (sourceType == it->second.sourceType)) {
            // 同设置类型同来源，但卡位不同：合并卡位图
            if (settingType == TONE_SETTING_TYPE_SHOT || settingType == TONE_SETTING_TYPE_RINGTONE) {
                return MergeCardToneType(it->second, toneType);
            }
        }
    }
    return E_ERR;
}

int32_t RingtoneSettingManager::MergeCardToneType(SettingItem &item, int32_t toneType)
{
    // 检查新卡位是否已存在于已有项中
    if (static_cast<uint32_t>(item.toneType) & static_cast<uint32_t>(toneType)) {
        RINGTONE_INFO_LOG("card already merged, toneType=%{public}d", item.toneType);
        return E_OK;
    }
    // 位或运算合并卡位图，如 0b0001 | 0b0010 = 0b0011
    item.toneType = SetCardMask(item.toneType, toneType);
    RINGTONE_INFO_LOG("merged card mask=%{public}d", item.toneType);
    return E_OK;
}

int32_t RingtoneSettingManager::CommitSetting(int32_t toneId, string &tonePath, int32_t settingType, int32_t toneType,
    int32_t sourceType)
{
    RINGTONE_INFO_LOG("CommitSetting toneId=%{public}d, tonePath=%{public}s, settingType=%{public}d,"
        "toneType=%{public}d, sourceType=%{public}d", toneId, tonePath.c_str(), settingType, toneType, sourceType);
    // 步骤1: 参数合法性校验
    auto ret = CommitSettingCompare(settingType, toneType, sourceType);
    if (ret != E_OK) {
        return ret;
    }
    // 步骤2: 尝试合并到已有的同路径缓存项（如合并卡位图）
    ret = TryMergeExistingSetting(tonePath, settingType, toneType, sourceType);
    if (ret != E_ERR) {
        return ret;
    }
    // 步骤3: 无可合并项，新增缓存项
    SettingItem item = {toneId, settingType, toneType, sourceType};
    settings_.emplace(tonePath, item);
    return E_OK;
}
void RingtoneSettingManager::TravelSettings(function<int32_t (string &, SettingItem &)> func)
{
    for (auto it = settings_.cbegin(); it != settings_.cend(); ++it) {
        string first = it->first;
        SettingItem item = it->second;
        func(first, item);
    }
}

void RingtoneSettingManager::SetForceFlush(bool forceFlush)
{
    RINGTONE_INFO_LOG("flag = %{public}d", forceFlush ? 1 : 0);
    forceFlush_ = forceFlush;
}

/*
 * FlushSettings: 将内存中缓存的设置项(settings_)持久化到数据库
 *
 * 单遍遍历策略: 对每个SettingItem逐卡位处理
 * - SHOT/RINGTONE: 遍历卡1~卡4, 逐卡位判断是否需要设置
 * - NOTIFICATION/ALARM: 无卡位概念, 直接设置tone_type和source_type
 *
 * 逐卡位处理逻辑(SHOT/RINGTONE):
 * 1. 源机toneType未设此卡 → 跳过
 * 2. 本机此卡已设自定义(source_type=2)且非forceFlush → 跳过(保护用户已设设置)
 * 3. 本机此卡未设自定义 → 通过tonePath找到local记录设置卡位bit, 同时清理预置记录中该卡位bit
 *
 * 最后清空settings_缓存
 */
void RingtoneSettingManager::FlushSettings()
{
    TravelSettings([this](string &tonePath, SettingItem &item) -> int32_t {
        if (item.settingType == TONE_SETTING_TYPE_SHOT ||
            item.settingType == TONE_SETTING_TYPE_RINGTONE) {
            ApplyCardSettings(tonePath, item);
        } else {
            ApplyNonCardSetting(tonePath, item.settingType, item.toneType, item.sourceType);
        }
        return E_OK;
    });
    settings_.clear();
}

int32_t RingtoneSettingManager::PopulateMetadata(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &metaData)
{
    std::vector<std::string> columnNames;
    int32_t err = resultSet->GetAllColumnNames(columnNames);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get all column names");
        return E_RDB;
    }

    for (const auto &col : columnNames) {
        ExtractMetaFromColumn(resultSet, metaData, col);
    }

    return E_OK;
}

void RingtoneSettingManager::ExtractMetaFromColumn(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &metadata, const std::string &col)
{
    RingtoneResultSetDataType dataType = RingtoneResultSetDataType::DATA_TYPE_NULL;
    RingtoneMetadata::RingtoneMetadataFnPtr requestFunc = nullptr;
    auto itr = metadata->memberFuncMap_.find(col);
    if (itr != metadata->memberFuncMap_.end()) {
        dataType = itr->second.first;
        requestFunc = itr->second.second;
    } else {
        RINGTONE_ERR_LOG("column name invalid %{public}s", col.c_str());
        return;
    }

    std::variant<int32_t, std::string, int64_t, double> data =
        ResultSetUtils::GetValFromColumn<const shared_ptr<NativeRdb::ResultSet>>(col, resultSet, dataType);

    // Use the function pointer from map and pass data to fn ptr
    if (requestFunc != nullptr) {
        (metadata.get()->*requestFunc)(data);
    }
}

int32_t RingtoneSettingManager::GetMetaDataFromResultSet(shared_ptr<NativeRdb::ResultSet> resultSet,
    vector<shared_ptr<RingtoneMetadata>> &metaDatas)
{
    if (resultSet == nullptr) {
        RINGTONE_INFO_LOG("invalid argument");
        return E_INVALID_ARGUMENTS;
    }
    auto ret = resultSet->GoToFirstRow();
    while (ret == NativeRdb::E_OK) {
        auto metaData = make_unique<RingtoneMetadata>();
        if (PopulateMetadata(resultSet, metaData) != E_OK) {
            resultSet->Close();
            RINGTONE_INFO_LOG("read resultset error");
            return E_DB_FAIL;
        }
        metaDatas.push_back(std::move(metaData));
        ret = resultSet->GoToNextRow();
    };

    return E_OK;
}

/*
 * IsCardAlreadyCustomised: 检查本机指定类型+卡位是否已有自定义(source_type=2)的记录
 *
 * 通过GetCombinationsForCard生成包含该卡位的所有tone_type组合值,
 * 查询DB中是否有source_type=2且tone_type匹配的记录
 *
 * 参数:
 *   settingType - SHOT 或 RINGTONE
 *   cardMask    - 卡位掩码(SIM_CARD_1_MASK等)
 * 返回: true表示已有自定义记录
 */
bool RingtoneSettingManager::IsCardAlreadyCustomised(int32_t settingType, int32_t cardMask)
{
    string typeColumn;
    string sourceColumn;
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        typeColumn = RINGTONE_COLUMN_SHOT_TONE_TYPE;
        sourceColumn = RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE;
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        typeColumn = RINGTONE_COLUMN_RING_TONE_TYPE;
        sourceColumn = RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE;
    } else {
        return false;
    }

    auto combinations = GetCombinationsForCard(cardMask);
    if (combinations.empty()) {
        return false;
    }
    string inClause = "(";
    for (size_t i = 0; i < combinations.size(); i++) {
        if (i > 0) {
            inClause += ", ";
        }
        inClause += to_string(combinations[i]);
    }
    inClause += ")";

    string querySql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " + sourceColumn +
        " = " + to_string(SOURCE_TYPE_CUSTOMISED) + " AND " + typeColumn + " IN " + inClause + ";";
    auto resultSet = ringtoneRdb_->QuerySql(querySql);
    if (resultSet == nullptr) {
        return false;
    }
    int32_t count = 0;
    if (resultSet->GoToFirstRow() == NativeRdb::E_OK) {
        resultSet->GetInt(0, count);
    }
    resultSet->Close();
    return count > 0;
}

/*
 * ApplyCardSetting: 通过tonePath找到local记录, 设置对应卡位bit + source_type
 *
 * SQL: UPDATE ToneFiles SET tone_type = tone_type | cardMask, source_type = sourceType
 *      WHERE data = "tonePath"
 * 若当前tone_type为0(未设置), 0 | cardMask = cardMask, 效果等同于直接赋值
 */
int32_t RingtoneSettingManager::ApplyCardSetting(const std::string &tonePath, int32_t settingType,
    int32_t cardMask, int32_t sourceType)
{
    string typeColumn;
    string sourceColumn;
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        typeColumn = RINGTONE_COLUMN_SHOT_TONE_TYPE;
        sourceColumn = RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE;
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        typeColumn = RINGTONE_COLUMN_RING_TONE_TYPE;
        sourceColumn = RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE;
    } else {
        return E_INVALID_ARGUMENTS;
    }

    // 通过文件路径找到记录, OR合并卡位bit
    string updateSql = "UPDATE " + RINGTONE_TABLE + " SET " +
        typeColumn + " = " + typeColumn + " | " + to_string(cardMask) + ", " +
        sourceColumn + " = " + to_string(sourceType) +
        " WHERE " + RINGTONE_COLUMN_DATA + " = \"" + tonePath + "\"";

    int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
    if (rdbRet < 0) {
        RINGTONE_ERR_LOG("ApplyCardSetting failed, tonePath=%{public}s, cardMask=%{public}d",
            tonePath.c_str(), cardMask);
        return E_DB_FAIL;
    }
    return E_OK;
}

/*
 * ClearPresetCardBit: 清理local DB中预置记录(source_type=1)的指定卡位bit
 *
 * 查询所有预置记录中tone_type包含该卡位的记录, 逐条清除该bit
 * 若清除后tone_type变为0, 同时重置source_type为默认值
 */
int32_t RingtoneSettingManager::ClearPresetCardBit(int32_t settingType, int32_t cardMask)
{
    string typeColumn;
    string sourceColumn;
    int32_t defaultSourceType;
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        typeColumn = RINGTONE_COLUMN_SHOT_TONE_TYPE;
        sourceColumn = RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE;
        defaultSourceType = SHOT_TONE_SOURCE_TYPE_DEFAULT;
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        typeColumn = RINGTONE_COLUMN_RING_TONE_TYPE;
        sourceColumn = RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE;
        defaultSourceType = RING_TONE_SOURCE_TYPE_DEFAULT;
    } else {
        return E_INVALID_ARGUMENTS;
    }

    // 查询预置记录中包含该卡位的记录
    string querySql = "SELECT " + RINGTONE_COLUMN_TONE_ID + ", " + typeColumn + " FROM " + RINGTONE_TABLE +
        " WHERE " + sourceColumn + " = " + to_string(SOURCE_TYPE_PRESET) +
        " AND (" + typeColumn + " & " + to_string(cardMask) + ") != 0;";
    auto resultSet = ringtoneRdb_->QuerySql(querySql);
    if (resultSet == nullptr) {
        return E_OK;
    }
    vector<pair<int32_t, int32_t>> records;
    auto ret = resultSet->GoToFirstRow();
    while (ret == NativeRdb::E_OK) {
        int32_t toneId = 0;
        int32_t toneType = 0;
        resultSet->GetInt(0, toneId);
        resultSet->GetInt(1, toneType);
        records.emplace_back(toneId, toneType);
        ret = resultSet->GoToNextRow();
    }
    resultSet->Close();

    // 逐条清除卡位bit
    for (const auto &[toneId, toneType] : records) {
        int32_t newType = ClearCardMask(toneType, cardMask);
        string updateSql = "UPDATE " + RINGTONE_TABLE + " SET " + typeColumn + " = " + to_string(newType);
        if (newType == 0) {
            // 清除后无卡位设置, 同时重置source_type
            updateSql += ", " + sourceColumn + " = " + to_string(defaultSourceType);
        }
        updateSql += " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);
        int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
        if (rdbRet < 0) {
            RINGTONE_ERR_LOG("ClearPresetCardBit failed, toneId=%{public}d, cardMask=%{public}d",
                toneId, cardMask);
        }
    }
    return E_OK;
}

/*
 * ApplyNonCardSetting: 处理NOTIFICATION/ALARM设置(无卡位概念)
 *
 * 1. 查询本机该类型是否已有自定义(source_type=2)记录
 * 2. 若已有且非forceFlush → 跳过(保护用户已设设置)
 * 3. 通过tonePath找到local记录, 设置tone_type + source_type
 */
int32_t RingtoneSettingManager::ApplyNonCardSetting(const std::string &tonePath, int32_t settingType,
    int32_t toneType, int32_t sourceType)
{
    string typeColumn;
    string sourceColumn;
    if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        typeColumn = RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE;
        sourceColumn = RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE;
    } else if (settingType == TONE_SETTING_TYPE_ALARM) {
        typeColumn = RINGTONE_COLUMN_ALARM_TONE_TYPE;
        sourceColumn = RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE;
    } else {
        return E_INVALID_ARGUMENTS;
    }

    // 检查本机是否已有自定义记录
    if (!forceFlush_) {
        string checkSql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " + sourceColumn +
            " = " + to_string(SOURCE_TYPE_CUSTOMISED) + " AND " + typeColumn + " = " + to_string(toneType) + ";";
        auto resultSet = ringtoneRdb_->QuerySql(checkSql);
        if (resultSet != nullptr) {
            int32_t count = 0;
            if (resultSet->GoToFirstRow() == NativeRdb::E_OK) {
                resultSet->GetInt(0, count);
            }
            resultSet->Close();
            if (count > 0) {
                return E_OK; // 已有自定义记录, 跳过
            }
        }
    }

    // 通过tonePath找到local记录, 设置tone_type + source_type
    string updateSql = "UPDATE " + RINGTONE_TABLE + " SET " +
        typeColumn + " = " + to_string(toneType) + ", " +
        sourceColumn + " = " + to_string(sourceType) +
        " WHERE " + RINGTONE_COLUMN_DATA + " = \"" + tonePath + "\"";
    int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
    if (rdbRet < 0) {
        RINGTONE_ERR_LOG("ApplyNonCardSetting failed, tonePath=%{public}s", tonePath.c_str());
        return E_DB_FAIL;
    }
    return E_OK;
}

/*
 * GetTargetSourceType: 通过tonePath查询local DB中该记录的source_type
 *
 * 返回 SOURCE_TYPE_PRESET(1) / SOURCE_TYPE_CUSTOMISED(2), 查不到返回 SOURCE_TYPE_INVALID(-1)
 */
int32_t RingtoneSettingManager::GetTargetSourceType(const std::string &tonePath)
{
    if (ringtoneRdb_ == nullptr) {
        RINGTONE_ERR_LOG("ringtone rdb_ is nullptr");
        return SOURCE_TYPE_INVALID;
    }
    string querySql = "SELECT " + RINGTONE_COLUMN_SOURCE_TYPE + " FROM " + RINGTONE_TABLE +
        " WHERE " + RINGTONE_COLUMN_DATA + " = \"" + tonePath + "\"";
    auto resultSet = ringtoneRdb_->QuerySql(querySql);
    if (resultSet == nullptr || resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        RINGTONE_INFO_LOG("tone not found, tonePath=%{public}s", tonePath.c_str());
        if (resultSet != nullptr) {
            resultSet->Close();
        }
        return SOURCE_TYPE_INVALID;
    }
    int32_t sourceType = SOURCE_TYPE_INVALID;
    resultSet->GetInt(0, sourceType);
    resultSet->Close();
    RINGTONE_INFO_LOG("GetTargetSourceType tonePath=%{public}s, sourceType=%{public}d",
        tonePath.c_str(), sourceType);
    return sourceType;
}

/*
 * ApplyCardSettings: 逐卡位处理SHOT/RINGTONE设置
 *
 * 遍历卡1~卡4, 对每个卡位:
 * 1. 源机toneType未设此卡 → 跳过
 * 2. 本机此卡已设自定义(source_type=2)且非forceFlush → 跳过
 * 3. 本机此卡未设自定义 → 设置卡位bit + 清理预置记录中该卡位bit
 */
void RingtoneSettingManager::ApplyCardSettings(const std::string &tonePath, const SettingItem &item)
{
    struct CardInfo {
        int32_t mask;
        bool (*isSet)(int32_t);
    };
    static const CardInfo cards[] = {
        {SIM_CARD_1_MASK,  IsSimCard1Set},
        {SIM_CARD_2_MASK,  IsSimCard2Set},
        {ESIM_CARD_1_MASK, IsESimCard1Set},
        {ESIM_CARD_2_MASK, IsESimCard2Set},
    };

    for (const auto &card : cards) {
        // 1. 源机未设此卡 → 跳过
        if (!card.isSet(item.toneType)) {
            continue;
        }
        // 2. 本机此卡已设自定义且非forceFlush → 跳过
        if (!forceFlush_ && IsCardAlreadyCustomised(item.settingType, card.mask)) {
            continue;
        }
        // 3. 设置卡位bit
        ApplyCardSetting(tonePath, item.settingType, card.mask, item.sourceType);
        int32_t targetSourceType = GetTargetSourceType(tonePath);
        // 4. 目标机存在自定义，则需要清理预置记录中该卡位bit，且默认值场景也不能清理
        if (targetSourceType == static_cast<int32_t>(SOURCE_TYPE_CUSTOMISED)) {
            ClearPresetCardBit(item.settingType, card.mask);
        }
    }
}

/*
 * TravelQueryResultSet: 执行查询SQL, 将结果集解析为RingtoneMetadata列表,
 * 遍历每条记录调用回调函数
 *
 * 回调返回true时提前终止遍历
 * 供RingtoneDefaultSetting::GetDefaultTonePathByDisplayName等外部调用方使用
 */
int32_t RingtoneSettingManager::TravelQueryResultSet(string querySql,
    function<bool (shared_ptr<RingtoneMetadata> &)> func)
{
    auto resultSet = ringtoneRdb_->QuerySql(querySql);
    if (resultSet == nullptr) {
        RINGTONE_INFO_LOG("result is null");
        return E_OK;
    }
    vector<shared_ptr<RingtoneMetadata>> metaDatas = {};
    int32_t ret = GetMetaDataFromResultSet(resultSet, metaDatas);
    if (ret != E_OK) {
        RINGTONE_INFO_LOG("read resultSet error");
        resultSet->Close();
        return E_DB_FAIL;
    }
    for (auto meta : metaDatas) {
        if (func(meta) == true) {
            break;
        }
    }

    resultSet->Close();
    return E_OK;
}

int32_t RingtoneSettingManager::Update(int &changedRows, const NativeRdb::ValuesBucket &values,
    const NativeRdb::AbsRdbPredicates &predicates)
{
    if (ringtoneRdb_ == nullptr) {
        RINGTONE_ERR_LOG("ringtone rdb_ is nullptr");
        return E_DB_FAIL;
    }
    return ringtoneRdb_->Update(changedRows, values, predicates);
}

} // namespace Media
} // namespace OHOS
