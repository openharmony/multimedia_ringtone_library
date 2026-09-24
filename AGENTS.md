# 铃音库框架指引

## 项目定位

本仓库对应 OpenHarmony `foundation/multimedia/ringtone_library`。优先按这些目录定位问题：

- `services/ringtone_data_extension/`：DataShare 扩展主入口，`RingtoneDataManager` 增删改查、`RingtoneDataCommand` 命令分发、`RingtoneDatashareStubImpl` stub 实现、`RingtoneRdbStore` 数据库存取。
- `services/ringtone_scanner/`：预制铃音扫描器，`RingtoneScannerObj` 扫描入口、`RingtoneScanExecutor` 执行体、`RingtoneMetadataExtractor` 元数据提取、`RingtoneScannerDb` 扫描库写入。
- `services/ringtone_helper/`：Asset 实现，`RingtoneAsset`、`VibrateAsset`、`SimcardSettingAsset`、`Haptic2ToneAsset`、`RingtoneFetchResult`。
- `services/ringtone_restore/`：备份/恢复，`RingtoneRestore` 入口、`RingtoneRestoreFactory` 场景注册、`RingtoneRdbTransaction` 事务、`RingtoneDualfwkRestore` 双框架恢复、`customised_tone_processor` 自定义铃音处理。
- `services/ringtone_dfx/`：埋点/故障上报，`DfxManager` 计数、`DfxReporter` 上报、`DfxWorker` 工作线程。
- `services/ringtone_setting/`：系统铃音设置，`RingtoneSettingManager`、`RingtoneMetadata`、`VibrateMetadata`、`RingMockHapticAudioMetadata`。
- `services/utils/`：通用工具，`RingtonePrivacyManager` 权限、`PermissionUtils`、`RingtoneFileUtils`、`RingtoneMimetypeUtils`、`RingtoneXcollie` 超时看门狗、`RingtoneTracer` 性能打点。
- `interfaces/inner_api/native/`：对外暴露的 inner_kits 头文件（见下文"知识路由"）。
- `frameworks/ringtone_extension_hap/`：Stage 模式 HAP，ets 薄层 stub（`DataShareExtAbility`、`RingtoneBackupExtAbility`）委托 native 库。
- `test/unittest/`：13 个单元测试目标，`get_self_permissions`、`ringtone_unittest_utils` 为共享测试工具。

## 典型工作流

1. 先判断改动场景，读取下文知识路由对应的关键文件；一个任务跨多个场景时，按影响面同时读取多个入口。
2. 定位公开接口和内部实现边界：先看 `interfaces/inner_api/native/`（`ringtone_type.h`、`ringtone_db_const.h`、`ringtone_asset.h` 等），再看 `services/ringtone_data_extension/` 与 `services/ringtone_helper/` 的实现。
3. 改动涉及 DataShare URI、数据库表结构、权限校验、静默访问（proxyData）、SIM/eSIM 位图、备份兼容性或 NAPI 模块时，先确认生命周期、安全边界、错误码映射和数据库升级幂等性。
4. 小步修改，就近复用项目已有宏、错误码、日志和测试资源。
5. 按下文验证矩阵和知识路由中的验证重点运行最相关的测试目标。
6. 最终回复要写明读取过的关键文件、完成的验证、未覆盖的 XTS 缺口，以及是否已提交或 push。
7. 提交和 push 前按下文"提交和推送"要求完成检查。

## 依赖和接口边界

本仓对外依赖在 `bundle.json` 中声明，常见跨子系统边界包括：

- 数据存储与共享：`data_share`、`relational_store`、`kv_store`、`preferences`。
- 运行时与能力：`ability_runtime`、`ability_base`、`bundle_framework`、`samgr`、`ipc`、`init`、`os_account`、`common_event_service`、`safwk`。
- 日志/可观测：`hilog`、`hicollie`、`hisysevent`、`hitrace`。
- 多媒体：`media_foundation`、`media_library`、`player_framework`、`image_framework`。
- 安全：`access_token`（`libaccesstoken_sdk`、`libprivacy_sdk`）。
- 电话（可选，`ringtone_core_service_enable`）：`core_service`。
- 其余：`app_file_service`、`config_policy`、`c_utils`、`libxml2`、`libuv`、`napi`、`node`、`ets_frontend`。

