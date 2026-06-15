#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in float fragScalar;

layout(location = 0) out vec4 outColor;

// HSL → RGB 转换：彩虹渐变的关键
vec3 hsl_to_rgb(float h, float s, float l) {
    vec3 rgb = clamp(
        abs(mod(h * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0,
        0.0, 1.0);
    return l + s * (rgb - 0.5) * (1.0 - abs(2.0 * l - 1.0));
}

void main() {
    float hue = fragScalar;
    vec3 color = hsl_to_rgb(hue, 0.85, 0.55);

    // 提高亮度，线条更明显
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.5);
    outColor = vec4(color * diffuse, 1.0);
}
