#version 460

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img;

layout(location = 2) uniform float pixelMultiplier;
layout(location = 3) uniform float newPixelMultiplier;
layout(location = 4) uniform int size;

void main(){
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    
	int xMinus = -1;
	int xPlus = 1;
	int yMinus = -1;
	int yPlus = 1;

	if (pixel_coords[0] >= size-1){ xPlus = -(size-1); }
	else if (pixel_coords[0] <= 0){ xMinus = size-1; }
	if (pixel_coords[1] >= size-1){ yPlus = -(size-1); }
	else if (pixel_coords[1] <= 0){ yMinus = size-1; }

	vec4 pixel = imageLoad(img, pixel_coords);
	vec4 newPixel = vec4(0.0f);
	newPixel += pixel;
	newPixel += imageLoad(img, pixel_coords + ivec2(xPlus,0));
	newPixel += imageLoad(img, pixel_coords + ivec2(xMinus,0));
	newPixel += imageLoad(img, pixel_coords + ivec2(0,yMinus));
	newPixel += imageLoad(img, pixel_coords + ivec2(0,yPlus));
	newPixel /= 5;

	// Change strength of diffusion
	// And slowly fade the image by making sure the sums of the two numbers being used for multiplication is < 1
	newPixel = (pixel*pixelMultiplier + newPixel*newPixelMultiplier);

	imageStore(img, pixel_coords, newPixel);
}
