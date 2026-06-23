#version 450
// ============================================================
// line.frag — 纯亮色彩虹片段着色器
// 作用：和 mesh.frag 一样的彩虹色映射，但不乘光照暗化
// 原因：线只有 1px 宽，再暗化会完全看不见
// 使用场景：画线段（配合 line.vert）
// 被谁用：线管线 (pipeline_)
// ============================================================

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in float fragScalar;

layout(location = 0) out vec4 outColor;

// HSL → RGB（和 mesh.frag 一样）
vec3 hsl_to_rgb(float h, float s, float l) {
    vec3 rgb = clamp(abs(mod(h * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return l + s * (rgb - 0.5) * (1.0 - abs(2.0 * l - 1.0));
}

void main() {
    float hue = fragScalar;
    vec3 color = hsl_to_rgb(hue, 0.85, 0.55);
    outColor = vec4(color, 1.0);  // 直接输出亮色，不乘 diffuse
}
