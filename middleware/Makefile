# =====================================================================
#  lib_mw.a -- HCN 中间件纯 C 核心静态库
#
#  位置    : source/vendor/hcn/middleware/Makefile （编译本目录下代码）
#  方式    : 显式源码列表（可控，新增模块在此追加一行）
#  工具链  : SDK 自带 buildroot 内部工具链（arm-buildroot-linux-gnueabi-,
#            GCC 7.4.0 / glibc），默认由 out/host 推算，可被命令行覆盖
#  原则    : 尽量减少对外部依赖
#            - lib_mw.a 纯 C 仅依赖 CC/AR；C++ 走路线A独立静态库 lib_mw_cxx.a
#            - .a 只归档不链接 -> 永无 -l 依赖；未解析符号在最终链入
#              AWTK 二进制时由链接器按需抽取 .o 解析
#            - 头文件搜索路径由 SRCS 所在目录派生，不调 find、不引入外部 -I
#            - 唯一外部头入口：EXTRA_INCFLAGS（默认空）
#
#  独立编译 : make                  （用 SDK 工具链，需先 build 出 out/host）
#  包内编译 : 由 Buildroot lib_mw.mk 调用，传入 CC=$(TARGET_CC) AR=$(TARGET_CROSS)ar
#  调试进程 : make MW_BUILD=exe     （编出独立进程 mw_app，不打包进工程）
#             adb push mw_app /tmp && adb shell /tmp/mw_app 交互式调试
# =====================================================================

# ---------- 1. SDK 工具链 ----------
# HOST_DIR/CROSS 非 make 内置变量，?= 正常生效。
# CC/AR 是 make 内置变量（内置 CC=cc），必须用 `=` 覆盖；命令行 CC=/AR= 仍可覆盖此处
#   （Buildroot .mk 即以 `make CC=$(TARGET_CC) AR=$(TARGET_CROSS)ar` 传入，优先级最高）。
# ⚠勿把 CC 改回 ?=：?= 不覆盖内置 CC=cc，会回退宿主 cc 编出 x86-64（hcn_cjson 同类坑）。
HOST_DIR ?= $(CURDIR)/../../../../out/host
CROSS    ?= arm-buildroot-linux-gnueabi-
CC       = $(HOST_DIR)/usr/bin/$(CROSS)gcc
AR       = $(HOST_DIR)/usr/bin/$(CROSS)ar
# C++ 编译器（路线A：C++ 模块独立成 lib_mw_cxx.a；Buildroot 以 CXX=$(TARGET_CXX) 传入）
CXX      = $(HOST_DIR)/usr/bin/$(CROSS)g++

# ---------- 1b. OSAL 后端选择 ----------
# MW_OS=linux(默认,本工程可构建) | rtthread | freertos
# 选定后自动编入对应 OSAL 与 GPIO 后端，并定义后端宏给 osal.h
MW_OS ?= linux
ifeq ($(MW_OS),linux)
    OSAL_SRCS := hal/osal/osal_linux.c hal/gpio/hal_gpio_linux.c \
                hal/key/hal_key_linux.c \
                hal/adc/hal_adc_linux.c \
                hal/pwm/hal_pwm_linux.c \
                hal/i2c/hal_i2c_linux.c \
                hal/wdg/hal_wdg_linux.c \
                hal/can/hal_can_linux.c
    OSAL_DEFS := -DOSAL_OS_LINUX
else ifeq ($(MW_OS),rtthread)
    OSAL_SRCS := hal/osal/osal_rtthread.c hal/gpio/hal_gpio_rtthread.c \
                hal/key/hal_key_rtthread.c \
                hal/adc/hal_adc_rtthread.c \
                hal/pwm/hal_pwm_rtthread.c \
                hal/i2c/hal_i2c_rtthread.c \
                hal/wdg/hal_wdg_rtthread.c \
                hal/can/hal_can_rtthread.c
    OSAL_DEFS := -DOSAL_OS_RTTTHREAD
else ifeq ($(MW_OS),freertos)
    OSAL_SRCS := hal/osal/osal_freertos.c hal/gpio/hal_gpio_freertos.c \
                hal/key/hal_key_freertos.c \
                hal/adc/hal_adc_freertos.c \
                hal/pwm/hal_pwm_freertos.c \
                hal/i2c/hal_i2c_freertos.c \
                hal/wdg/hal_wdg_freertos.c \
                hal/can/hal_can_freertos.c
    OSAL_DEFS := -DOSAL_OS_FREERTOS
endif

