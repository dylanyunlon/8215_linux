# platform/device —— 嵌入式 Linux(AC8215) 后端

和 `platform/pc` 是**同一个端口面的两种实现**。`ui/` 里的代码在两个后端下
完全一样，切后端只换这个目录 + 链接的库。

```
cmake -S . -B build_device -DHCN_APPLICATION_BACKEND=device \
      -DHCN_APPLICATION_MW_INC=<sysroot>/usr/include \
      -DHCN_APPLICATION_MW_LIB=<lib_mw.a 所在目录>
```

## 分层

```
ui/                    只 include hcn_application 自持的类型契约头 + vehicle_services.h
  |  只调 vs_*
  v
vehicle_services/      hcn/vehicle_services/，唯一 include <mw/...> 的层
  v
lib_mw.a / lib_mw_cxx.a
```

**本目录（`platform/device/`）不 include 任何中间件头，也不调任何中间件函数。**
它只做一件事：

1. `ui_backend.c` —— 入口调 `vs_init()`，把 vehicle_services（进而中间件）拉起来；
2. 实现 `ui/` 需要、而中间件**没有**的那批端口（见下表）。

## 端口面：41 个符号

按 `ui/**` 的实际调用归类：

| 类别 | 数量 | 处置 |
| --- | --- | --- |
| ① 中间件已提供 | 11 | 经 **vehicle_services 门面**（`vs_*`）访问，**本目录不实现、也不直连** |
| ② 中间件没有 | 30 | 本目录实现（见下表） |
| ③ ui/ 自己实现 | — | 不涉及 |

① 的 11 个（经门面访问）：`vehicle_get_data` `vehicle_set_data` `get_usr_param`
`set_usr_param` `get_recovery_usr_param` `set_backlight_level` `get_soc_version`
`set_key_event_cb` `set/get_boot_animation_status` `set_check_self_state`

收音机（`vs_radio_*` 19 个）同属 ①：中间件是 Linux 侧的 `radio_mw`（`/dev/fmradio`），
由 `vehicle_services/vehicle_services.c` 转发；pc 侧的替身在 `platform/pc/mock_radio_mw.c`。
本目录不参与收音机，也不实现它的任何符号。

② 的 30 个按文件分布：

| 文件 | 符号 |
| --- | --- |
| `dev_storage.c` | mile_param 3 个 + os_datetime 2 个 |
| `dev_navi.c` | easy_navi 2 个 + carlink 3 个 |
| `dev_ota.c` | `get_current_update_info` |
| `dev_pending.c` | NFC 7 个 + 无线钥匙 6 个（**占位实现**） |
| `rtthread_shim.c` | `rt_tick_get` `rt_kprintf` `rt_thread_*` 共 6 个 |

## 头文件策略（重要）

`platform/include/` 在**两个后端都留在 include 路径上**，且排在
本目录之前。它是 hcn_application 的**类型契约头**：

1. **只提供 `enum` / `struct` / 宏，不再声明任何中间件函数。**
   `ui/` 拿得到 `VEH_SPEED_CURRENT` 这类「符号名」，却调不到中间件函数，
   只能走 `vs_*` —— 边界是硬的。
2. 契约头与中间件的取值一致性由 `hcn/vehicle_services/vehicle_services.c`
   顶部的 101 条 `_Static_assert` 在 device 侧 **编译期**保证（pc 后端不编译它）。
   当前契约：`veh_data_e` 88 项逐项相同，中间件只在其后追加 30 项；
   `usr_param_handle_e` 33=33、`key_event_e` 19=19、`animation_state_e` 3=3、
   `check_self_state_e` 3=3 全等。
3. 中间件函数原型在 pc 侧的替身声明集中在 `platform/pc/mock_mw_api.h`
   （pc 后端扮演「中间件替身」，需要有原型才能编译那些 mock 实现）。

## 已知陷阱

- **背光模块默认不编译**：中间件的 `module/backlight/usr_backlight.{h,c}` 都用
  `#if MW_BL_PWM_ENABLE` 包着，而中间件 `dev_config.h` 里该宏默认 **0** ——
  整个背光模块（含 `set_backlight_level` 本体）都不参与编译。门面侧用同一个宏
  判定，未开时 `vs_backlight_set()` 空转。详见 `vehicle_services/README.md`。
