# VkContext 重构记录

## 日期
2026-06-02

## 重构目标
- 解决 VkContext 单文件 400+ 行、职责过多的问题
- 解决 app.h 暴露 Vulkan 头文件、编译慢的问题

---

## 1. VkContext 拆分

### 拆分前
```
VkContext（一个类管全部）
├── Instance + Surface
├── PhysicalDevice + LogicalDevice + Queues
├── Swapchain + ImageViews + Framebuffers
├── RenderPass
└── CommandPool
```

### 拆分后
```
VkContext（协调整合者，~50 行）
├── InstanceCtx    -- Instance + Surface
├── DeviceCtx      -- PhysicalDevice 选择 + LogicalDevice + Queue 获取
├── SwapchainCtx   -- Swapchain + ImageViews + Framebuffers
├── RenderPass     -- 仍在 VkContext（够简单，不值得单独成类）
└── CommandPool    -- 仍在 VkContext
```

### 新增文件
| 文件 | 类名 | 职责 |
|------|------|------|
| `vk_instance.h/.cpp` | `InstanceCtx` | Instance 创建/销毁、Surface 创建/销毁、validation layer 管理 |
| `vk_device.h/.cpp` | `DeviceCtx` | 选择物理设备、创建逻辑设备、获取 Queue 句柄、QueueFamilyIndices 查询 |
| `vk_swapchain.h/.cpp` | `SwapchainCtx` | Swapchain 创建/销毁/重建、ImageViews、Framebuffers |

### 为什么这样拆
- **InstanceCtx** — 纯 OS/窗口层，与具体 GPU 无关。未来加 headless 渲染只需换这个
- **DeviceCtx** — 封装 GPU 选择和通信。未来加多 GPU 支持时扩展这个
- **SwapchainCtx** — 窗口 resize 时只重建这个，不影响其他
- RenderPass 和 CommandPool 留在 VkContext：它们各只有几十行，过度拆分反而增加文件数

### 注意：类命名避免与 Vulkan 类型冲突
Vulkan SDK 中已有 `VkInstance`、`VkDevice`、`VkSwapchainKHR` 等类型名。
自己的 wrapper 类统一用 `XxxCtx` 后缀（Ctx = Context），避免之前 `VkPipeline` 和 Vulkan 的 `VkPipeline` 重名的问题。

---

## 2. app.h PIMPL 重构

### 问题
`app.h` 直接 include `vk_context.h` 和 `vk_imgui.h`，导致任何 include `app.h` 的文件都被迫引入完整的 Vulkan 头文件链（数万行），编译变慢。

### 解决方案
使用 `std::unique_ptr` + 前置声明 + 析构函数定义在 `.cpp`：

```cpp
// app.h — 不暴露 Vulkan
class VkContext;      // 前置声明
class VkImGui;        // 前置声明

class App {
public:
    ~App();           // 声明但不实现
private:
    std::unique_ptr<VkContext> vk_ctx_;    // OK，unique_ptr 允许 incomplete type
    std::unique_ptr<VkImGui> vk_imgui_;
};

// app.cpp — 看到完整类型，析构函数在此实现
#include "renderer/vk_context.h"
#include "renderer/vk_imgui.h"

App::~App() = default;  // 此时 VkContext/VkImGui 已是 complete type
```

### 为什么之前失败
之前没有显式声明 `~App()`，编译器试图在 `app.h` 自动生成析构函数，此时只看到前置声明，`unique_ptr` 的 default deleter 触发 `static_assert(sizeof(T) > 0)`。

解决方式：在 `app.h` 声明 `~App()`，在 `app.cpp` 用 `= default` 实现。此时完整类型可见，编译通过。

---

## 3. 重构后的依赖关系

```
main.cpp
  └── App (app.h: 只前置声明, 不暴露 Vulkan)
        ├── VkContext (组合者)
        │     ├── InstanceCtx   (Instance + Surface)
        │     ├── DeviceCtx     (GPU + Queue)
        │     └── SwapchainCtx  (Swapchain + ImageViews + Framebuffers)
        └── VkImGui
```

### SwapchainCtx 两阶段初始化

拆分原因是循环依赖：
- `create_render_pass()` 需要 `swapchain_format`（swapchain 创建后才确定）
- `create_framebuffers()` 需要 `render_pass`（render_pass 创建后才存在）

```
init_phase1  →  swapchain + image_views  (format 可用)
create_render_pass  →  使用 format
init_phase2  →  framebuffers  (使用 render_pass)
```

### CMake 变化
- `viewer_renderer` 新增三个源文件：`vk_instance.cpp`、`vk_device.cpp`、`vk_swapchain.cpp`
- 每个子模块显式 link 它需要的库（不再靠全局传递）

---

## 4. 为什么这些改动利于后期维护

| 改动 | 维护收益 |
|------|---------|
| InstanceCtx 独立 | 换 Vulkan 版本/加 headless 模式只改一个文件 |
| DeviceCtx 独立 | 加多 GPU 支持/GPU 特性检测 只改一个文件 |
| SwapchainCtx 独立 | resize 逻辑完全封装，不影响其他模块 |
| PIMPL on App | 改 Vulkan 头文件不触发全项目重编译 |
| 显式 link 依赖 | 新人加模块时不会"莫名其妙能编译但链接报错" |