# ---------- 2. 显式源码列表（★新增模块在此追加一行）----------
# 相对本 Makefile 的路径；新增模块在末尾追加一行即可（勿忘行尾 \）
# ⚠续行必须用空格缩进，不可用 TAB（TAB 是 recipe 专用，变量续行用 TAB 会报 missing separator）
SRCS := \
    3rd/cJson/cJSON.c \
    3rd/cJson/json_utils.c \
    common/pthread/mw_pthread.c \
    common/timer/mw_timer.c \
    $(OSAL_SRCS) \
    hal/bsp/hal_bsp.c \
    common/task_server/task_server.c \
    common/utils/usr_utils.c \
    core/mw_init.c \
    core/mw_common.c \
    core/mw_version.c \
    core/mw_modules.c \
    module/set_param/find_file_name.c \
    module/set_param/json_param_parse.c \
    module/set_param/set_param.c \
    module/vehicle_param/vehicle_param.c \
    module/dev_state/dev_state.c \
    module/key_module/adc_key.c \
    module/key_module/key_common.c \
    module/io_module/gpio_light.c \
    module/backlight/usr_backlight.c \
    module/light_sensor/mw_light_sensor.c \
    module/display_mode/mw_display_mode.c \
    module/watchdog/mw_watchdog.c \
    module/can_module/mw_can_rx.c \
    module/can_module/mw_can_tx.c \
    module/msg_manage/msg_manage.c \
    module/maintence/mw_mileage_maintence.c \

#   mcu_comm/mcu_comm.c \
#   mcu_comm/protocol.c \
#   aw88082/aw88082.c \
#   fm_radio/fm_radio.c \
#   adc/adc.c \
#   datahub/datahub.c

# ---------- 2a. C++ 源码列表（路线A：独立静态库 lib_mw_cxx.a）----------
# 与 C 源同轮编译（一次 make 全部触发）；lib_mw.a 保持纯 C 不受影响
# 纪律：对外仅 extern "C" 接口（见 module/bt/bt_cxx.h），实现入口 try/catch 兜底，
#       异常禁止穿越 C 边界；禁止 STL 类型出现在对外头文件
# ⚠续行必须用空格缩进，不可用 TAB（同 SRCS）
CPPSRCS := \
    module/bt/bt_cxx.cpp \
    module/bt/bt_device_manager.cpp \

# ---------- 2b. 产物形态（MW_BUILD）----------
# lib (默认) : 仅 lib_mw.a，行为与历史版本完全一致（Buildroot 走此路径）
# exe        : 仅 mw_app 独立调试进程（main 在 app/mw_main.c，交互命令行）
# all        : 两个产物都编
# ⚠mw_app 链接必须 -lpthread -lm（cJSON 用 sqrt/fabs/pow，.a 模式从不触发）
# ⚠切换 MW_BUILD 前先 make clean（build/*.o 不感知选项变化，避免混链）
MW_BUILD ?= lib
ifeq ($(MW_BUILD),lib)
    PRODUCTS := lib_mw.a lib_mw_cxx.a
else ifeq ($(MW_BUILD),exe)
    PRODUCTS := mw_app
    SRCS += app/mw_main.c
    CFLAGS += -g
else ifeq ($(MW_BUILD),all)
    PRODUCTS := lib_mw.a mw_app
    SRCS += app/mw_main.c
    CFLAGS += -g
else
    $(error MW_BUILD 仅支持 lib | exe | all，当前: $(MW_BUILD))
endif
EXE_LDLIBS := -lpthread -lm

# ---------- 3. 产物 ----------
OBJDIR := build
OBJS := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
CPPOBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPPSRCS))
DEPS := $(OBJS:.o=.d) $(CPPOBJS:.o=.d)

# ---------- 4. 编译选项 ----------
# -fPIC     : .a 将链入 AWTK 二进制(可能 PIE)，须位置无关
# -std=gnu11: 固定语言标准，减少工具链默认差异
# -Wall -O2 : 警告 + 优化
# 头文件路径：由 SRCS 所在目录自动派生（. 与各模块目录），无需 find、无需单独维护 -I
#             仅覆盖本库内部头，不引入任何外部 include
# sysroot   : 由 SDK 交叉 gcc 包装器自带，无需显式 --sysroot
# -pthread  : 多线程编译规范（_REENTRANT；.a 不链接，pthread 符号由 AWTK -lpthread 解析）
CFLAGS  += -fPIC -std=gnu11 -Wall -O2 -pthread
CFLAGS  += $(addprefix -I,$(sort . common/config common/log common/lock module/uart_tpms hal/include $(dir $(SRCS))))
CFLAGS  += $(OSAL_DEFS)
# 唯一外部头入口：默认空 = 零外部依赖；需要时由调用方传入，如
#   make EXTRA_INCFLAGS=-I$(STAGING_DIR)/usr/include
CFLAGS  += $(EXTRA_INCFLAGS)

