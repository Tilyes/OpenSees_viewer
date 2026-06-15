#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in float inScalar;

void main() {
    // 直接用 -1~1 坐标，不加变换（无 UBO）
    gl_Position = vec4(inPosition, 1.0);
}