- **`MW_BL_PW_ENABLE` 是 hcn_application 契约头里的名字**（`platform/include/dev_config.h`
  定义 1），中间件用的是 `MW_BL_PWM_ENABLE`（默认 0），差一个字母，别混。
- `ui/common/ui_feature_config.h` 的 `UI_EASY_NAVI_ENABLE` 默认 **0**。
  一旦置 1，`get_easy_navi_bt_name()` 会被真正调用，而
  `platform/include/mw_easy_navi.h` 把它包在 `#if MW_EASY_NAVI_ENABLE` 里、
  该宏在 hcn_application 的 `dev_config.h` 中**不会定义** → 调用点变成隐式声明
  （返回 int），64 位下指针被截断。届时需要把 `mw_easy_navi.h` 的声明提到
  `#if` 之外。默认配置（0）不会触发。
- `vs_init()` 会去开 `/dev/*`（GPIO/PWM/CAN/NFC…）。在 x86 PC 上跑 device 后端
  时这些必然失败，但中间件里失败**只打日志、不返回错误**，不阻塞启动。
  验收时"一堆 /dev 打不开的报错"是预期现象。

## 完整性

本目录只提供 ② 的 30 个符号。① 的 11 个由 vehicle_services
（进而 lib_mw.a）提供；**不要把 pc 后端那些桩搬过来**，否则会重复定义。

## 收音机预置台数量（pc 模拟器 = 真机）

`vs_radio_get_preset()` 在**两个后端都是 8 个波段 × 18 个**默认预置。

- 真中间件 `radio_mw.c` 在 `radio_init()`（以及 `radio_set_region()` 切地区）调
  `radio_apply_default_presets()`，对 8 个波段各生成 18 个槽位（`DEFAULT_PRESET_N`，
  频点 = `min + j * ((max - min) / 17)`，按该地区频段范围均匀取）。
  中国区实际值：FM `87500,88705,...,107985`；AM `531,595,...,1619`。
- pc 侧 `platform/pc/mock_radio_mw.c` 的 `mock_fill_presets()` 用**同一公式 + 同一组
  中国区范围**（FM 87500~108000/100KHz，AM 531~1629/9KHz），所以 PC 上看到的
  台数与频点和上板后一致。
- 唯一差别：**AS 搜台后真机会用搜到的真实电台覆盖“当前波段”**
  （`vs_radio_scan_as(18)` 的 `count` 只是上限，实际台数 = 信号强度达标的命中数，可能少于 18），
  其余波段仍保持默认 18 个；pc 模拟器只走假搜台进度、不改预置表。
- 上电初始频点也从预置表里取（FM-1 的 `MOCK_INIT_PRESET` 位，=100755KHz），
  这样开机就有台号与高亮可验；真机的初始频点由驱动上报决定。

> PC 模拟器上 FM-1/FM-2/FM-3/AM-1 轮换均打印 `radio_get_preset(band=…) -> 18 items`，
> AM 预置行显示 `1 531 / 2 595 / 3 659 …`。

## 收音页尺寸（1024x600）

`ui/radio_page.c` 的页面按 **1024x600** 设计：预置列表一屏 6 个 = 一页，18 个台分 3 页
（1-6 / 7-12 / 13-18），横向滑动按页翻 —— 松手后吸附到最近的页边界，拖过半页就翻页、
不到半页自动回正（ViewPager 思路）；当前台落在别的页时自动跳到那一页显示。

尺寸/分页宏在 `ui/view/radio_view/radio_view.h` 的 `RADIO_PRESET_*`。吸附点是
`radio_page.c` 的 `lv_obj_set_scroll_snap_x(preset_cont, LV_SCROLL_SNAP_START)` 与
`radio_view.c` 里「只给每页第一个台保留 `LV_OBJ_FLAG_SNAPPABLE`」共同决定的 ——
这样吸附粒度是整页(0/6/12)而不是逐台。自动翻页走 `lv_obj_scroll_to_x(cont, page * RADIO_PRESET_PAGE_STEP)`，
落点与手动滑动一致。

- **上板前提**：屏不能小于 1024x600，否则页面会被裁切；
  页面创建时会在日志里打 `WARN: display … < page …` 提醒。
- **PC 模拟器**：默认分辨率改成了 1024x600（`port/hcn_application_entry.c` 的
  `HCN_APPLICATION_SCREEN`）；仪表页（home/setting/phone/ota）是 480x272 设计的，
  回看它们时用 `set HCN_APPLICATION_SCREEN=480x272` 切回去。