改动触达上述依赖的接口、权限语义、URI、buffer 语义或错误码时，不要仅在本仓内处理；需要检查依赖方公开头文件、运行时能力和调用方假设（如 `SystemSoundManager`、`RingtoneKit` 的静默访问路径），并在提交说明中写明跨仓影响和验证方式。

## 验证

按改动范围选择最相关的测试目标，具体优先参考知识路由。涉及对外接口或数据库行为时，还需验证对应 XTS 用例。

任务级验证参考：

| 改动类型 | 近端验证 | 额外要求 |
|----------|----------|----------|
| 文档、知识路由、注释 | 检查链接、路径、术语和代码锚点是否存在 | 不改行为时通常不需要额外验证 |
| C++ 内部实现 | 对应模块最相关的单元测试 | 关注错误码、日志、资源释放和异常路径 |
| DataShare URI、proxyData、Asset 接口 | `ringtone_data_extension_unittest`、`ringtone_helper_unittest` | 检查权限校验、多用户越权、静默访问不拉起进程 |
| 数据库表结构/升级 | `ringtone_data_extension_unittest`（`ringtone_rdbstore_test`） | 必须验证升级幂等性、旧数据兼容、列读取逻辑 |
| 扫描器、元数据提取 | `ringtone_scanner_unittest`、`ringtone_scanner_test` | 补跑相关集成测试，覆盖预制目录扫描、增量扫描 |
| 备份/恢复、双框架 | `ringtone_restore_unittest`、`ringtone_dualfwk_restore_unittest` | 验证场景兼容（next2next/hmos2next）、目录映射 |
| 权限、隐私、安全修复 | `ringtone_utils_unittest`（`ringtone_privacy_manager_test`） | 检查越权、多用户、token 校验 |
| NAPI 模块 `@ohos.multimedia.ringtonerestore` | `ringtone_restore_unittest` | 检查 NAPI 接口与 native 行为一致 |

XTS 用例不在本仓完整维护。涉及公开接口、URI、权限、数据库行为或兼容性时，必须查 OpenHarmony XTS 仓、CI 配置或团队用例映射；查不到时，在最终回复中明确写"XTS 目标未确认"，并列出已跑的本仓单元测试和需要人工补充确认的接口场景。

## 编译

构建从 OpenHarmony 源码根目录执行，本组件置于 `foundation/multimedia/ringtone_library`，组件名 `ringtone_library`、subsystem `multimedia`。本仓不含 `build_system.sh`，沿用源码根目录的构建脚本。

### 编译命令

参考写法（构建入口与参数以团队实际环境为准）：

```bash
NextBuild --cache ./build_system.sh --abi-type generic_generic_arm_64only --device-type general_all_phone_standard --ccache --build-variant root --gn-flags=--export-compile-commands -j50 --build-target ringtone_data_extension ringtonerestore ringtone_data_helper ringtone_extension_hap
```

- 只编某模块时，`--build-target` 只留对应目标名（见下表）。
- 编全部单元测试时，追加 `--build-target test`（`test/unittest/BUILD.gn` 的 `:test` group 汇总 13 个单元测试目标）。
- 组件构建开关在 `ringtone_library.gni` 的 `declare_args()` 中：`ringtone_link_opt`、`ringtone_config_policy_enable`、`ringtone_media_library_enable`、`ringtone_core_service_enable`；按环境用 `--gn-args <开关>=true/false` 调整。不要照搬 camera 的 `use_cfi`/`fwk_no_hidden` 等 gn-args，铃音库的 sanitize 与 visibility 在各 `BUILD.gn` 内定义。

### 验证命令

