#version 450
precision highp float;

layout(binding = 0) uniform sampler2D inIndirect;
layout(binding = 1) uniform sampler2D inNormal;

layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform PassData {
    mat4 cameraOrientation;
    mat4 lastCameraOrientation;
    vec2 screenSize;
    vec2 blurDir;
    float time; 
    
};
#define BLUR 13
#define SIGMA 0.7
#define PI 3.1415926

void main() {
    vec3 origin = (vec4(0, 0, 0, 1) * cameraOrientation).xyz;
    vec3 lastOrigin = (vec4(0, 0, 0, 1) * lastCameraOrientation).xyz;

    vec2 uv = gl_FragCoord.xy / screenSize;
	uv.y = 1.0 - uv.y;

    vec4 indirect = vec4(0);
    float samples = 0.0;

    vec3 normal = texture(inNormal, uv).xyz;
    vec4 middleIndirect = texture(inNormal, uv);

    for (int j = -BLUR; j < BLUR; j++) {
        vec2 pix = blurDir * j;
        vec2 ourUv = uv + pix / screenSize;
        vec3 ourNormal = texture(inNormal, ourUv).xyz;
        //float w = max(dot(ourNormal, normal), 0);

        vec4 ourIndirect = texture(inIndirect, ourUv);

        float wg = exp( -(float(j * j) / float(BLUR * BLUR)) / (2. * SIGMA * SIGMA) ) / sqrt(2. * PI * SIGMA);
        float wn = max(pow(dot(ourNormal, normal), 5.), 0.);
        float wd = max(1. - abs(ourIndirect.a - middleIndirect.a) / 7.0, 0.);
        

        indirect += ourIndirect * wn * wg * wd;
        samples += wn * wg * wd;
    }

    if (samples != 0)
        indirect /= samples;

    color.xyz = indirect.xyz;
	color.a = indirect.a;
}
