#version 150 core

out vec4 outColor;

in vec3 n;
in vec3 color;
in vec3 pos;

uniform samplerBuffer tria;
uniform samplerBuffer tria2;
uniform int tbo_size;
uniform int tbo_size2;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;

float epsilon = 0.000001;

struct Ray{
    vec3 ray_origin;
    vec3 ray_dir;
};

struct Intersection{
    vec3 position;
    vec3 normal;
    vec3 color;
    float d; // distance
    bool is_intersecting;
    bool is_reflecting;
    bool is_light;
    int triangle_id;
};

struct Triangle{
    vec3 v1;
    vec3 v2;
    vec3 v3; // vectices of the triangle
    vec3 n1;
    vec3 n2;
    vec3 n3; // normals of the triangle
    vec3 color;
    bool is_reflecting;
    bool is_light;
    int id;
};

Triangle get_triangle(int i){
    Triangle triangle;
    triangle.v1 = texelFetch(tria, 8 * i).rgb;
    triangle.v2 = texelFetch(tria, 8 * i + 1).rgb;
    triangle.v3 = texelFetch(tria, 8 * i + 2).rgb;
    triangle.n1 = texelFetch(tria, 8 * i + 3).rgb;
    triangle.n2 = texelFetch(tria, 8 * i + 4).rgb;
    triangle.n3 = texelFetch(tria, 8 * i + 5).rgb;
    triangle.color = texelFetch(tria, 8 * i + 6).rgb;
    vec3 temp = texelFetch(tria,8 * i + 7).rgb;
    triangle.is_reflecting = temp.r >= 1;
    triangle.is_light = temp.g >= 1;
    triangle.id = i;
    return triangle;
}

// ray triangle intersection
Intersection intersect(Ray ray, Triangle triangle){
    Intersection inter;
    // initialize a non-intersection
    inter.is_intersecting = false;
    inter.is_reflecting = false;
    inter.is_light = false;
    inter.color = vec3(0);
    inter.d = -1;
    inter.triangle_id = -1;
    vec3 n = normalize(cross(triangle.v1 - triangle.v2, triangle.v2 - triangle.v3));
    float perpen = dot(ray.ray_dir, n);
    if(perpen == 0) 
        return inter;
    float t = dot((triangle.v1 - ray.ray_origin), n) / perpen;

    if(t < 0)
        return inter;

    vec3 intersection_point = ray.ray_origin + ray.ray_dir * t;
    vec3 v0 = triangle.v3 - triangle.v1;
    vec3 v1 = triangle.v2 - triangle.v1;
    vec3 v2 = intersection_point - triangle.v1;

    float dot_00 = dot(v0, v0);
    float dot_01 = dot(v0, v1);
    float dot_02 = dot(v0, v2);
    float dot_11 = dot(v1, v1);
    float dot_12 = dot(v1, v2);

    float inverse_denom = 1.0 / (dot_00 * dot_11 - dot_01 * dot_01);
    float u = (dot_11 * dot_02 - dot_01 * dot_12) * inverse_denom;
    float v = (dot_00 * dot_12 - dot_01 * dot_02) * inverse_denom;

    if ((u >= 0) && (v >= 0) && (u + v < 1)){
        if(perpen>0)   
            inter.normal = -n;
        else
            inter.normal = n;
        inter.position = intersection_point;
        inter.color = triangle.color;
        inter.is_intersecting = true;
        inter.is_reflecting = triangle.is_reflecting;
        inter.is_light = triangle.is_light;
        inter.d = t;
        inter.triangle_id = triangle.id;
        return inter; 
    } 
    else 
        return inter;
}


vec3 Phong(vec3 color, vec3 normal, vec3 light_pos, vec3 pos, vec3 cam_pos, vec3 light_para){
    normal = normalize(normal);
    vec3 light_dir = normalize(light_pos - pos);
    return clamp( color * light_para.x + 
            color * max(0.0, dot(normal, light_dir)) + 
            vec3(1.0) * pow(max(0.0, dot( normalize(cam_pos - pos), normalize( reflect(-light_dir, normal)))), light_para.y),
            0.0, 1.0);
}

bool shadow(Intersection inter){
    vec3 position = inter.position;
    int id = inter.triangle_id;
    Ray ray = Ray(position, normalize(lightPos - pos));
    for(int j = 0; j < tbo_size; ++j){
        // generate the triangle
        Triangle triangle = get_triangle(j);
        Intersection temp_inter = intersect(ray, triangle);
        //if(temp_inter.is_intersecting && temp_inter.d > 0 && temp_inter.d < length(light_pos - position) && id != triangle.id)
        if(temp_inter.is_intersecting && id != triangle.id)
            return true;
    }
    return false;
}

vec3 ray_tracing(){
    int depth = 10;
    vec3 result;
    vec3 pos_temp = camPos;
    vec3 dir_temp = normalize(pos - camPos);
    Intersection inter_buffer[10];
    int current_depth = 0;
    for(int i = 0; i < depth; ++i){
        Ray ray = Ray(pos_temp, dir_temp);
        float min_distance = -1;
        Intersection inter;
        for(int j = 0; j < tbo_size; ++j){
            // generate the triangle
            Triangle triangle = get_triangle(j);
            Intersection temp_inter = intersect(ray, triangle);
            if(temp_inter.is_intersecting && (temp_inter.d < min_distance || min_distance < 0)){
                min_distance = temp_inter.d;
                inter = temp_inter;
            }
        }
        if(!inter.is_intersecting){
            inter.color = vec3(1,0,0);
            break;
        }
        inter_buffer[i] = inter;
        if(!inter.is_reflecting){
            // if(shadow(inter.position, lightPos, inter.triangle_id)){
            //     return clamp(inter.color * lightParams.x, 0.0, 1.0);
            // }
            // else{
            //     return Phong(inter.color, inter.normal, lightPos, inter.position, camPos, lightParams);
            // }
            break;
        }
        else{
            current_depth++;
            pos_temp = inter.position;
            dir_temp = normalize(reflect(dir_temp, inter.normal));
        }
    }
    result = inter_buffer[current_depth].color;
    for(int k = current_depth; k >=0; --k){
        if(shadow(inter_buffer[k])){
            result = clamp(result * lightParams.x, 0.0, 1.0);
        }
        else{
            result =  Phong(result, inter_buffer[k].normal, lightPos, inter_buffer[k].position, camPos, lightParams);
        }
    }
    return result;
}

void main()
{
    vec3 col = ray_tracing();
    outColor = vec4(col, 1.0);
}
