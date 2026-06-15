#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in float fragScalar;

layout(location = 0) out vec4 outColor;

// jet colormap: blue -> cyan -> green -> yellow -> red
vec3 jet_colormap(float t) {
    t = clamp(t, 0.0, 1.0);
    vec3 c;
    c.r = clamp(1.5 - abs(t - 0.75) * 4.0, 0.0, 1.0);
    c.g = clamp(1.5 - abs(t - 0.5) * 4.0, 0.0, 1.0);
    c.b = clamp(1.5 - abs(t - 0.25) * 4.0, 0.0, 1.0);
    return c;
}

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.3);
    vec3 color = jet_colormap(fragScalar) * diffuse;
    outColor = vec4(color, 1.0);
}
