#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 v_texCoord;

uniform float textureRatio;
uniform float offsetX;
uniform float offsetY;
uniform float zoomMultiplier;

void main(){
    gl_Position = vec4(position, 1.0f);
    v_texCoord = (texCoord + vec2(offsetX, offsetY) - 0.5f) * zoomMultiplier + 0.5f;
    v_texCoord.x *= textureRatio;
}
