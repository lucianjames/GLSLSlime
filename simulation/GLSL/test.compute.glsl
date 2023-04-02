#version 460 core
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img;

// Do something cool and animated for testing purposes
void main(){
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(pos) / vec2(imageSize(img));
    vec3 col = vec3(0.0);
    col.r = sin(uv.x * 10.0 + uv.y * 10.0 + float(gl_NumWorkGroups.x) * 0.1);
    col.g = sin(uv.x * 10.0 + uv.y * 10.0 + float(gl_NumWorkGroups.y) * 0.1);
    col.b = sin(uv.x * 10.0 + uv.y * 10.0 + float(gl_NumWorkGroups.z) * 0.1);
    imageStore(img, pos, vec4(col, 1.0));
}