#version 450
precision highp float;

layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform PassData {
    mat4 cameraOrientation;
    vec2 screenSize;
    float time;
};



#define RAY_STEPS 150
#define EPSILON 0.001

#define RAYTRACE 0

struct Hit_Info
{
    vec3 pos;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 tex_coord;
    float dist;
    bool is_hit;
};

struct Material
{
    vec3 diffuse_color;
    vec3 emission_color;
};



struct Camera
{
    vec3 position;
    mat3 orientation;
};  




Camera camera;


mat3 rotation_matrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s, 
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s, 
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c          
    );
}


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

#define PI 3.14159265358979


float hash(vec3 p)  // replace this by something better
{
    p  = fract( p*0.3183099+.1 );
	p *= 17.0;
    return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

float smoothmix(float a, float b, float t)
{
    return mix(a, b, -cos(t * PI) / 2. + .5);
}

float vnoise(vec3 p)
{
    vec3 fp = floor(p);

    float f000 = hash(fp + vec3(0, 0, 0));
    float f100 = hash(fp + vec3(1, 0, 0));
    float f010 = hash(fp + vec3(0, 1, 0));
    float f110 = hash(fp + vec3(1, 1, 0));
    float f001 = hash(fp + vec3(0, 0, 1));
    float f101 = hash(fp + vec3(1, 0, 1));
    float f011 = hash(fp + vec3(0, 1, 1));
    float f111 = hash(fp + vec3(1, 1, 1));

    vec3 fr = fract(p);
    
    float l0 = smoothmix(
        smoothmix(f000, f100, fr.x),
        smoothmix(f010, f110, fr.x),
        fr.y);
    float l1 = smoothmix(
        smoothmix(f001, f101, fr.x),
        smoothmix(f011, f111, fr.x),
        fr.y);
    return smoothmix(l0, l1, fr.z);
}


float stepper(float x, float span, float move_span, float step_size)
{
    return (min(mod(x, span), move_span) / move_span + floor(x / span)) * step_size;
}


float weight_map(vec3 p)
{
    vec3 down_step = vec3(0, stepper(time, 5.0, 1.0, 1.0), 0);
    float weight = (vnoise(p / 10. + vec3(-10.231, -32.453, -18.123) + down_step.yyy) + vnoise(p / 10. + down_step)) / 2.0 - 0.7;
    weight += max(p.y - 5., 0.) / 10.; // ceiling
    weight += max(-20. - p.y, 0.) / 10.; // floor;
    return weight;
}

// vec3 weight_gradient(vec3 p)
// {
//     vec2 e = vec2(0, EPSILON);
//     return (vec3(
//         (weight_map(p - e.yxx)) - (weight_map(p + e.yxx)),
//         (weight_map(p - e.xyx)) - (weight_map(p + e.xyx)),
//         (weight_map(p - e.xxy)) - (weight_map(p + e.xxy))
//     )) / EPSILON;
// }

bool is_voxel(vec3 p)
{
    p = floor(p);
    
    // float bottom_hill_height = sin(p.x / 10.) * cos(p.z / 10.) * 10.;
    // float top_hill_height = bottom_hill_height + 10.;


    float weight = weight_map(p);
    
    bool ishit = weight > 0.;

    if (distance(camera.position, p) < 3.)
        ishit = false;

    return ishit;
}

vec3 get_normal(vec3 p)
{
    vec2 e = vec2(0, EPSILON);
    return normalize(vec3(
        (is_voxel(p - e.yxx) ? 1 : 0) - (is_voxel(p + e.yxx) ? 1 : 0),
        (is_voxel(p - e.xyx) ? 1 : 0) - (is_voxel(p + e.xyx) ? 1 : 0),
        (is_voxel(p - e.xxy) ? 1 : 0) - (is_voxel(p + e.xxy) ? 1 : 0)
    ));
}

vec3 get_tangent(vec3 n)
{
    vec3 an = abs(n); //an = absolute normal
    bool isx = an.x > an.y && an.x > an.z;
    bool isy = an.y > an.x && an.y > an.z;
    bool isz = an.z > an.x && an.z > an.y;
    if (isx)
    {
        if (n.x > 0.) 
            return vec3(0, 0, 1);
        else
            return vec3(0, 0, -1);
    }
    else if (isy)
    {
        if (n.y > 0.)
            return vec3(1, 0, 0);
        else
            return vec3(-1, 0, 0);        
    }
    else if (isz)
    {
        if (n.z > 0.)
            return vec3(0, 1, 0);
        else
            return vec3(0, -1, 0);
    }
}

Hit_Info cast_ray(vec3 o, vec3 d)
{
    float t = 0.;
    for (int i = 0; i < RAY_STEPS; i++) {
        vec3 p = o + d * t;
        
        if (is_voxel(p))
            break;
        
        vec3 deltas = (step(0., d) - fract(p)) / d;
        t += max(min(min(deltas.x, deltas.y), deltas.z), EPSILON);
    }

    Hit_Info hit;
    hit.dist = t;
    hit.pos = o + d * t;
    hit.is_hit = is_voxel(hit.pos);
    hit.normal = get_normal(hit.pos);
    hit.tangent = get_tangent(hit.normal);
    hit.bitangent = cross(abs(hit.tangent), hit.normal);
    hit.tex_coord = hit.pos.xz * hit.normal.y + hit.pos.xy * hit.normal.z + hit.pos.yz * hit.normal.x;

    return hit;
}


float edge_func(vec3 hit_pos, vec3 tangent)
{
    float tanDir = dot(fract(hit_pos), tangent);
    tanDir += step(tanDir, 0.);
    tanDir = tanDir * 2. - 1.;
    if (tanDir < 0.)
        tanDir = 0.;
    return tanDir;
}

Material render(Hit_Info hit)
{

    vec2 ft = abs(fract(hit.tex_coord) * 2. - 1.);
    float edgyness = 0.;//smoothstep(0.2, 0.1, 1. - max(ft.x, ft.y));
    float gridiness = 0.;
    vec3 blockPos = floor(hit.pos);

    float emission = step(vnoise(blockPos / 5.), 0.2);

//vec3(0.2, 0.8, 1.);//
    vec3 edgeColor = vec3(1,0.8,0);//vec3(0.08);
    vec3 innerColor = vec3(0.3);

    // col = vec3(emission) + edgyness; //edgyness * vec3(1) + (1. - edgyness) * vec3(0);

    // innerColor = vec3(hash(blockPos),hash(blockPos+.4),hash(blockPos-.4));

    {
        // edge detect
        
        // transcript
        // stav: lol im still here
        // sean: lol nice 
        // stav: ill be back at 8
        // sean: alright


        float edgeDist = 0.0;
        float rawTanDir0 = edge_func(hit.pos, hit.tangent);
        float rawTanDir1 = edge_func(hit.pos, -hit.tangent);
        float rawTanDir2 = edge_func(hit.pos, hit.bitangent);
        float rawTanDir3 = edge_func(hit.pos, -hit.bitangent);
        
        float tanDir0 = is_voxel(blockPos + hit.tangent) ? 0. : rawTanDir0;
        float tanDir1 = is_voxel(blockPos + -hit.tangent) ? 0. : rawTanDir1;
        float tanDir2 = is_voxel(blockPos + hit.bitangent) ? 0. : rawTanDir2;
        float tanDir3 = is_voxel(blockPos + -hit.bitangent) ? 0. : rawTanDir3;
        
        // float bitanDir = (dot(fract(hit.pos), hit.bitangent)) * 2. - 1.;
        // bitanDir += step(bitanDir, 0.);

        // edgeDist = min(tanDir, bitanDir);

        edgeDist = max(max(tanDir0, tanDir1), max(tanDir2, tanDir3));
        float gridDist = max(max(rawTanDir0, rawTanDir1), max(rawTanDir2, rawTanDir3));

        edgyness = smoothstep(0.6, 1.0, edgeDist);
        gridiness = step(0.9, gridDist);
    }



    Material material;
     material.diffuse_color = (1. - emission) * mix(innerColor + gridiness * .1, edgeColor, edgyness);
    material.emission_color = emission * vec3(3) + 0.5 * mix(vec3(0), edgeColor, edgyness);//* (length(fract(hit.pos) - 0.5));
    //material.emission_color = vec3(hit.normal);
    return material;
}

vec3 hemisphere(vec3 rng, vec3 normal)
{
    rng = normalize(rng);
    normal = normalize(normal);
    float v = dot(normal, rng);
    rng -= v * normal;
    rng += abs(v) * normal;
    return rng;
}


Camera get_camera(float time)
{
    Camera camera_new;

    vec3 linear_pos = vec3(time * 5.);

    camera_new.position = vec3(-100, 0, 0);
    vec2 noi = vec2(vnoise(linear_pos / 40. ), vnoise(linear_pos / 40. + vec3(100.)));
    vec2 noi_rot = vec2(vnoise(linear_pos / 20. + vec3(200.)), vnoise(linear_pos / 20. + vec3(300.)));
    
    mat3 camera_orientation = rotation_matrix(vec3(0, 1, 0), (noi_rot.y * 2. - 1.) / 2.5) * rotation_matrix(vec3(1, 0, 0), (noi_rot.x * 2. - 1.) / 2.5);

    mat3 camera_orbit = rotation_matrix(vec3(0, 1, 0), time / 10.);

    camera_orientation *= camera_orbit;

    camera_new.position *= camera_orbit;
    camera_new.position.xyz += vec3(noi * 30. - vec2(0, 25), 0) * camera_orientation;

    camera_new.orientation = camera_orientation;

    // for (int i = 0; i < 4; i++)
    //     camera_new.position += weight_gradient(camera_new.position) * max(5. + weight_map(camera_new.position), 0.);

    return camera_new;
}

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



    // vec2 uv = gl_FragCoord.xy / iResolution.xy;

    // camera = get_camera(iTime);

    // vec3 d = vec3(uv * 2. - 1., 1);
    // vec3 barrel_dir = vec3(sin(d.x * PI / 2.0), sin(d.y * PI / 2.0), cos(d.x * PI / 2.0) * cos(d.y * PI / 2.0));

    // d = refract(mix(barrel_dir, d, 0.8), -d, 1.500);

    // d.x*=iResolution.x / iResolution.y;
    // d=normalize(d);
    
    // d *= camera.orientation;
    // d *= ;

    // uv           => 0 -> 1
    // uv * 2       => 0 -> 2
    // uv * 2 - 1   => -1 -> 1

    Hit_Info hit = cast_ray(origin, dir); // camera.position, d);

    vec3 col = vec3(0);

    if (hit.is_hit)
    {
       
        Material material = render(hit);

        // mix(0.5, 1., clamp(weight_map(hit.pos) * 10., 0., 1.)) * vec3(1);//

        vec3 ambient_light = vec3(0.0);
        float samples = 0.0;
#if RAYTRACE
        for (int i = 0; i < 1; i++)
        {
            vec3 rand_dir = random3(vec3(uv, (i) % 16));
            vec3 hemi = hemisphere(rand_dir, hit.normal);
            
            Hit_Info bounce = cast_ray(hit.pos + hit.normal * 0.1, hemi);
            Material bounce_material = render(bounce);
            ambient_light += bounce_material.emission_color;
            samples++;
        }
#else
        for (int i = 0; i < 5; i++)
        {
            
            vec3 rand_dir = random3(vec3(uv, (i) % 16));
            vec3 hemi = hemisphere(rand_dir, hit.normal) * 0.5;
            ambient_light += float(!is_voxel(hit.pos + hemi)) * .5;
            samples++;
        }
#endif    
        if (samples == 0.0)
            ambient_light = vec3(0.5);
        ambient_light /= max(samples, 1.);

        col = ambient_light * material.diffuse_color + material.emission_color;
        col = mix(col, vec3(0), smoothstep(30.0, 70.0, hit.dist));
    }

    color = vec4(col, 1);

     
    // color.xyz = origin;
}
