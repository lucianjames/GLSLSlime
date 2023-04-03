#version 460 core

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img;
layout(location = 3) uniform int size;
layout(location = 4) uniform float sensorDistance;
layout(location = 5) uniform float sensorAngle;
layout(location = 6) uniform float turnSpeed;

layout (std140, binding=2) buffer agentData{
    vec3 data[];
};

vec4 getPixel(float angle, float dist, uint agentID){
    vec2 location = vec2(data[agentID].x, data[agentID].y) + vec2(cos(angle), sin(angle))*dist;
	ivec2 intLoc = ivec2(int(location[0]), int(location[1]));
	if (intLoc[0] >= size){ intLoc[0] -= size-1; }
	if (intLoc[0] <= 0){ intLoc [0] += size+1; }	
	if (intLoc[1] >= size){ intLoc[1] -= size-1; }
	if (intLoc[1] <= 0){ intLoc[1] += size+1; }
	return imageLoad(img, intLoc);
}

void loopBounds(inout vec2 pos){
	if(pos[0] >= size){ pos[0] -= size; }
	if(pos[1] >= size){ pos[1] -= size; }
	if(pos[0] <= 0){ pos[0] += size; }
	if(pos[1] <= 0){ pos[1] += size; }
}

void main(){
    uint agentID = gl_GlobalInvocationID.x;
    float leftSensor = getPixel(data[agentID].z+sensorAngle, sensorDistance, agentID).w; // Uses alpha channel
    float rightSensor = getPixel(data[agentID].z-sensorAngle, sensorDistance, agentID).w;
    data[agentID].z += leftSensor*turnSpeed;
    data[agentID].z -= rightSensor*turnSpeed;

    // Update location of agent
    vec2 direction = vec2(cos(data[agentID].z), sin(data[agentID].z));
    vec2 newpos = vec2(data[agentID].x, data[agentID].y) + (direction);

    loopBounds(newpos);

    // Set agent position
    data[agentID].x = newpos[0];
    data[agentID].y = newpos[1];

    // Draw a pixel at the agents location
    
    imageStore(img, ivec2(int(data[agentID].x), int(data[agentID].y)), vec4(0.0f, 1.0f, (direction[1]+1)/2, 1.0f));
}