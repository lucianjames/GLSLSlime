#version 460 core
out vec4 FragColor;
in vec2 v_texCoord;

uniform sampler2D textureSampler;

void main(){
    FragColor = texture(textureSampler, v_texCoord);
}