| 场景 | 命令 | 说明 |
|------|------|------|
| 全量编译 | 见上方编译命令 | 编译全部业务目标 |
| 增量编译 | 去掉 `--cache` 参数 | 仅编译变更文件 |
| 清理后编译 | 先清理 build 目录再执行编译命令 | 排查增量缓存导致的脏构建 |
| 单模块编译 | `--build-target <目标名>` | 如 `ringtone_data_extension`、`ringtone_utils`、`ringtone_setting` |
| 单元测试 | 追加 `--build-target test` 后执行各 `ringtone_*_unittest` | 目标见「验证」矩阵与知识路由「验证重点」列 |

### 构建目标与产物

| GN 目标 | 类型 | 产物 / 说明 |
|---------|------|-------------|
| `services:ringtone_data_extension` | 共享库 | `libringtone_data_extension.z.so`（`install_enable=true`，DataShare 扩展主入口） |
| `services:ringtonerestore` | 共享库 | `libringtonerestore.z.so`（`install_enable=false`，随 HAP 打包；含 NAPI 模块 `@ohos.multimedia.ringtonerestore`） |
| `services/ringtone_helper:ringtone_data_helper` | 共享库 | `libringtone_data_helper.z.so`（inner_kits 之二，Asset 实现） |
| `services:ringtone_utils` | 共享库 | `libringtone_utils.z.so`（权限、错误码、文件/MIME、xCollie/Tracer） |
| `services:ringtone_setting` | 共享库 | `libringtone_setting.z.so`（系统铃音设置、系统参数持久化） |
| `frameworks/ringtone_extension_hap:ringtone_extension_hap` | HAP | `Ringtone_Library_Ext.hap`，安装到 `app/com.ohos.ringtonelibrary.RingtoneLibraryData`，签名 `signature/ringtonelibrary.p7b` |
| `test/unittest:test` | 测试 group | 汇总 13 个单元测试目标（见 `test/unittest/BUILD.gn`） |

产物名以实际构建输出为准。`services/BUILD.gn` 另有 `ringtone_*_param.para`/`ringtone_param.para.dac` 等 `ohos_prebuilt_etc`，安装到 `etc/param`。

## 提交和推送

以下为 Agent 提交约定，可能与历史人工提交风格不同；人工提交按团队现有规范执行。提交建议使用 `git commit -s` 自动生成 `Signed-off-by`，其姓名和邮箱来自 `git config user.name` 与 `git config user.email`，格式类似 `Signed-off-by: zhangsan <zhangsan@example.com>`。同时在 commit message 末尾额外空一行写入 `Co-Authored-By: Agent`：

```text
<type>(<scope>): <summary>

<body，可选>

Signed-off-by: <name> <email>

Co-Authored-By: Agent
```

没有明确项目要求时，`type` 优先使用 `fix`、`feat`、`refactor`、`test`、`docs`、`build`，`scope` 使用模块名或目录名。若关联 issue、缺陷单或需求单，在 body 中写清编号和影响范围。

### Issue、PR 与门禁闭环

用户要求完成推送、Issue/PR 和门禁时，按以下流程推进；只要求某一步时按授权范围执行。下文 `<仓库>` 指上游 `owner/repo`，占位符须按实际替换。

