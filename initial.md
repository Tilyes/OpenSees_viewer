# OpenSees Viewer 项目文档

## 项目架构

```
main.cpp
  └── App (总管，PIMPL 模式)
       ├── Camera ──────── 3D 相机，鼠标拖拽 → view/proj 矩阵
       │
       ├── VkContext ───── Vulkan 子系统总协调
       │     ├── InstanceCtx ── Instance + Surface（连接 Vulkan 和窗口）
       │     ├── DeviceCtx ──── GPU 选择 + 逻辑设备 + 队列
       │     └── SwapchainCtx ─ 画布轮替系统
       │     ├── RenderPass ── 画布规格（颜色格式、清屏规则）
       │     └── CommandPool ─ 命令缓冲区内存池
       │
       ├── RenderLoop ──── 每帧 8 步流水线
       │     ├── 1 个 fence (CPU 等 GPU)
       │     ├── 1 个 semaphore (swapchain → 渲染)
       │     ├── N 个 semaphore (渲染 → 展示)
       │     └── 1 个 cmd buffer
       │
       ├── VkMeshRenderer ─ 3D 内容（三角形）
       │     ├── Pipeline ──── 画笔规则（shader + 顶点格式 + UBO 布局）
       │     ├── GpuBuffer ─── 顶点 buffer + UBO buffer
       │     └── DescriptorSet ─ UBO ↔ shader binding=0
       │
       ├── VkImGui ─────── ImGui ↔ Vulkan
       ├── Viewport ────── ImGui 面板
       ├── Inspector ───── ImGui 面板
       └── Timeline ────── ImGui 面板

数据层（待接入）:
  Mesh, Field, Model, ElementRegistry, DataSource, FileSource, OpenSeesSource
```

## 一帧执行流程

```
main_loop():
  ① 鼠标输入 → camera.orbit(yaw, pitch)
  ② imgui new_frame + 三个面板 render()      ← CPU 攒 UI 数据
  ③ render_loop.begin_frame()                ← 等 fence → 拿 swapchain 图 → 清屏
  ④ camera 算 UBO → upload → bind → draw     ← 画三角形
  ⑤ imgui.render(cmd)                        ← 画 UI
  ⑥ render_loop.end_frame()                  ← 提交 + 展示
```

---

## Vulkan 渲染全过程

### 第一阶段：搭工厂（init，只做一次）

#### 1. 创建窗口
```
glfwInit() → glfwCreateWindow(1600, 900)
```
向 Windows 申请窗口。`GLFW_NO_API` = 不用 OpenGL，自己用 Vulkan。GPU 还没碰。

#### 2. Instance + Surface
```
vkCreateInstance()          → 告诉驱动"我是 OpenSees_viewer，用 Vulkan 1.0"
glfwCreateWindowSurface()   → 把窗口和 Vulkan 连起来
```
驱动知道你了，但还没选 GPU。

#### 3. 选 GPU + Device
```
vkEnumeratePhysicalDevices()           → 列出所有显卡
vkGetPhysicalDeviceProperties()        → 看每块卡型号
选独显 RTX 5070 → physical_device_
vkCreateDevice()                       → device_（和 GPU 的通信通道）
vkGetDeviceQueue()                     → graphics_queue_, present_queue_
```
和 RTX 5070 建立了连接。两个队列：graphics 管画图，present 管展示。

#### 4. Swapchain
```
vkGetPhysicalDeviceSurfaceCapabilitiesKHR()  → 支持的分辨率范围
vkGetPhysicalDeviceSurfaceFormatsKHR()       → 支持的像素格式
vkGetPhysicalDeviceSurfacePresentModesKHR()  → 支持的刷新模式
vkCreateSwapchainKHR()                       → swapchain_
拿图片 → swapchain_images_
每张图 → ImageView
每个 ImageView 配 Framebuffer
```
创建"双缓冲画布"：两张 1600×900 的画布，一张屏幕显示、一张 GPU 后台画，画完交换。

