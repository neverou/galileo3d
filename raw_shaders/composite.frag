#version 450
precision highp float;

layout(binding = 0) uniform sampler2D inImage;
layout(binding = 1) uniform sampler2D inDirect;
layout(binding = 2) uniform sampler2D inIndirect;
layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform PassData {
    mat4 cameraOrientation;
    mat4 lastCameraOrientation;
    vec2 screenSize;
    vec2 blurDir;
    float time; 
    
};

vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0, 1);
}

void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;

	uv.y = 1.0 - uv.y;
    color = texture(inImage, uv);
    
    // if the depth is not 0 we hit something
    if (color.a != 0.0) color *= (texture(inDirect, uv) + texture(inIndirect, uv));

    color.xyz = ACESFilm(color.xyz);

}
