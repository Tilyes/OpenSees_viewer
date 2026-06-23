#version 450
// ============================================================
// mesh.vert — 点/面顶点着色器
// 作用：把 3D 顶点坐标 × 相机矩阵 = 屏幕坐标，传给片段着色器
// 使用场景：画节点（POINT_LIST）+ 未来画面单元（TRIANGLE_LIST）
// 被谁用：Pipeline 加载为 SPIR-V → GPU 每顶点跑一次
// ============================================================

// --- 输入：从 C++ 的 vertex_buffer 读数据 ---
// 这三个 location 和 C++ 里 Vertex 结构体的布局一一对应
layout(location = 0) in vec3 inPosition;   // 顶点坐标 (x, y, z)
layout(location = 1) in vec3 inNormal;     // 法向量 (nx, ny, nz)
layout(location = 2) in float inScalar;    // 标量值（用于着色）

// --- UBO：所有顶点共享的全局变量 ---
// C++ 每帧上传 model/view/proj 三个矩阵
// set=0 表示第 0 组 descriptor，binding=0 对应 descriptor set layout
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;   // 模型矩阵（物体的位置和旋转）
    mat4 view;    // 视图矩阵（相机位置和朝向）
    mat4 proj;    // 投影矩阵（透视 / 正交）
} ubo;

// --- 输出：传给片段着色器 ---
layout(location = 0) out vec3 fragNormal;   // 法向量（用于光照计算）
layout(location = 1) out float fragScalar;  // 标量值（用于颜色映射）

void main() {
    // 核心：3D 坐标 → 屏幕坐标
    // 矩阵乘法从右往左：先 model 变换 → view 变换 → proj 变换
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    // 法向量跟着 model 旋转（不跟着平移，所以用 mat3 丢弃位移部分）
    fragNormal = mat3(ubo.model) * inNormal;

    // 标量直接透传
    fragScalar = inScalar;

    // 点的大小——只有 POINT_LIST 拓扑时生效。LINE_LIST 时被忽略
    gl_PointSize = 8.0;
}
