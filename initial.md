# OpenSees Viewer 项目文档

> 基于 C++20 + Vulkan 的 OpenSees 有限元模型 3D 可视化工具

## 当前功能状态

| 功能 | 状态 | 说明 |
|------|------|------|
| Vulkan 渲染框架 | ✅ | Instance → Device → Swapchain → RenderPass → CommandPool 全链路 |
| 帧循环 | ✅ | 双缓冲 swapchain + fence/semaphore 同步 |
| ImGui 界面 | ✅ | Win11 Fluent 风格，菜单栏 + 面板固定布局 |
| 3D 相机 | ✅ | 鼠标左键旋转、滚轮缩放、自动适配模型包围盒 |
| 离屏渲染 | ✅ | 3D → 离屏纹理 → ImGui::Image 显示在 Viewport 面板 |
| TCL 文件解析 | ✅ | 识别 node/element 命令（forceBeamColumn/truss/zeroLength） |
| 杆系结构显示 | ✅ | LINE_LIST 画线 + POINT_LIST 画节点，彩虹渐变色 |
| set 变量替换 | ⏳ | 简单 set 支持，[expr...] 待支持 |
| 文件对话框 | ⏳ | 当前硬编码路径，待加原生文件选择 |
| 时间步动画 | ⏳ | Timeline 面板已就绪，待接入 Field 数据 |
| 应力云图 | ⏳ | contour.frag 已写好，待接入 |

---

## 项目文件结构

```
OpenSees_viewer/
├── CMakeLists.txt              ← 顶层 CMake
├── CMakePresets.json           ← vcpkg + 编译预设
├── vcpkg.json                  ← 依赖清单
├── .gitignore
│
├── shaders/                    ← GLSL 着色器（编译成 .spv）
│   ├── mesh.vert / mesh.frag   ← 点 shader（带光照 + UBO）
│   ├── line.vert / line.frag   ← 线 shader（亮色彩虹，无光照）
│   ├── simple.vert/.frag       ← 无 UBO 测试用 shader（备用）
│   ├── contour.frag            ← 应力云图 shader（备用）
│   └── wireframe.geom          ← 几何着色器画粗线（备用）
│
├── src/
│   ├── main.cpp                ← 程序入口
│   │
│   ├── core/                   ← 数据模型层（不依赖渲染）
│   │   ├── mesh.h/.cpp         ← 节点 + 单元拓扑
│   │   ├── field.h/.cpp        ← 结果场（位移/应力/应变）
│   │   ├── model.h/.cpp        ← Mesh + 时间步列表
│   │   └── element_registry.h/.cpp ← 单元类型注册表
│   │
│   ├── adapter/                ← 数据源层
│   │   ├── data_source.h       ← DataSource 抽象接口
│   │   ├── file_source.h/.cpp  ← TCL 文件解析器
│   │   └── opensees_source.h/.cpp ← OpenSees 直连（stub）
│   │
│   ├── renderer/               ← Vulkan 渲染层
│   │   ├── vulkan/             ← Vulkan 底层（通常不需要看）
│   │   │   ├── vk_instance.h/.cpp  ← Instance + Surface
│   │   │   ├── vk_device.h/.cpp    ← GPU 选择 + Device + Queue
│   │   │   └── vk_swapchain.h/.cpp ← Swapchain 双缓冲系统
│   │   ├── vk_context.h/.cpp   ← 协调上述三者 + RenderPass + CommandPool
│   │   ├── vk_render_loop.h/.cpp ← 每帧流水线（fence/semaphore/cmd）
│   │   ├── vk_pipeline.h/.cpp  ← 画笔规则（shader + 顶点格式 + UBO）
│   │   ├── vk_buffer.h/.cpp    ← GPU 显存封装
│   │   ├── vk_mesh_renderer.h/.cpp ← 3D 内容绘制（线/点）
│   │   ├── vk_render_texture.h/.cpp ← 离屏渲染目标（ImGui 可显示）
│   │   ├── vk_imgui.h/.cpp     ← ImGui ↔ Vulkan 接线员
│   │   └── camera.h/.cpp       ← 3D 相机
│   │
│   ├── ui/                     ← ImGui 面板层
│   │   ├── app.h/.cpp          ← App 总管（PIMPL 模式）
│   │   ├── viewport.h/.cpp     ← 3D 视口面板
│   │   ├── inspector.h/.cpp    ← 属性面板
│   │   └── timeline.h/.cpp     ← 时间轴面板
│   │
│   └── utils/
│       ├── log.h               ← spdlog 封装
│       └── file_io.h/.cpp      ← 文件读写（shader 加载等）
│
├── example/                    ← 示例 TCL 模型
│   └── exam1/simple.tcl        ← 简化版 2D 杆系模型
│
└── docs/
    └── refactor_vkcontext.md   ← VkContext 重构记录
```

---

## 当前渲染流程（三阶段）