## 台名(PS)/电台文本(RT) 从哪来（界面不造数据）

中间件不造台名：`radio_mw.c` 的 `handle_rds_ps()` / `handle_rds_rt()` 只把驱动上报的
RDS 报文（`/dev/fmradio` 的 RDS 事件）拷进 `radio_info_t.ps/rt`，没收到就是空串。

- **AM 波段没有 RDS**，所以 AM 的 PS/RT 永远是空 -> 界面显示 `--`
  （`radio_view_refresh_ps()` / `radio_view_refresh_rt()` 对空串的处理）。
- **换台/切波段后会先变 `--`**，等 RDS 重新同步（pc 模拟器是 800ms）才会出现新台名；
  切到 AM 则是永久 `--`。真中间件本身不主动清 `ps/rt`（只覆盖驱动上报的值），
  pc 模拟器清空是为了把「换台要重新同步 RDS」这个时序演出来。
- FM 台也分「带 RDS / 不带 RDS」，不带的同样显示 `--`；顶栏的 RDS/TP/TA 图标
  来自 `radio_info_t.rds_state`，同样是驱动上报，界面不推算。
- **PC 模拟器**：`platform/pc/mock_radio_mw.c` 没有真驱动，就造了一段「假 RDS 解码」
  来演示这个时序：换台（调谐/预置/切波段/搜台结束）-> PS/RT 清空、RDS/TP 图标熄灭；
  FM 台稳定 `MOCK_RDS_DECODE_MS`(800ms) 后 -> 按台下标从 `s_rds_ps_names[]` 取一个
  PS 名填进 `ps/rt` 并点亮 RDS/TP；AM 台永远不「解码」。仅供模拟器演示，上板后这层不存在。

> PC 模拟器进收音页 -> `RDS decoded: PS="TALK FM"`（FM-1 第 12 台，列表自动翻到第 2 页）；
> 按 NEXT 换台瞬间 PS/RT 显示 `--`，0.8s 后 `PS="SPORT FM"`（第 13 台 101.9MHz，自动翻到第 3 页）；
> 切到 FM-2 后 `RDS lost (retune, was PS="TALK FM")` -> `PS="CITY FM"`；
> 切到 AM-1(531KHz) PS/RT 全 `--`、MONO、RDS/TP 图标不亮。
> 分页（鼠标拖动 -300px 回正、-600px 翻页、PREV 后自动跳回 #11 所在的第 2 页）也已截图确认。

## 上板接线清单

本目录的实现分三档：**真实现** / **可写入的状态容器** / **占位实现**。下面把「不是真实现」的全部列出来，
这些就是上板前要逐项处理的清单 —— 界面能跑不代表功能已通。

### 总览

| 文件 | 档位 | 说明 |
| --- | --- | --- |
| `dev_storage.c` | ✅ 真实现 | 里程参数走文件持久化（带校验和），系统时间走 POSIX。**不用接线** |
| `rtthread_shim.c` | ✅ 真实现（兼容层） | 用 pthread/clock 模拟 RT-Thread API。板级如果自带 RT-Thread 兼容层可整体替换 |
| `dev_ota.c` | ⚠ 状态容器 | `get_current_update_info()` + 4 个 `set_update_state_*` 已可用，**缺的是"谁去调 set_*"** —— 等真实升级服务到位 |
| `dev_navi.c` | ⚠ 部分占位 | easy_navi 3 个：结构在、数据源缺（等中间件 BLE 推数据时调 `set_easy_navi_info()`）；carlink 3 个是 占位实现 |
| `dev_pending.c` | ❌ 占位实现 | NFC + 433 无线钥匙 + `start_wifi_ap`，共 **23 个函数**，见下 |

### ❌ `dev_pending.c`：NFC（13 个函数）

当前中间件未提供 NFC 模块。接入时由板级/中间件补齐 `nfc_manager_*` 与 `nfc_*`
实现（I2C/SPI、复位脚、中断注册）；补齐前由本目录的桩维持 UI 可运行。

