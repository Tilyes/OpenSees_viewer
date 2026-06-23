 #version 450
// ============================================================
// mesh.frag — 带光照的彩虹片段着色器
// 作用：根据 scalar 值生成彩虹色，再乘光照暗化 = 有立体感的颜色
// 使用场景：画节点（配合 mesh.vert）
// 被谁用：点管线 (point_pipeline_)
// ============================================================

// --- 输入：从顶点着色器传来的插值数据 ---
layout(location = 0) in vec3 fragNormal;    // 法向量（GPU 自动在三角形内插值）
layout(location = 1) in float fragScalar;   // 标量值（GPU 自动在三角形内插值）

// --- 输出：这个像素的最终颜色 ---
layout(location = 0) out vec4 outColor;     // RGBA

// HSL → RGB 转换：把色相/饱和度/亮度变成红绿蓝
// h(0~1) = 红→黄→绿→青→蓝→紫→红
vec3 hsl_to_rgb(float h, float s, float l) {
    vec3 rgb = clamp(
        abs(mod(h * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0,
        0.0, 1.0);
    return l + s * (rgb - 0.5) * (1.0 - abs(2.0 * l - 1.0));
}

void main() {
    // 1. 用标量值决定色相
    float hue = fragScalar;                    // 0.0=红, 0.33=绿, 0.66=蓝, 1.0=紫
    vec3 color = hsl_to_rgb(hue, 0.85, 0.55); // 饱和度 85%, 亮度 55%

    // 2. 简单光照：法向量和光源方向的夹角决定明暗
    //    正面朝向光源 → 亮；侧面或背面 → 暗
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));         // 光源在右上前方
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.5); // dot = cos 夹角
    //                                                ↑ 最低 0.5 保证阴影不会全黑

    // 3. 最终颜色 = 彩虹色 × 光照暗化
    outColor = vec4(color * diffuse, 1.0);
}