1. **准备**：用 `git status --short`、`git remote -v`、`git branch --show-current` 核对工作区、fork、上游和分支；检查 `oh-gc --version`、`oh-gc auth status`。
2. **Issue**：用 `oh-gc issue list --search "<关键词>" --state all --repo <仓库>` 查重；需新建时执行 `oh-gc issue create --repo <仓库> --title "<标题>" --body "<说明>" --json`。说明包含问题、原因、修复范围和验证缺口；记录编号和链接，用于提交说明及 PR 关联。
3. **提交推送**：执行 `git diff --check`，用 `git add -- <本次文件>` 精确暂存、`git diff --cached` 复核，再执行 `git commit -s -F <提交说明文件>` 和 `git push -u <fork-remote> HEAD:refs/heads/<分支>`。按上文保留两个 trailer，用 `git log -1 --format=full` 核对 SHA 和签名。CRLF 文件用 `git -c core.whitespace=cr-at-eol diff --check` 检查。
4. **PR 创建与关联**：
   - 模板：`oh-gc file raw .gitcode/PULL_REQUEST_TEMPLATE.md <目标分支> --repo <仓库>`；存在时按模板填写，确认不存在时自行组织说明。
   - 创建：`oh-gc pr create --repo <仓库> --head <fork-owner>:<分支> --base <目标分支> --title "<标题>" --body "<说明>" --json`。
   - 关联：`oh-gc pr link <PR编号> <Issue编号> --repo <仓库> --json`。
   - 核对：分别执行 `oh-gc pr view`、`oh-gc pr files`、`oh-gc pr linked-issues`，均追加 `<PR编号> --repo <仓库> --json`，确认源仓库、分支、SHA、文件范围和关联结果。
5. **触发门禁**：确认 PR 已收到最新 SHA，再执行 `oh-gc pr comment <PR编号> --repo <仓库> --body 'start build'`；已有本轮构建时直接跟踪。用 `oh-gc pr comments <PR编号> --repo <仓库> --latest --limit 10 --full-body --json` 获取报告，核对报告对应的 SHA。
6. **修复重跑**：按具体 CodeCheck、编译或测试错误修复并验证，重复第 3 步向同一分支追加签名提交；用 `oh-gc pr update <PR编号> --repo <仓库> --body "<更新后的说明>"` 更新记录，再按第 5 步重跑，直到最新提交门禁通过。无法自行解决的阻塞须说明原因和待处理事项。

多行正文在 PowerShell 中用 `Get-Content -Raw` 读取后传给 `--body`；`oh-gc pr comments` 提供报告入口，具体错误需读取对应 CI 报告。

同一任务、同一影响范围内，沿用用户已确认的检视选择；新增影响范围按下文确认。不得通过删测试、屏蔽检查或 `oh-gc pr review/test` 手工标记代替 CI，通过后不自动合并 PR。

最终提供 Issue/PR 链接、最新 SHA、门禁结果和验证缺口。准确区分实际通过、`IGNORE`、`NA` 和未执行；编译成功或 `Upgrade only` 冒烟测试通过不代表新增用例已执行。

## 知识路由

改动前按场景定位：知识路由统一以目录级表述，「先读」为入口/契约目录，「代码锚点」为实现目录，「验证重点」为最相关的测试目标与关注项：

