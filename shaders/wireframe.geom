#version 450

layout(triangles) in;
layout(line_strip, max_vertices = 4) out;

layout(location = 0) in vec3 geomNormal[];
layout(location = 1) in float geomScalar[];

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out float fragScalar;

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        fragNormal = geomNormal[i];
        fragScalar = geomScalar[i];
        EmitVertex();
    }
    gl_Position = gl_in[0].gl_Position;
    fragNormal = geomNormal[0];
    fragScalar = geomScalar[0];
    EmitVertex();
    EndPrimitive();
}
