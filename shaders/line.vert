#version 450
// ============================================================
// line.vert — 线段顶点着色器
// 作用：和 mesh.vert 完全一样的坐标变换逻辑（相同 UBO 布局）
// 区别：没有 gl_PointSize（画线不需要点大小）
// 使用场景：画线段（LINE_LIST）
// 被谁用：线管线 (pipeline_)
// ============================================================

// --- 输入：和 mesh.vert 完全一致 ---
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in float inScalar;

// --- UBO：和 mesh.vert 完全一样的布局，确保 descriptor set 兼容 ---
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// --- 输出 ---
layout(location = 0) out vec3 outNormal;
layout(location = 1) out float outScalar;

void main() {
    // 3D → 屏幕（和 mesh.vert 一模一样的公式）
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    outNormal = inNormal;
    outScalar = inScalar;
}
