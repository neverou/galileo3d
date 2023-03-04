#version 450
precision highp float;

layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform PassData {
    mat4 cameraOrientation;
    vec2 screenSize;
    float time;
};


void main() {
    vec3 origin = (vec4(0, 0, 0, 1) * cameraOrientation).xyz;
    
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec3 front = (vec4(0, 0, 1, 0) * cameraOrientation).xyz;
    vec3 right = (vec4(1, 0, 0, 0) * cameraOrientation).xyz;
    vec3 up    = (vec4(0, 1, 0, 0) * cameraOrientation).xyz;

    vec3 raw_dir = vec3(uv * 2 - 1, 1);
    raw_dir.x *= screenSize.x / screenSize.y;
    raw_dir = normalize(raw_dir);

    vec3 dir = raw_dir.x * right + raw_dir.y * up + raw_dir.z * front;

     
    // color.xyz = origin;
}
