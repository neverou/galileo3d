#version 450
precision highp float;

layout(binding = 0) uniform sampler2D inImage;
layout(binding = 1) uniform sampler2D inLastImage;

layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform PassData {
    mat4 cameraOrientation;
    mat4 lastCameraOrientation;
    vec2 screenSize;
    vec2 blurDir;
    float time; 
    
};

vec3 GetDir(mat4 orientation, vec2 uv) {
    vec3 front = (vec4(0, 0, 1, 0) * orientation).xyz;
    vec3 right = (vec4(1, 0, 0, 0) * orientation).xyz;
    vec3 up    = (vec4(0, 1, 0, 0) * orientation).xyz;

    vec3 raw_dir = vec3(uv * 2 - 1, 1);
    raw_dir.x *= screenSize.x / screenSize.y;
    raw_dir = normalize(raw_dir);
    return raw_dir.x * right + raw_dir.y * up + raw_dir.z * front;
}

void main() {
    vec3 origin = (vec4(0, 0, 0, 1) * cameraOrientation).xyz;
    vec3 lastOrigin = (vec4(0, 0, 0, 1) * lastCameraOrientation).xyz;

    vec2 uv = gl_FragCoord.xy / screenSize;
	uv.y = 1.0 - uv.y;

    vec3 dir = GetDir(cameraOrientation, uv);

	vec4 img = texture(inImage, uv);

    vec3 pos = img.a * dir + origin;
 
    vec3 relativePosToLastCamera = (vec4(pos, 1) * inverse(lastCameraOrientation)).xyz;
    vec2 proj = abs((relativePosToLastCamera).xy / relativePosToLastCamera.z  * vec2(screenSize.y / screenSize.x, 1)  / 2. + .5);
	
   
    float blend = 0.95;


    vec3 lastDir = GetDir(lastCameraOrientation, proj);

	proj.y = 1.0 - proj.y;
	vec4 lastImagePos = texture(inLastImage, proj);
	vec3 lastProjectedPos = lastImagePos.a * lastDir + lastOrigin;

    // history sample is out of screen bounds, discard it
    if (abs(proj.x-0.5)>.5||abs(proj.y-0.5)>.5)
        blend = 0.0;

	if (distance(lastProjectedPos, pos) > 1.)
		blend = 0.0;

    color.xyz = mix(img.xyz, lastImagePos.xyz, blend);
	color.a = img.a;
}
