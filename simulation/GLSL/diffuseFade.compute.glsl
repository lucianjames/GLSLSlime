#version 460

#define GROUP_SIZE 32

layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE) in;
layout(rgba32f, binding = 0) uniform image2D img;

uniform float diffuse;
uniform float fade;
uniform int size;

shared vec4 block[GROUP_SIZE+2][GROUP_SIZE+2];

void main(){
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 local_coords = ivec2(gl_LocalInvocationID.xy);
    ivec2 group_coords = ivec2(gl_WorkGroupID.xy);

    // Load block of pixels into shared memory
    block[local_coords.x+1][local_coords.y+1] = imageLoad(img, pixel_coords);

	// If at edge of local work group, load the edge pixels into shared memory
    if (local_coords.x == 0) { // At left edge
        block[0][local_coords.y+1] = imageLoad(img, pixel_coords + ivec2(-1,0));
    }
    if (local_coords.x == GROUP_SIZE-1) { // At right edge
        block[GROUP_SIZE+1][local_coords.y+1] = imageLoad(img, pixel_coords + ivec2(1,0));
    }
    if (local_coords.y == 0) { // At top edge
        block[local_coords.x+1][0] = imageLoad(img, pixel_coords + ivec2(0,-1));
    }
    if (local_coords.y == GROUP_SIZE-1) { // At bottom edge
        block[local_coords.x+1][GROUP_SIZE+1] = imageLoad(img, pixel_coords + ivec2(0,1));
    }
    if (local_coords.x == 0 && local_coords.y == 0) { // At top left corner
        block[0][0] = imageLoad(img, pixel_coords + ivec2(-1,-1));
    }
    if (local_coords.x == GROUP_SIZE-1 && local_coords.y == 0) { // At top right corner
        block[GROUP_SIZE+1][0] = imageLoad(img, pixel_coords + ivec2(1,-1));
    }
    if (local_coords.x == 0 && local_coords.y == GROUP_SIZE-1) { // At bottom left corner
        block[0][GROUP_SIZE+1] = imageLoad(img, pixel_coords + ivec2(-1,1));
    }
    if (local_coords.x == GROUP_SIZE-1 && local_coords.y == GROUP_SIZE-1) { // At bottom right corner
        block[GROUP_SIZE+1][GROUP_SIZE+1] = imageLoad(img, pixel_coords + ivec2(1,1));
    }

    // Synchronize threads to ensure all pixels are loaded into shared memory
    barrier();

    // Calculate average of current pixel, above pixel, below pixel, left pixel, and right pixel
	// Need to add 1 to local_coords to account for the extra pixels loaded into shared memory
    vec4 newPixel = vec4(0.0f);
	newPixel += block[local_coords.x+1][local_coords.y+1];
	newPixel += block[local_coords.x+1+1][local_coords.y+1];
	newPixel += block[local_coords.x+1-1][local_coords.y+1];
	newPixel += block[local_coords.x+1][local_coords.y+1+1];
	newPixel += block[local_coords.x+1][local_coords.y+1-1];
    newPixel /= 5.0f;

    // Calculate original and new pixel values based on diffuse and fade uniforms
    float original = 1.0f - diffuse - fade;
    float new = diffuse;
    newPixel = (imageLoad(img, pixel_coords)*original + newPixel*new);

    // Store new pixel value back into image
    imageStore(img, pixel_coords, newPixel);
}