#### 5. RenderPass + CommandPool
```
vkCreateRenderPass()     → 画布规格：SRGB 8bit、画前清黑、画后可展示
vkCreateCommandPool()    → 命令缓冲区的 malloc
```
RenderPass = 画布规格书。CommandPool = 分配"指令纸"的内存池。GPU 听不懂 C++，只认 CommandBuffer（指令纸）。

#### 6. RenderLoop（同步对象 + 命令缓冲区）
```
vkAllocateCommandBuffers() → cmd_（指令纸）
vkCreateSemaphore() → image_available_（GPU 内部灯："画布就绪"）
vkCreateSemaphore() → render_finished_（GPU 内部灯："画完了"）
vkCreateFence() → in_flight_（CPU-GPU 围栏："GPU 还在忙"）
```
- cmd_ = 一张纸，把绘图指令写上去，GPU 读它干活
- semaphore = GPU 内部排队用的信号灯
- fence = CPU 和 GPU 之间："GPU 画完没？没画完我等着"

#### 7. Pipeline（画笔规则）
```
加载 shader 文件（SPIR-V 二进制）
填 VkPipelineVertexInputStateCreateInfo    → 顶点格式：vec3 pos + vec3 normal + float
填 VkPipelineInputAssemblyStateCreateInfo  → 三个顶点拼三角形
填 VkPipelineRasterizationStateCreateInfo  → 填充三角形、不剔除
填 VkPipelineMultisampleStateCreateInfo    → 不开抗锯齿
填 VkPipelineColorBlendStateCreateInfo     → 直接输出颜色
vkCreateDescriptorSetLayout()              → UBO 布局：binding=0, uniform buffer
vkCreatePipelineLayout()                   → 带上 descriptor set layout
vkCreateGraphicsPipelines()                → 组合成最终画笔 → pipeline_
```
告诉 GPU：顶点格式是这样、shader 代码是这些、UBO 在 binding=0。

#### 8. VkMeshRenderer（画什么）
```
vertex_buffer_.create()       → 显存分配一块地，装三角形 3 个顶点
vertex_buffer_.upload()       → CPU 数据拷进显存
ubo_buffer_.create()          → 显存分配一块地，装 model/view/proj 矩阵
vkCreateDescriptorPool()      → descriptor set 内存池
vkAllocateDescriptorSets()    → 从池里拿一个 descriptor set
vkUpdateDescriptorSets()      → "binding=0 指向 ubo_buffer"
```
CPU 数据 → GPU 显存。Descriptor = 告诉 shader "binding=0 读的是这个 buffer"。

#### 9. VkImGui
```
ImGui_ImplGlfw_InitForVulkan()    → 鼠标键盘走 GLFW
ImGui_ImplVulkan_Init()           → ImGui 用你的 Vulkan 环境画
```
ImGui 是 UI 数据生成器，不是渲染器。它把按钮、面板拆成三角形，通过 Vulkan 画。

---

### 第二阶段：每帧流水线（main_loop，每 16ms 一次）

#### CPU 阶段：攒 UI 数据
```
vk_imgui->new_frame()       → "开始记账"
viewport.render()           → 记：视口面板
inspector.render()          → 记：属性面板
timeline.render()           → 记：时间轴
```
没碰 GPU。只是在内存里攒 UI 数据。

#### GPU 阶段步 1：等 + 拿纸
```
vkWaitForFences(in_flight_)          → CPU 等"GPU 上帧画完没？"
vkAcquireNextImageKHR()              → 拿一张空画布
vkResetCommandBuffer(cmd_)           → 清空指令纸
vkBeginCommandBuffer(cmd_)           → 开始写指令
vkCmdBeginRenderPass(cmd_)           → "清屏，画在这张图上"
```

