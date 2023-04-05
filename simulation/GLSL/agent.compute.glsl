#version 460 core

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img;
uniform int size;
uniform float sensorDistance;
uniform float sensorAngle;
uniform float turnSpeed;
uniform float speed;
uniform int drawSensors;
uniform vec3 sensorColour;
uniform vec3 mainAgentColour;
uniform vec3 agentYDirectionColour;
uniform vec3 agentXDirectionColour;

layout (std140, binding=2) buffer agentData{
    vec3 data[];
    // x = x position
    // y = y position
    // z = angle
    // Also pad the data to make it a vec4 when you send it to this shader
};

ivec2 getPixelCoords(float angle, float dist, uint agentID){
    vec2 location = vec2(data[agentID].x, data[agentID].y) + vec2(cos(angle), sin(angle))*dist;
	ivec2 intLoc = ivec2(int(location[0]), int(location[1]));
	if (intLoc[0] >= size){ intLoc[0] -= size-1; }
	if (intLoc[0] <= 0){ intLoc [0] += size+1; }	
	if (intLoc[1] >= size){ intLoc[1] -= size-1; }
	if (intLoc[1] <= 0){ intLoc[1] += size+1; }
    return intLoc;
}

vec4 getPixel(float angle, float dist, uint agentID){
	ivec2 intLoc = getPixelCoords(angle, dist, agentID);
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

    float leftSensor;
    float rightSensor;
    if(drawSensors == 1){
        ivec2 pixelCoords_left = getPixelCoords(data[agentID].z+sensorAngle, sensorDistance, agentID);
        ivec2 pixelCoords_right = getPixelCoords(data[agentID].z-sensorAngle, sensorDistance, agentID);
        leftSensor = imageLoad(img, pixelCoords_left).w; // Uses alpha channel
        rightSensor = imageLoad(img, pixelCoords_right).w;
        imageStore(img, pixelCoords_left, vec4(sensorColour, leftSensor));
        imageStore(img, pixelCoords_right, vec4(sensorColour, rightSensor));
    }else{
        rightSensor = getPixel(data[agentID].z-sensorAngle, sensorDistance, agentID).w;
        leftSensor = getPixel(data[agentID].z+sensorAngle, sensorDistance, agentID).w; // Uses alpha channel
    }

    data[agentID].z += leftSensor*turnSpeed;
    data[agentID].z -= rightSensor*turnSpeed;
    // I should probably make it so that the angle gets looped around to ensure no floating point problems

    // Update location of agent
    vec2 direction = vec2(cos(data[agentID].z), sin(data[agentID].z))*speed;
    vec2 newpos = vec2(data[agentID].x, data[agentID].y) + (direction);

    loopBounds(newpos);

    // Set agent position
    data[agentID].x = newpos[0];
    data[agentID].y = newpos[1];

    // Draw a pixel at the agents location
    vec3 colour = ((((direction.x/speed)+1)*agentXDirectionColour + (((direction.y/speed)+1)*agentYDirectionColour) + mainAgentColour))/1.5f;
    imageStore(img, ivec2(int(data[agentID].x), int(data[agentID].y)), vec4(colour, 1.0f));
}