# ---------- 4b. C++ 编译选项（lib_mw_cxx.a）----------
# -std=c++14              : GCC7.4 完整支持；勿用 C++20（filesystem 还需 -lstdc++fs）
# -fexceptions -funwind-tables : ARM32 异常展开必须显式保证
# -fvisibility=hidden     : 符号默认隐藏，仅 extern "C" wrapper 导出
#                           （静态链入单一可执行文件，静态链接期照常解析，无碍）
# 头文件路径/宏与 CFLAGS 同源（按 CPPSRCS 派生），EXTRA_INCFLAGS 同样透传
CXXFLAGS += -fPIC -std=c++14 -Wall -O2 -fexceptions -funwind-tables -pthread
CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
CXXFLAGS += $(addprefix -I,$(sort . common/config common/log common/lock module/uart_tpms hal/include $(dir $(SRCS)) $(dir $(CPPSRCS))))
CXXFLAGS += $(OSAL_DEFS)
CXXFLAGS += $(EXTRA_INCFLAGS)
ARFLAGS  := rcs

# ---------- 5. 规则 ----------
.PHONY: all clean checkcc

all: checkcc $(PRODUCTS)
	@printf '\n\033[1;32m✅[hcn_mw] hcn middleware build success\033[0m  ->  %s\n' '$(PRODUCTS)'

# 工具链存在性检查（仅 all 依赖；clean 不受影响）
checkcc:
	@test -x "$(CC)" || { echo "ERROR: SDK 工具链未找到: $(CC)（先编译 buildroot 工具链，或 make CC=... AR=... 覆盖）"; exit 1; }
	@test -x "$(CXX)" || { echo "ERROR: SDK 交叉 g++ 未找到: $(CXX)（defconfig 需开 BR2_TOOLCHAIN_BUILDROOT_CXX）"; exit 1; }

# 归档：ar 仅打包 .o，不链接 -> 天然无外部库依赖
lib_mw.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

# C++ 适配层归档（路线A）：与 lib_mw.a 完全独立，纯 C 消费者不受影响
lib_mw_cxx.a: $(CPPOBJS)
	$(AR) $(ARFLAGS) $@ $^

# 链接：独立调试进程（MW_BUILD=exe|all）
# -lm 必需：cJSON 用 sqrt/fabs/pow；-lpthread：OSAL 线程/信号量
# 含 C++ 对象 -> 必须用 CXX(g++) driver 链接，自动链 libstdc++（异常展开亦依赖它）
mw_app: $(OBJS) $(CPPOBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(EXE_LDLIBS)

# 编译：.o 落入 build/ 镜像源码目录结构，源码树不被 .o 污染
#       -MMD -MP 自动生成头文件依赖，改头后只重编受影响 .o
$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# C++ 编译：与 %.c 同层模式规则，同一轮 make（-j 并行）触发，依赖跟踪同走 -MMD
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJDIR) lib_mw.a lib_mw_cxx.a mw_app

# =====================================================================
#  说明：cJSON 已并入本库统一编译（hcn_cjson 包已删除）
#  - cJSON.c/json_utils.c 编进 libhcn_mw.a；头文件 cJSON.h/json_utils.h
#    由 hcn_mw.mk 装到 staging/usr/include/（#include <cJSON.h> 兼容）。
#  - hcn_cjson 包已删除：源 Makefile 已删、源码改由本 Makefile 统一编译；
#    configs/package/hcn_cjson/、Config.in 的 source 行、defconfig 行均已移除。
#  - ⚠链接期 -lm：cJSON.c 用 sqrt/fabs（math.h），libhcn_mw.a 只归档不链接故
#    本 Makefile 无需 -lm；最终链入 AWTK 二进制时，AWTK 链接行需加 -lm，
#    否则报 pow/sqrt/fabs 等 undefined reference。
#  - C++ 路线A已落地：C++ 模块编入独立 lib_mw_cxx.a（CPPSRCS），lib_mw.a 保持
#    纯 C；对外仅 extern "C"（module/bt/bt_cxx.h 为范式）。第三方 vendor .so 由
#    C++ 适配层 dlopen（RTLD_NOW|RTLD_LOCAL，阶段化启动，不进 application 关键路径）。
#    最终消费者（AWTK）链接行需 -lmw_cxx -lstdc++（见 application/src/SConscript）。
# =====================================================================