| 符号 | UI 用 | 现在的行为 | 接线时要做的事 |
| --- | --- | --- | --- |
| `nfc_manager_init` | — | 打日志返回 `RT_EOK` | 换成 RTOS 版的设备初始化（I2C/SPI + 复位脚 + 中断注册） |
| `nfc_manager_get_state` | — | 返回全 0 结构 | 接真实状态机 |
| `nfc_manager_get_power_state` | — | 返回 0 | 接低功耗状态 |
| `nfc_manager_send_event` | — | 空转 | 接事件下发 |
| `nfc_manager_enter_lpcd_sync` | — | 空转返回 | RTOS 版用 `rt_sem` 等中断完成；Linux 上换 `pthread_cond` / `eventfd` |
| `nfc_lpcd` | — | 返回 0 | 同上 |
| `nfc_learn` | ✅ | 置 `s_nfc_learn_status=1`（只为让 UI 显示"学习中"） | 接"开始扫卡"，扫到后写 `MILE_PARAM_NFC_UID1/2` |
| `nfc_stop_cur` | ✅ | 清标志 | 接"停止扫卡" |
| `nfc_manage_get_nfc_learn_status` | ✅ | 返回内部标志 | 改为返回驱动/状态机真实状态 |
| `nfc_delete` | ✅ | **真写**：把对应槽位清 0（落 `dev_storage.c` 的文件） | 可保留；如驱动侧也有副本要一起清 |
| `nfc_manager_get_card_count` | ✅ | **真读**：从 `MILE_PARAM_NFC_UID1/2` 数非 0 | 可保留 |
| `nfc_manage_get_card_valid` | ✅ | **真读**：槽位非 0 即有效 | 可保留 |
| `nfc_manager_get_card_uid` | ✅ | **真读**：把 4 字节 UID 从参数里抄出来 | ⚠ 目前存的是 4 字节，真实卡片 UID 常见 4/7 字节，接线时确认长度与 `mile_param_t` 的字段宽度 |

### ❌ `dev_pending.c`：433 无线钥匙（6 个函数）+ EV1527（3 个）+ WiFi AP（1 个）

当前 Linux 中间件未提供 EV1527/无线钥匙实现；接入时由板级接收模块或无线管理服务补齐。
`start_wifi_ap()` 由无线管理服务提供。

| 符号 | UI 用 | 现在的行为 | 接线时要做的事 |
| --- | --- | --- | --- |
| `start_key_learn` | ✅ | 置 `s_key_learn_active=1` | 接"进入学习模式"，收到帧后写 `MILE_PARAM_WIRELESS_KEY1/2` |
| `stop_key_learn` | ✅ | 清标志 | 接"退出学习模式" |
| `get_wireless_key_state` | ✅ | 返回内部标志 | 接真实学习状态 |
| `get_wireless_key_count` | ✅ | **真读**：数槽位非 0 | 可保留 |
| `get_wireless_key_addr` | ✅ | **真读**：取 32 位地址 | 可保留 |
| `delete_key` | ✅ | **真写**：按地址匹配后清槽位 | 可保留 |
| `ev1527_init` | — | 打日志返回 0 | 接 433 接收模块/dev 节点 + 中断（Linux：字符设备 IOCTL 或 `gpio` + `poll/epoll`） |
| `ev1527_recv` | — | **一直返回 `-RT_ETIMEOUT`**（不假装收到按键） | 接帧接收队列 |
| `ev1527_irq_enable` | — | 空实现 | ⚠ **关机防误唤醒要用**：真实实现必须在关机路径关中断 |
| `start_wifi_ap` | ✅ | 只打日志 | 接 carlink 的 `start_wifi_ap()`（Linux 侧先确认 carlink 是否已迁） |

### 接线时的三个注意点

1. **持久化那半边已经通了**：卡号/钥匙槽位真的落在 `mile_param_t` 的 `nfc_uid1/nfc_uid2/wireless_key1/wireless_key2`
   四个字段（`dev_storage.c` 的文件，重启不丢）。接线**只需要换"扫卡 / 收帧"**，存储路径不用动。
2. **RTOS 原语要替换**：RTOS 版用 `rt_device` / `rt_sem` / `rt_thread` / 中断回调；Linux 上对应
   `open()+ioctl()` 字符设备、`pthread_cond`（或 `eventfd`）、`pthread`、`poll/epoll` + `gpio`。
3. **接完要回来删桩**：本文件的桩与 `platform/pc/mock_sdk.c` 里的同名桩是**两份**（pc 后端用那份），
   别只改一边导致两个后端的 UI 表现不一致。

> 说明：`dev_navi.c` 的 carlink 与 `dev_ota.c` 的升级流程也属于“有接口、无实现”，接线时一并处理。
> 各自的文件头注释里写了缺什么。