| 场景 | 先读 | 代码锚点 | 验证重点 |
|------|------|----------|----------|
| DataShare 增删改查、URI 分发、stub 实现 | `interfaces/inner_api/native/`（DB 列常量、proxy URI 常量） | `services/ringtone_data_extension/` | `ringtone_data_extension_unittest`，关注权限、多用户、静默访问 |
| 静默访问 proxyData（不拉起进程） | `interfaces/inner_api/native/`（proxy URI 常量） | `frameworks/ringtone_extension_hap/`（HAP proxyDatas 配置节点）、`services/utils/` | 验证 `?Proxy=true` URI 路径、RDB metadata、`ACCESS_CUSTOM_RINGTONE` 权限 |
| 数据库表结构、版本升级（当前 v13） | `interfaces/inner_api/native/`（DB 列常量、版本号） | `services/ringtone_data_extension/`、`services/ringtone_scanner/` | `ringtone_rdbstore_test`，关注升级幂等、`ALTER TABLE`、旧数据兼容 |
| 扫描预制铃音、元数据提取、增量扫描 | `services/ringtone_scanner/include/`（扫描入口） | `services/ringtone_scanner/` | `ringtone_scanner_unittest`、`ringtone_scanner_test`，覆盖预制目录、`scanner_flag` 标志位 |
| Asset 对象、查询结果集 | `interfaces/inner_api/native/`（Asset、FetchResult 接口） | `services/ringtone_helper/` | `ringtone_helper_unittest`，关注 getter/setter、member map、列读取 |
| SIM/eSIM 铃音位图编码（最多 4 卡） | `interfaces/inner_api/native/`（SIM/eSIM 枚举与位图常量） | `services/ringtone_helper/` | 验证位图组合、`soundMode*100+toneType` 编码、`GetSimCardCount` 等内联函数 |
| 振动/触感、haptic 二合一 | `interfaces/inner_api/native/`（振动类型、haptic 二合一接口） | `services/ringtone_helper/` | `ringtone_helper_unittest`，关注同步/经典播放模式、标准/弱化振动 |
| 备份/恢复、双框架恢复、场景注册 | `services/ringtone_restore/include/`（恢复入口） | `services/ringtone_restore/` | `ringtone_restore_unittest`、`ringtone_dualfwk_restore_unittest`、`customised_tone_processor_unittest`，关注 backup_config 场景配置兼容、目录映射 |
| DFX 埋点上报 | `services/ringtone_dfx/include/`（DFX 常量）、仓根（hiSysEvent 字段定义） | `services/ringtone_dfx/` | `ringtone_dfx_unittest`，关注 `RINGTONELIB_DFX_MESSAGE` 字段、按类别计数 |
| 系统铃音设置、系统参数持久化 | `services/ringtone_setting/include/`（设置入口） | `services/ringtone_setting/`、`services/etc/`（系统参数文件） | `ringtone_setting_unittest`，关注 `persist.ringtone.setting.*` 参数 |
| 权限/隐私、文件、MIME、xCollie/Tracer | `services/utils/include/`（权限、错误码、日志） | `services/utils/` | `ringtone_utils_unittest`，关注 token 校验、错误码、超时看门狗 |

术语路由（按触发符号定位到目录，目录↔类名映射见「项目定位」）：

| 触发词 | 优先读取 | 重点 |
|--------|----------|------|
| `Insert`/`Delete`/`Update`/`Query`、`datashare:///ringtone`、stub、命令分发 | `services/ringtone_data_extension/` | DataShare CRUD 主链路与 stub 实现 |
| `proxyData`、`Proxy=true`、静默访问、不拉起进程 | `interfaces/inner_api/native/`（proxy URI 常量）、`frameworks/ringtone_extension_hap/`（proxyDatas 配置） | RDB metadata、权限校验、proxy URI 常量 |
| `ringtone_library.db`、版本升级、`ALTER TABLE`、列常量 | `interfaces/inner_api/native/`（DB 常量）、`services/ringtone_data_extension/`（升级实现） | 数据库版本演进（当前 v13）、升级幂等 |
| `RingtoneScannerObj`、`BootScan`、`IncrementalScann`、`scanner_flag` | `services/ringtone_scanner/` | 扫描入口、预制目录、增量扫描、标志位 |
| `RingtoneAsset`、`VibrateAsset`、`RingtoneFetchResult`、member map | `interfaces/inner_api/native/` | Asset getter/setter、结果集游标 |
| `ShotToneType`、`RingToneType`、SIM/eSIM、位图、`ALL_CARD_MASK` | `interfaces/inner_api/native/` | 位图编码、4 卡组合、`soundMode*100+toneType` |
| `VibrateType`、`VibratePlayMode`、同步/经典、标准/弱化 | `interfaces/inner_api/native/` | 振动类型、播放模式 |
| `RingtoneRestore`、`next2next`、`hmos2next`、`backup_config` | `services/ringtone_restore/`、`frameworks/ringtone_extension_hap/`（backup_config 场景配置） | 备份场景、目录映射、事务 |
| `RINGTONELIB_DFX_MESSAGE`、`RingtoneCountInfo` | 仓根（hiSysEvent 字段定义）、`services/ringtone_dfx/` | 埋点字段、按类别计数上报 |
| `persist.ringtone.setting.*`、系统参数、`ACCESS_CUSTOM_RINGTONE` | `services/etc/`（系统参数文件）、`services/ringtone_setting/`、`services/utils/`（权限） | 参数持久化、权限校验 |

