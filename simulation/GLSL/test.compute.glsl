#version 460 core
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img;

void main(){
    vec4 color = imageLoad(img, ivec2(gl_WorkGroupID.xy));
    float x = float(gl_WorkGroupID.x % 255) / 255.0;
    float y = float(gl_WorkGroupID.y % 255) / 255.0;
    color.r *= 0.9;
    color.g *= 0.9;
    color.r += x*0.1;
    color.g += y*0.1;
    imageStore(img, ivec2(gl_WorkGroupID.xy), color);
}