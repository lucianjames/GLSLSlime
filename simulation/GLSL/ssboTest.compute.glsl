#version 460 core

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img;

// An std::vector<positions> of X+Y coordinates will be passed to the shader
// (positions is a struct consisting of two integers)
layout(std430, binding = 0) buffer Positions {
    ivec2 positions[];
};

// Each thread will be responsible for one position, so we can use the
// gl_GlobalInvocationID.x to index into the positions array
void main() {
    ivec2 pos = ivec2(positions[gl_GlobalInvocationID.x]);
    imageStore(img, pos, vec4(0.0, 0.0, 0.0, 1.0));
}