```
main_loop() 每帧执行：

┌─ 第一阶段：CPU 攒 UI 数据 ─────────────────────┐
│ vk_imgui->new_frame()                         │
│ 菜单栏 render                                  │
│ Viewport.render_ui()  ← 只是占位，3D 画面等下画 │
│ Inspector.render()                            │
│ Timeline.render()                             │
│ 鼠标 → camera.orbit/zoom                       │
└───────────────────────────────────────────────┘
              ↓
┌─ 第二阶段：离屏渲染 3D ────────────────────────┐
│ vp_cmd 命令缓冲区：                              │
│   vkCmdBeginRenderPass(viewport 的离屏 framebuffer) │
│   ubo = {model, view, proj}                   │
│   mesh_renderer->render(cmd, extent, ubo)      │
│     → 绑顶点 → 绑 UBO → 画线 → 画点              │
│   vkCmdEndRenderPass                           │
│   布局转换（COLOR_ATTACHMENT → SHADER_READ）     │
│ vkQueueSubmit → GPU 离线渲染                    │
└───────────────────────────────────────────────┘
              ↓
┌─ 第三阶段：swapchain 渲染 UI ───────────────────┐
│ render_loop->begin_frame (拿 swapchain 图)       │
│   在 Viewport 面板里：ImGui::Image(离屏纹理)       │
│ vk_imgui->render(cmd) ← 画所有 UI 面板          │
│ render_loop->end_frame (提交 + 展示)             │
└───────────────────────────────────────────────┘
```

---

## 核心组件职责

| 组件 | 文件 | 一句话 |
|------|------|--------|
| App | ui/app.cpp | 总管所有模块的创建、帧循环编排、销毁 |
| VkContext | renderer/vk_context.cpp | 协调 Vulkan 子模块，对外暴露 getter |
| InstanceCtx | renderer/vulkan/vk_instance.cpp | Instance + Surface，连接 Vulkan 和 OS |
| DeviceCtx | renderer/vulkan/vk_device.cpp | 选 GPU，建 Device，获取 Queue |
| SwapchainCtx | renderer/vulkan/vk_swapchain.cpp | 双缓冲画布，窗口 resize 时重建 |
| Renderloop | renderer/vk_render_loop.cpp | 帧流水线：等 fence → 拿图 → submit → present |
| Pipeline | renderer/vk_pipeline.cpp | 画笔规则：shader + 顶点格式 + UBO 布局（创建时可指定拓扑） |
| GpuBuffer | renderer/vk_buffer.cpp | GPU 显存：create(分配) → upload(拷贝) → destroy(释放) |
| VkMeshRenderer | renderer/vk_mesh_renderer.cpp | 3D 内容：upload_mesh 把 Mesh 转 GPU 顶点，render 画线+画点 |
| RenderTexture | renderer/vk_render_texture.cpp | 离屏图像 + framebuffer + ImGui 纹理注册 |
| VkImGui | renderer/vk_imgui.cpp | ImGui↔Vulkan 接线，Win11 Fluent 风格 |
| Viewport | ui/viewport.cpp | 持有 RenderTexture + Camera，ImGui::Image 显示 3D |
| Inspector | ui/inspector.cpp | 模型信息面板（纯 ImGui） |
| Timeline | ui/timeline.cpp | 时间步控制面板（纯 ImGui） |
| FileSource | adapter/file_source.cpp | TCL 解析器：识别 node/element 生成 Mesh |
| Mesh | core/mesh.cpp | 节点坐标 + 单元拓扑 |
| Model | core/model.cpp | Mesh + 时间步列表 |

---

## 如何编译运行

### 环境要求
- Windows 10/11
- Visual Studio 2022 (Community 即可)
- Vulkan SDK 1.4+
- vcpkg（设 `VCPKG_ROOT` 环境变量）

### 编译
```bash
# 1. 安装依赖
vcpkg install --x-manifest-root=.

# 2. 编译 shader
cd shaders
glslc mesh.vert -o mesh.vert.spv
glslc mesh.frag -o mesh.frag.spv
glslc line.vert  -o line.vert.spv
glslc line.frag  -o line.frag.spv

# 3. VS 打开文件夹 → 选 CMake preset "default" → Build
```

### 运行
F5 启动 → File → Open Model... → 选 example/exam1/simple.tcl → 显示杆系结构

---

## 后续开发方向

| 优先级 | 任务 | 涉及文件 |
|--------|------|---------|
| 1 | 文件选择对话框 | app.cpp |
| 2 | set/expr 变量支持 | file_source.cpp |
| 3 | 深度缓冲（模型前后遮挡） | vk_context.cpp, vk_pipeline.cpp |
| 4 | 纹理尺寸随窗口调整 | viewport.cpp |
| 5 | 时间步动画 | timeline.cpp, app.cpp |
| 6 | 应力云图 | contour.frag, vk_mesh_renderer.cpp |
| 7 | 多视口 | viewport.cpp, app.cpp |
| 8 | OpenSees 子进程接入 | opensees_source.cpp |
