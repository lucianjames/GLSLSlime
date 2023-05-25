#version 460 core

#define GROUP_SIZE 32

layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE) in;
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

// Loops the position around to the other side of the texture if it goes out of bounds
void loopBounds(inout vec2 pos){
    if(pos[0] >= size){ pos[0] -= size; }
    if(pos[1] >= size){ pos[1] -= size; }
    if(pos[0] <= 0){ pos[0] += size; }
    if(pos[1] <= 0){ pos[1] += size; }
}

// Returns the pixel coordinates of a pixel at a certain angle and distance from the agent at agentID
ivec2 getPixelCoords(float angle, float dist, uint agentID){
    vec2 location = vec2(data[agentID].x, data[agentID].y) + vec2(cos(angle), sin(angle))*dist;
    loopBounds(location);
    return ivec2(int(location[0]), int(location[1]));
}

void main(){
    // Get agent variables
    uint agentID = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y*gl_NumWorkGroups.x*gl_WorkGroupSize.x;
    ivec2 pixelCoords_left = getPixelCoords(data[agentID].z+sensorAngle, sensorDistance, agentID);
    ivec2 pixelCoords_right = getPixelCoords(data[agentID].z-sensorAngle, sensorDistance, agentID);
    float leftSensor = imageLoad(img, pixelCoords_left).w; // Uses alpha channel
    float rightSensor = imageLoad(img, pixelCoords_right).w;
    if(drawSensors == 1){
        imageStore(img, pixelCoords_left, vec4(sensorColour, leftSensor));
        imageStore(img, pixelCoords_right, vec4(sensorColour, rightSensor));
    }

    // Update angle of agent
    data[agentID].z += leftSensor*turnSpeed - rightSensor*turnSpeed;
    data[agentID].z = mod(data[agentID].z, 6.28318530718f); // Ensure angle doesnt go up and up until floating point errors cause problems

    // Update location of agent
    vec2 direction = vec2(cos(data[agentID].z), sin(data[agentID].z))*speed;
    vec2 newpos = vec2(data[agentID].x, data[agentID].y) + (direction);
    loopBounds(newpos);

    // Set agent position
    data[agentID].x = newpos[0];
    data[agentID].y = newpos[1];

    // Draw a pixel at the agents location
    vec3 colour = ((((direction.x/speed)+1)*agentXDirectionColour + // Multiply the X direction by the X direction colour
                  (((direction.y/speed)+1)*agentYDirectionColour) +
                  mainAgentColour)) // Add in the "main" agent colour to the mix 
                  /1.5f;

    imageStore(img, ivec2(int(data[agentID].x), int(data[agentID].y)), vec4(colour, 1.0f));
}
