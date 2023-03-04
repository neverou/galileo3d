#version 450
precision highp float;


#define RAY_STEPS 512
#define EPSILON 0.001

#define MAX_BOUNCE_DIST 100
#define MAX_RAY_DIST 300

#define GOLDEN_RATIO 1.618033988

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outDirect;
layout(location = 2) out vec4 outIndirect;
layout(location = 3) out vec4 outNormal;

layout(std140, binding = 0) uniform PassData {
    mat4 cameraOrientation;
    mat4 lastCameraOrientation;
    vec2 screenSize;
    vec2 blurDir;
    float time; 
    
};


struct Voxel {
    vec4 color;
    vec4 emission;
};

layout(std140, binding = 0) buffer WorldData {
    ivec3 voxelSize;
    Voxel voxels[];
};

Voxel GetVoxel(ivec3 pos) {
    if (pos.x < 0 || pos.x >= voxelSize.x || pos.y < 0 || pos.y >= voxelSize.y || pos.z < 0 || pos.z >= voxelSize.z) {
        Voxel v;
        v.color = vec4(0);
        return v;
    }    
    else {
        return voxels[pos.x + pos.y * voxelSize.x + pos.z * voxelSize.x * voxelSize.y];
    }
}


struct HitInfo
{
    vec3 pos;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 texCoord;
    float dist;
    Voxel voxel;
    bool isHit;
};




vec3 random3(vec3 c) {
	float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0*j);
	j *= .125;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}


HitInfo CastRay(vec3 o, vec3 d, float maxDist) {
    float t = 0.;
    vec3 deltas = vec3(0);
    bool lastFilled = GetVoxel(ivec3(o)).color.a > 0;

    for (int i = 0; i < RAY_STEPS; i++) {
        if (t > maxDist) break;

        vec3 p = o + d * t;
        
        Voxel voxel = GetVoxel(ivec3(p));
        bool filled = voxel.color.a > 0;
        if ((filled && !lastFilled) && voxel.color.a > random3(fract(p + time) * GOLDEN_RATIO).x / 2. + .5)
            break;
        lastFilled = filled;

        deltas = (step(0., d) - fract(p)) / d;
        t += max(min(min(deltas.x, deltas.y), deltas.z), EPSILON);
    }


    HitInfo hit;

    hit.dist = t;
    hit.pos = o + d * t;
    hit.voxel = GetVoxel(ivec3(hit.pos));
    hit.isHit = hit.voxel.color.a > 0;

    vec3 absdeltas = abs(deltas);
    if (absdeltas.x < absdeltas.y && absdeltas.x < absdeltas.z)
        hit.normal = vec3(-sign(d.x), 0, 0);
    else if (absdeltas.y < absdeltas.z && absdeltas.y < absdeltas.x)
        hit.normal = vec3(0, -sign(d.y), 0);
    else
        hit.normal = vec3(0, 0, -sign(d.z));

    return hit;
}



// TEMP:
#define PI 3.141592
#define iSteps 16
#define jSteps 8

vec2 rsi(vec3 r0, vec3 rd, float sr) {
    // ray-sphere intersection that assumes
    // the sphere is centered at the origin.
    // No intersection when result.x > result.y
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b*b) - 4.0*a*c;
    if (d < 0.0) return vec2(1e5,-1e5);
    return vec2(
        (-b - sqrt(d))/(2.0*a),
        (-b + sqrt(d))/(2.0*a)
    );
}

vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g) {
    // Normalize the sun and view directions.
    pSun = normalize(pSun);
    r = normalize(r);

    // Calculate the step size of the primary ray.
    vec2 p = rsi(r0, r, rAtmos);
    if (p.x > p.y) return vec3(0,0,0);
    p.y = min(p.y, rsi(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(iSteps);

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = vec3(0,0,0);
    vec3 totalMie = vec3(0,0,0);

    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

    // Sample the primary ray.
    for (int i = 0; i < iSteps; i++) {

        // Calculate the primary ray sample position.
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);

        // Calculate the height of the sample.
        float iHeight = length(iPos) - rPlanet;

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // Calculate the step size of the secondary ray.
        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) {

            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);

            // Calculate the height of the sample.
            float jHeight = length(jPos) - rPlanet;

            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }

        // Calculate attenuation.
        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

        // Accumulate scattering.
        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;

        // Increment the primary ray time.
        iTime += iStepSize;

    }

    // Calculate and return the final color.
    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}

