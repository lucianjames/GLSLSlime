#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 v_texCoord;

uniform float textureRatio;
uniform float offsetX;
uniform float offsetY;
uniform float zoomMultiplier;

void main(){
    vec2 offset = vec2(offsetX, offsetY);
    gl_Position = vec4(position, 1.0f);
    v_texCoord = ((texCoord - 0.5f) * zoomMultiplier) + 0.5f - offset;
    v_texCoord.x *= textureRatio;
}
