#version 450
precision highp float;

layout(binding = 0) uniform sampler2D inImage;
layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform PassData {
    mat4 cameraOrientation;
    mat4 lastCameraOrientation;
    vec2 screenSize;
    float time;
};


void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;

    color = texture(inImage, uv);
}
