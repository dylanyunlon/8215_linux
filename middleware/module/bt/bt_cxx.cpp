/**
 * @file bt_cxx.cpp
 * @brief C ABI wrapper -- 所有 extern "C" 入口 try/catch 全兜底
 *
 * 边界纪律（每条都是硬规则）：
 *   1. 异常绝不穿越 C 边界 -- 否则 std::terminate 崩整个进程
 *   2. std::string -> char* 拷贝只在本层做（strncpy + 强制 NUL 终止）
 *   3. 返回值只使用 bt_cxx.h 定义的 int 错误码
 *   4. wrapper 不写业务逻辑 -- 全部下沉到 bt_device_manager
 */
#include <cstring>

#include "bt_cxx.h"
#include "bt_device_manager.h"
#include "mw_log.h"

extern "C" int bt_cxx_probe(void) {
    try {
        return 1;  /* 适配层版本号（真实实现可返回能力位） */
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_init(void) {
    try {
        return BtDeviceManager::instance().init();
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_deinit(void) {
    try {
        BtDeviceManager::instance().deinit();
        return BT_CXX_OK;
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_add_device(const char *addr, const char *name) {
    try {
        if (!addr || !name || !*addr || !*name) {
            return BT_CXX_ERR_ARGS;  /* C 指针入参校验只在 wrapper 做 */
        }
        return BtDeviceManager::instance().add_device(addr, name);
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_get_device_count(void) {
    try {
        return BtDeviceManager::instance().device_count();
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_get_device(int index, char *name_buf, int buf_len) {
    try {
        if (!name_buf || buf_len <= 0) {
            return BT_CXX_ERR_ARGS;
        }
        std::string name;
        int rc = BtDeviceManager::instance().get_device(index, name);
        if (rc == BT_CXX_OK) {
            /* std::string -> C 缓冲区：截断安全 + NUL 终止保证 */
            std::strncpy(name_buf, name.c_str(),
                         static_cast<std::size_t>(buf_len) - 1);
            name_buf[buf_len - 1] = '\0';
        }
        return rc;
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_connect(const char *addr) {
    try {
        if (!addr || !*addr) {
            return BT_CXX_ERR_ARGS;
        }
        return BtDeviceManager::instance().connect(addr);
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_start_reconnect(const char *addr) {
    try {
        if (!addr || !*addr) {
            return BT_CXX_ERR_ARGS;
        }
        return BtDeviceManager::instance().start_reconnect(addr);
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_stop_reconnect(void) {
    try {
        BtDeviceManager::instance().stop_reconnect();
        return BT_CXX_OK;
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_start_scan(int duration_ms) {
    try {
        return BtDeviceManager::instance().start_scan(duration_ms);
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_scan_wait(int timeout_ms) {
    try {
        return BtDeviceManager::instance().scan_wait(timeout_ms);
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_demo_scan_cancel(void) {
    try {
        return BtDeviceManager::instance().scan_cancel();
    } catch (...) {
        return BT_CXX_ERR;
    }
}

extern "C" int bt_cxx_selftest(void) {
    /* 自检不依赖门控结果（开关/车速取决于运行环境），只断言容器管理逻辑 */
    try {
        int pass = 0;
        char name[64];

        if (bt_cxx_demo_init() == BT_CXX_OK) pass++;
        if (bt_cxx_demo_add_device("AA:BB:CC:DD:EE:01", "Phone-A") == BT_CXX_OK) pass++;
        if (bt_cxx_demo_add_device("AA:BB:CC:DD:EE:02", "Phone-B") == BT_CXX_OK) pass++;
        /* 重复地址应被 unordered_map 查重拦截 */
        if (bt_cxx_demo_add_device("AA:BB:CC:DD:EE:01", "Phone-A") == BT_CXX_ERR_REPEAT) pass++;
        if (bt_cxx_demo_get_device_count() == 2) pass++;
        if (bt_cxx_demo_get_device(1, name, (int)sizeof(name)) == BT_CXX_OK &&
            std::strcmp(name, "Phone-B") == 0) pass++;
        /* Worker+Timer 重连服务启停（连接结果环境相关，只断言启停闭环） */
        if (bt_cxx_demo_start_reconnect("AA:BB:CC:DD:EE:02") == BT_CXX_OK) pass++;
        if (bt_cxx_demo_stop_reconnect() == BT_CXX_OK) pass++;

        /* connect 走完整 C API 门控链，结果只打日志（环境相关） */
        int rc = bt_cxx_demo_connect("AA:BB:CC:DD:EE:01");
        mw_log_info("selftest connect rc=%d "
                    "(-4=switch off / -5=driving / 0=ok, env dependent)\n", rc);
        mw_log_info("bt_cxx selftest: %d/8 checks passed\n", pass);
        return pass;
    } catch (...) {
        return BT_CXX_ERR;
    }
}