## 项目约束

不要做：

- 不要在扫描、数据库查询、元数据提取或 DataShare 命令分发的热点路径中增加全量扫描、重复大内存拷贝、字符串格式化或高频 INFO 日志；扫描路径影响开机时间，查询路径需有索引。
- 不要只改 ets stub 层（`frameworks/.../ets/`）来改变对外行为；DataShare 逻辑在 native `ringtone_data_extension` 中，stub 仅委托。
- 不要只改某一处来声明静默访问能力；`proxyData` 能力需 `ringtone_proxy_uri.h`、`module.json` proxyDatas、对应 RDB metadata profile 三处一致。
- 不要只改 `ringtone_db_const.h` 的列常量而漏掉 `ringtone_rdbstore.cpp` 升级逻辑、Asset 类成员和 `ringtone_fetch_result.h` 列读取；数据库结构变更需四处同步。
- 不要把 `ringtone_rdb_transaction.h` 当作普通业务路径分析；它仅用于备份恢复的事务封装，除非任务明确涉及备份恢复。
- 不要执行破坏性 git/文件操作或大范围机械重构，除非用户明确要求。

Ask before / 必须人工确认：

以下场景不是普通"建议确认"，而是 Agent 继续修改、提交或 push 前的门禁。触发后要向用户或模块责任人说明影响面、已读关键文件、计划改动和拟验证项，得到明确答复后再继续。

- 改数据库表结构、列常量、DB 版本号或升级逻辑前，先确认升级幂等策略、旧数据兼容和 Asset 类同步范围。
- 改 DataShare URI、proxyData URI、`ringtone_proxy_uri.h` 常量或 `module.json` proxyDatas 配置前，先确认静默访问路径、RDB metadata 和 `ACCESS_CUSTOM_RINGTONE` 权限一致性。
- 改 inner_kits 头文件（`ringtone_type.h`、`ringtone_db_const.h`、`ringtone_asset.h` 等）的枚举、结构体字段或错误码前，先确认 ABI 兼容和调用方（`SystemSoundManager`、`RingtoneKit`）假设。
- 改 SIM/eSIM 位图编码、`ShotToneType`/`RingToneType` 枚举或 `SimcardSettingAsset` 前，先确认 4 卡组合、`soundMode*100+toneType` 编码和系统参数映射。
- 改权限校验（`ringtone_privacy_manager.h`、`permission_utils.h`）或涉及多用户场景前，先确认越权风险、token 校验和当前用户判断。
- 改备份/恢复场景、`backup_config.json` 路径或 `ringtone_restore_factory` 注册前，先确认 `next2next`/`hmos2next` 兼容、目录映射和事务边界。
- 改扫描路径、扫描入口或 `scanner_flag` 逻辑前，先确认开机性能影响、预制目录覆盖和增量扫描行为。
- 改上述行为时，要同步检查错误码和接口映射，包括 `services/utils/include/ringtone_errno.h`、`services/utils/include/ringtone_log.h` 以及对应 Asset/DataManager 适配代码。

C++ 改动优先复用附近的 `CHECK_*`、`RINGTONE_LOG*`、`E_OK`/`E_ERR_*` 等项目宏、错误码和日志习惯。

## 完成定义

Agent 最终回复必须包含：

- 读取过的关键文件和对应场景。
- 修改的文件、行为影响面和明确未修改的关键文件。
- 已执行的单元测试、集成测试或验证命令；未执行时说明原因。
- 本地构建结果（执行「编译」节编译命令是否通过、有无新增 ERROR）；未执行时说明原因。
- XTS 目标无法确认时，列出缺口和需要人工确认的问题。
- 若涉及提交或 push，说明 commit message 是否包含 `Signed-off-by` 和 `Co-Authored-By: Agent`。
