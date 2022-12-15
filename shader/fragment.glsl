#version 150 core

out vec4 outColor;

in vec3 n;
in vec3 color;
in vec3 pos;

uniform samplerBuffer tria;
uniform samplerBuffer tria2;
uniform int tbo_size;
uniform int tbo_size2;

//uniform vec3 triangleColor;
//uniform vec3 lightPos;
//uniform vec3 lightParams;
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


struct Light{
    float id;
    float Ia;
    float Ii;
    vec3 p1;
    vec3 p2;
    vec3 p3;
    vec3 p4;
    vec3 dir;
};

Light get_light(int i){
// [id Ia Ii] [p1] [p2] [p3] [p4] [dir]
    Light light;
    light.id = texelFetch(tria2, 6 * i).r;
    light.Ia = texelFetch(tria2, 6 * i).g;
    light.Ii = texelFetch(tria2, 6 * i).b;
    light.p1 = texelFetch(tria2, 6 * i + 1).rgb;
    light.p2 = texelFetch(tria2, 6 * i + 2).rgb;
    light.p3 = texelFetch(tria2, 6 * i + 3).rgb;
    light.p4 = texelFetch(tria2, 6 * i + 4).rgb;
    light.dir = texelFetch(tria2, 6 * i + 5).rgb;
    return light;
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




vec3 Phong2(vec3 color, vec3 normal, vec3 position, vec3 cam_pos, Light l){
    normal = normalize(normal);
    vec3 cam_dir = normalize(cam_pos - position);
    vec3 light_dir = normalize(l.p1 - position);
    vec3 ref = normalize( reflect(-light_dir, normal));
    float nl = clamp( dot( normal, light_dir ), 0.0, 1.0);
    float vr = clamp( dot( cam_dir, ref ), 0.0, 1.0);
    return clamp( color * l.Ia + 
            color * nl * l.Ii + 
            l.Ii * vec3(1.0) * pow(vr, 50),
            0.0, 1.0);
}
vec3 Phong3(vec3 color, vec3 normal, vec3 position, vec3 cam_pos, Light l){
    normal = normalize(normal);
    vec3 cam_dir = normalize(cam_pos - position);
    vec3 light_dir, ref;
    float nl, vr;

    switch(int(l.id)){
        case 0: // directional light
            light_dir = -l.dir;
            ref = normalize( reflect(-light_dir, normal));
            nl = clamp( dot( normal, light_dir ), 0.0, 1.0);
            vr = clamp( dot( cam_dir, ref ), 0.0, 1.0);
            return clamp( color * l.Ia + 
                    color * nl * l.Ii + 
                    l.Ii * vec3(1.0) * pow(vr, 50),
                    0.0, 1.0);
            break;
        case 1: // point light
            light_dir = normalize(l.p1 - position);
            ref = normalize( reflect(-light_dir, normal));
            nl = clamp( dot( normal, light_dir ), 0.0, 1.0);
            vr = clamp( dot( cam_dir, ref ), 0.0, 1.0);
            return clamp( color * l.Ia + 
                    color * nl * l.Ii + 
                    l.Ii * vec3(1.0) * pow(vr, 50),
                    0.0, 1.0);
            break;
        case 2: // Spot light
            light_dir = normalize(l.p1 - position);
            ref = normalize( reflect(-light_dir, normal));
            nl = clamp( dot( normal, light_dir ), 0.0, 1.0);
            vr = clamp( dot( cam_dir, ref ), 0.0, 1.0);
            return clamp( color * l.Ia + 
                    color * nl * l.Ii + 
                    l.Ii * vec3(1.0) * pow(vr, 50),
                    0.0, 1.0);
            break;
    
    }


}



vec3 shadow2(Intersection inter, Light l){
    vec3 position = inter.position;
    int id = inter.triangle_id;
    vec3 light_dir;
    Ray ray;
    switch(int(l.id)){
        case 0:
            ray = Ray(position, normalize(-l.dir));
            for(int j = 0; j < tbo_size; ++j){
            // generate the triangle
                Triangle triangle = get_triangle(j);
                Intersection temp_inter = intersect(ray, triangle);
                if(temp_inter.is_intersecting && id != triangle.id){
                    return clamp(inter.color * l.Ia, 0.0, 1.0);
                }
            }
            break;
        case 1:
            ray = Ray(position, normalize(l.p1 - position));
            for(int j = 0; j < tbo_size; ++j){
            // generate the triangle
                Triangle triangle = get_triangle(j);
                Intersection temp_inter = intersect(ray, triangle);
                if(temp_inter.is_intersecting && temp_inter.d > epsilon && temp_inter.d < length(l.p1 - position) && id != triangle.id){
                    return clamp(inter.color * l.Ia, 0.0, 1.0);
                }
            }
            break;
        case 2:
            light_dir = l.dir;
            float angle = radians(l.p2.r);
            vec3 va = light_dir;
            vec3 vb = normalize(position - l.p1);
            float realAngle = acos( length( dot(va, vb) ));
            if(realAngle > angle) { return vec3(0.0); }
            else{
                ray = Ray(position, normalize(l.p1 - position));
                for(int j = 0; j < tbo_size; ++j){
                // generate the triangle
                    Triangle triangle = get_triangle(j);
                    Intersection temp_inter = intersect(ray, triangle);
                    if(temp_inter.is_intersecting && temp_inter.d > epsilon && temp_inter.d < length(l.p1 - position) && id != triangle.id){
                        return clamp(inter.color * l.Ia, 0.0, 1.0);
                    }
                }
            }
            break;
    }

    return vec3(-1.0);
}



vec3 ray_tracing(){
    int depth = 10;
    vec3 currentColor, result = vec3(0.0);
    vec3 pos_temp = camPos;
    vec3 dir_temp = normalize(pos - camPos);
    Intersection inter_buffer[10];
    int current_depth = 0;
    int current_id = -1;
    for(int i = 0; i < depth; ++i){
        Ray ray = Ray(pos_temp, dir_temp);
        float min_distance = -1;
        Intersection inter;
        for(int j = 0; j < tbo_size; ++j){
            // generate the triangle
            Triangle triangle = get_triangle(j);
            Intersection temp_inter = intersect(ray, triangle);
            if(temp_inter.is_intersecting && (temp_inter.d < min_distance || min_distance < 0)
                && current_id != temp_inter.triangle_id){
                min_distance = temp_inter.d;
                inter = temp_inter;
            }
        }
        if(!inter.is_intersecting){
            inter.color = vec3(1,0,0);
            break;
        }
        inter_buffer[i] = inter;
        current_id = inter.triangle_id;
        if(!inter.is_reflecting){
            break;
        }
        else{
            current_depth++;
            pos_temp = inter.position;
            dir_temp = normalize(reflect(dir_temp, inter.normal));
        }
    }
    currentColor = inter_buffer[current_depth].color;
    for(int k = current_depth; k >=0; --k){
        if(inter_buffer[k].is_reflecting){
            result -= vec3(0.03,0.03,0.03);
            continue;
        }
        for(int i = 0; i < tbo_size2; i++){
            Light l = get_light(i);
            vec3 temp;
            switch(int(l.id)){
                case 0:
                    temp = shadow2(inter_buffer[k], l);
                    if(  temp != vec3(-1.0) ){
                        currentColor = temp;
                    }else{
                        currentColor = Phong3(inter_buffer[k].color, inter_buffer[k].normal, inter_buffer[k].position, camPos, l);
                    }
                    break;
                case 1:
                    temp = shadow2(inter_buffer[k], l);
                    if(  temp != vec3(-1.0) ){
                        currentColor = temp;
                    }else{
                        currentColor = Phong3(inter_buffer[k].color, inter_buffer[k].normal, inter_buffer[k].position, camPos, l);
                    }
                    break;
                case 2:
                    temp = shadow2(inter_buffer[k], l);
                    if(  temp != vec3(-1.0) ){
                        currentColor = temp;
                    }else{
                        currentColor = Phong3(inter_buffer[k].color, inter_buffer[k].normal, inter_buffer[k].position, camPos, l);
                    }
                    break;
            }
            result += currentColor;
        }
    }
    return clamp(result, 0.0, 1.0);
}

void main()
{
    vec3 col = ray_tracing();
    //Light l = get_light(0);
    //vec3 col = vec3(l.p2.r, 0.0, 0.0);
    outColor = vec4(col, 1.0);
}