vec3 sky(vec3 d) {
    //return vec3(.5, .6, .7);
    float t = time / 5.0;
    vec3 col = atmosphere(
        d,           // normalized ray direction
        vec3(0,6372e3,0),               // ray origin
        vec3(cos(t), 1, sin(t)) * 10,                        // position of the sun
        64.0, // 22.0 (default)                           // intensity of the sun
        6371e3,                         // radius of the planet in meters
        6471e3,                         // radius of the atmosphere in meters
        vec3(5.5e-6, 13.0e-6, 22.4e-6), // Rayleigh scattering coefficient
        21e-6,                          // Mie scattering coefficient
        8e3,                            // Rayleigh scale height
        1.2e3,                          // Mie scale height
        0.758                           // Mie preferred scattering direction
    );

    return col;
}

//

void main() {
    vec3 origin = (vec4(0, 0, 0, 1) * cameraOrientation).xyz;
    
    vec2 uv = gl_FragCoord.xy / screenSize;
	uv.y = 1.0 - uv.y;

    vec3 front = (vec4(0, 0, 1, 0) * cameraOrientation).xyz;
    vec3 right = (vec4(1, 0, 0, 0) * cameraOrientation).xyz;
    vec3 up    = (vec4(0, 1, 0, 0) * cameraOrientation).xyz;
  
    vec3 raw_dir = vec3(uv * 2 - 1, 1);
    raw_dir.x *= screenSize.x / screenSize.y;
    raw_dir = normalize(raw_dir);

    vec3 dir = raw_dir.x * right + raw_dir.y * up + raw_dir.z * front;

    HitInfo hit = CastRay(origin, dir, MAX_RAY_DIST);
    
    outColor = vec4(sky(dir), 0);
    outNormal = vec4(0);
    if (hit.isHit) {
        vec3 lightDir = normalize(vec3(cos(time / 5), 1, sin(time / 5)));
        vec3 lightColor = vec3(1.5, 1.2, 1);
        float lightIntensity = 7.0;
        // vec3 albedo =   hit.voxel.color.xyz; //hit.normal;//
        // float light = max(dot(hit.normal, lightDir), 0.1);
        
        // HitInfo shadow = CastRay(hit.pos + hit.normal * EPSILON, lightDir, MAX_RAY_DIST);
        // if (shadow.isHit)
        //     light = 0.1;

        // vec3 ambientLight = vec3(0);
        // float ambientSamples = 0.0;

        // for (int i = 0; i < 2; i++) {
        //     vec3 randDir = normalize(random3(vec3(uv, fract((time + i) * GOLDEN_RATIO))));
        //     HitInfo skyRay = CastRay(hit.pos + hit.normal * EPSILON, randDir, MAX_BOUNCE_DIST);
        //     if (skyRay.isHit) {
        //         ambientLight += skyRay.voxel.emission.xyz;
        //     }
        //     else {
        //         ambientLight += sky(randDir);
        //     }
        //     ambientSamples++;
        // }
        // ambientLight /= ambientSamples / 2.0;

        vec3 albedo = hit.voxel.color.xyz; //hit.normal;//

        vec3 light = lightColor * lightIntensity * max(dot(hit.normal, lightDir), 0.0);
        
        HitInfo shadow = CastRay(hit.pos + hit.normal * EPSILON, lightDir, MAX_RAY_DIST);
        if (shadow.isHit)
            light = vec3(0);

        vec3 indirectLight = vec3(0);
        float indirectSamples = 0.0;

        for (int i = 0; i < 1; i++) {
            vec3 randDir = normalize(random3(vec3(uv, fract((time + i) * GOLDEN_RATIO))));
            if (dot(randDir, hit.normal) < 0) randDir *= -1;
            HitInfo giRay = CastRay(hit.pos + hit.normal * EPSILON, randDir, MAX_BOUNCE_DIST);
            if (giRay.isHit) {
                vec3 ourAlbedo = giRay.voxel.color.xyz;
                vec3 ourLight = lightColor * lightIntensity * max(dot(giRay.normal, lightDir), 0.0);
                HitInfo shadow = CastRay(giRay.pos + giRay.normal * EPSILON, lightDir, MAX_RAY_DIST);
                if (shadow.isHit)
                    ourLight = vec3(0);
                indirectLight += ourAlbedo * ourLight + giRay.voxel.emission.xyz;
            }
            else {
                indirectLight += sky(randDir);
            }
            indirectSamples++;
        }
        indirectLight /= indirectSamples;

        outColor = vec4(albedo, hit.dist);
        outIndirect = vec4(indirectLight, hit.dist);
        outDirect = vec4(light + hit.voxel.emission.xyz, hit.dist);
        outNormal = vec4(hit.normal, hit.dist);
    }


    // for (int i = 0; i < 256; i++) {
    //     p += dir * 0.25;
        
    //     Voxel v = GetVoxel(ivec3(floor(p)));
    //     if (v.color.a > 0) {
    //         color = v.color * 5. / length(p - origin);
    //         break;
    //     }   
    // }
}
