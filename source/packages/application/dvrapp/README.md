# DVR Application - Refactored Architecture

**Version**: Phase 1 - Basic UI Framework  
**Date**: 2025-10-30  
**Status**: Initial Implementation

---

## 项目概述

这是DVR应用的重构版本，采用清晰的分层架构：

- **Backend Layer**: C++ 业务逻辑（DVRBackend, Managers）
- **UI Layer**: QML 声明式界面
- **Integration**: CQObjListener 系统事件集成

---

## 目录结构

```
app_refactor/
├── backend/                    # C++ Backend层
│   ├── dvrbackend.h/cpp       # 主协调器（已实现CQObjListener）
│   ├── cameramanager.h/cpp    # 摄像头管理（空桩）
│   ├── previewmanager.h/cpp   # 预览管理（空桩）
│   ├── recordmanager.h/cpp    # 录制管理（空桩）
│   ├── playerbackend.h/cpp    # 播放管理（空桩）
│   └── filemanager.h/cpp      # 文件管理（空桩）
├── qml/                        # QML UI层
│   ├── main.qml               # 应用入口
│   ├── MainWindow.qml         # 主窗口
│   └── components/            # UI组件
│       ├── ToolbarComponent.qml        # 顶部工具栏
│       ├── ButtonContainerComponent.qml # 底部按钮栏
│       ├── SettingsPanelComponent.qml  # 设置面板
│       └── ToastComponent.qml          # 消息提示
├── resources/                  # 资源文件
│   └── images/                # 图标（待添加）
├── main.cpp                    # 主入口
├── dvr_refactor.pro           # qmake工程文件
└── resources.qrc              # Qt资源文件

```

---

## Phase 1 功能清单

### ✅ 已实现

1. **目录结构搭建**
   - 完整的代码目录层次
   - Backend和QML分离

2. **DVRBackend主协调器**
   - CQObjListener集成（doVideoFocusChanged, doAudioFocusChanged）
   - 基本生命周期管理（initialize, shutdown）
   - QML属性暴露（isDualCameraMode, isPreviewing, isRecording）

3. **Manager类框架**
   - CameraManager (空桩)
   - PreviewManager (空桩)
   - RecordManager (空桩)
   - PlayerBackend (空桩)
   - FileManager (空桩)

4. **QML主界面**
   - 全屏窗口（支持800x480和1024x600）
   - 响应式布局（isSmallScreen自适应）
   - 顶部工具栏（标题+退出按钮）
   - 底部按钮栏（5个按钮：录制、拍照、切换、设置、文件）
   - 设置面板（右侧overlay）
   - Toast消息提示
   - 预览区域占位符

5. **构建配置**
   - qmake .pro文件
   - Qt资源文件 (.qrc)

### ❌ 未实现（后续Phase）

- 实际预览功能（DVR_StartDualPreview）
- 录制功能（DVR_StartDualRecord）
- 文件列表和播放
- 图标资源集成
- 设置项持久化

---

## Phase 1 验证目标

**主要验证点**：
1. ✅ QML窗口是否能正常显示
2. ✅ CQObjListener是否正确注册
3. ✅ 窗口生命周期管理（打开、关闭、再打开）
4. ✅ 是否解决"第二次打开黑屏"问题

---

## 构建和运行

### 1. 生成Makefile

```bash
cd src/app_refactor
qmake dvr_refactor.pro
```

### 2. 编译

```bash
make
```

### 3. 运行

```bash
../../bin/dvr_refactor
```

---

## 调试日志

Phase 1实现了完整的日志追踪：

```
[Main] DVR Application Starting (Refactored)
[DVRBackend] DVRBackend constructor
[DVRBackend] DVRBackend initializing...
[DVRBackend] Registered as CQObjListener
[main.qml] Root window created: 1024 x 600
[MainWindow] Created - Screen: 1024 x 600 Small: false
[Toolbar] Exit button clicked
[DVRBackend] DVRBackend destructor - start cleanup
[DVRBackend] Unregistered from CQObjListener
```

---

## 已知问题和注意事项

### 1. CQObjListener生命周期

**关键修复**：
- DVRBackend在构造函数中不直接注册CQObjListener
- 改为在 `initialize()` 方法中注册
- 在 `shutdown()` 方法中取消注册
- 确保析构函数调用 `shutdown()` 清理

**原因**：防止C++对象销毁后回调函数仍被调用导致的崩溃。

### 2. QML对象生命周期

**正确模式**：
```cpp
// main.cpp
DVRBackend backend;  // Stack allocation
backend.initialize();
engine.rootContext()->setContextProperty("dvrBackend", &backend);
// backend在main函数结束时自动析构
```

### 3. 资源文件

Phase 1使用Unicode字符作为按钮图标占位符：
- 📷 = Snapshot
- 🔄 = Switch Camera
- ⚙ = Settings
- 📁 = Directory

Phase 2+将替换为实际PNG图标。

---

## 下一步计划（Phase 2+）

1. **PreviewManager实现**
   - DVR_StartDualPreview/SinglePreview集成
   - Surface管理
   - 摄像头切换

2. **RecordManager实现**
   - DVR_StartDualRecord/SingleRecord
   - 录制状态管理
   - 时长显示

3. **文件列表和播放**
   - FileManager实现
   - 视频缩略图
   - 播放器界面

4. **图标资源**
   - 复用原app的PNG图标
   - 更新resources.qrc

5. **设置持久化**
   - QSettings集成
   - 分辨率、时长等配置保存

---

## 参考文档

- `docs/qml_refactor/00_OVERVIEW.md` - 架构概览
- `docs/qml_refactor/01_BACKEND_DESIGN.md` - Backend设计（含UI布局）
- `docs/qml_refactor/02_QML_DESIGN.md` - QML设计
- `docs/qml_refactor/03_SURFACE_INTEGRATION.md` - Surface集成

---

**最后更新**: 2025-10-30  
**作者**: DVR开发团队