#### GPU 阶段步 2：画三角形
```
ubo_buffer_.upload(新矩阵)            → 相机矩阵拷进显存
vkCmdSetViewport(cmd)                → "画在 1600×900 区域"
vkCmdSetScissor(cmd)                 → "区域外不画"
vkCmdBindVertexBuffers(cmd)          → "读这块显存里的顶点"
vkCmdBindDescriptorSets(cmd)         → "读 binding=0 的 UBO"
vkCmdBindPipeline(cmd)               → "用这支笔"
vkCmdDraw(cmd, 3)                    → "画 3 个顶点！"
```
指令纸写满了，GPU 还没执行，只是录好了。

#### GPU 阶段步 3：画 UI
```
ImGui::Render()                      → 把"记账"转成三角形
ImGui_ImplVulkan_RenderDrawData(cmd) → 画到同一张指令纸上
```

#### GPU 阶段步 4：提交 + 展示
```
vkCmdEndRenderPass(cmd_)                  → "画完了"
vkEndCommandBuffer(cmd_)                  → "指令写完了"
vkQueueSubmit(graphics_queue, cmd_)       → 交指令纸给 GPU 执行
   等 image_available_ 信号              → GPU 内部等画布就绪
   干完发 render_finished_ 信号          → 通知展示队列
   干完发 in_flight_ 信号                → 通知 CPU
vkQueuePresentKHR(present_queue, ...)    → 把画完的画布展示到屏幕
```

### GPU 执行时内部发生了什么

```
① 顶点 shader（mesh.vert 跑 3 次）：
   每个顶点坐标 × UBO 矩阵 = 屏幕坐标

② 光栅化器（GPU 固定硬件）：
   3 个屏幕坐标围成的三角形 → 切成几千个像素

③ 片段 shader（mesh.frag 跑几千次）：
   每个像素算颜色

④ 输出合并：
   颜色写进 swapchain image

⑤ Present：
   swapchain image → 显示器
```

---

## 核心概念速查

| 概念 | 一句话 | 在哪 |
|------|--------|------|
| Instance | "我是谁" | vk_instance.cpp |
| Surface | Vulkan 和窗口的桥梁 | vk_instance.cpp |
| PhysicalDevice | 物理显卡 | vk_device.cpp |
| Device | 和显卡的通信通道 | vk_device.cpp |
| Queue | 给 GPU 派活的管道 | vk_device.cpp |
| Swapchain | 双缓冲画布系统 | vk_swapchain.cpp |
| RenderPass | 画布规格（格式、清屏） | vk_context.cpp |
| CommandPool | 指令纸的 malloc | vk_context.cpp |
| CommandBuffer | 指令纸，记录 vkCmd* 命令 | vk_render_loop.cpp |
| Fence | CPU ← GPU："画完没？" | vk_render_loop.cpp |
| Semaphore | GPU 内部排队信号灯 | vk_render_loop.cpp |
| Pipeline | 画笔：shader+顶点格式+UBO布局 | vk_pipeline.cpp |
| GpuBuffer | GPU 显存封装 | vk_buffer.cpp |
| DescriptorSet | "binding=0 指向这个 buffer" | vk_mesh_renderer.cpp |
| UBO | 所有顶点共享的全局变量（相机矩阵） | vk_mesh_renderer.cpp |
| Shader | 跑在 GPU 上的小程序 | shaders/*.vert/.frag |
| ImGui | UI 数据生成器（不管渲染） | vk_imgui.cpp |

## CPU vs GPU 分工

```
CPU（你写的 C++）：                   GPU（shader + 固定硬件）：
─────────────────────                ──────────────────────────
分配显存（buffer）                    读 buffer 拿数据
拷数据进显存（upload）                 顶点 × 矩阵 = 屏幕坐标
录指令（vkCmd*）                      三角形 → 像素（光栅化）
交指令纸（submit）                    算像素颜色（fragment shader）
等完成（fence）                       写 swapchain image
                                     展示（present）
```
