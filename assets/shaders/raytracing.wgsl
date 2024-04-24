
struct CommonUniforms {
    camera: Camera,
    time: f32,
    total_acceleration_structures_in_view: u32,
};

struct Camera {
    position: vec3<f32>,
    up: vec3<f32>,
    front: vec3<f32>
    side: vec3<f32>,
    fov: f32,
    aspect: f32,
    resolution: vec2<u32>,
    z_near: f32,
    z_far: f32,
};

// TODO: Implement a bit level boolean to store multiple boolean values in a single u32
// This will allow us to store multiple boolean values in a single u32
struct RayPayload {
    total_absorption_spectrum: vec3<f32>,
    total_light_spectrum: vec3<f32> ,
    primiary_ray: u32,
    total_absorption_spectrum_initialized: u32,
    total_light_spectrum_initialized: u32,
}

struct Ray {
    direction: vec3<f32>,
    origin: vec3<f32>,
    payload: RayPayload,
}

struct AABB {
    min: vec3<f32>,
    max: vec3<f32>,
}

struct AccelerationStructure {
    aabb: AABB,
    face_offset: u32,
    face_count: u32,
    transparent: u32,
}

struct RayIntersectFaceInfo {
    t: f32,
    u: f32,
    v: f32,
}

struct RayHitInfo {
    intersection_info: RayIntersectFaceInfo,
    face_index: u32,
    acceleration_structure_index: u32,
}

struct RayGenInput {
    @builtin(global_invocation_id) id: vec3<u32> 
}

struct ComputeInput {
    @builtin(global_invocation_id) id: vec3<u32> 
}

// There are limitation to the acceleration structre, it can only be 1 level deep. As a result, BVH constructions from a mesh
// will be useless. Instead, we will use a single bounding box acceleration structure for each mesh. The top level acceleration
// structure will be a bounding box of the scene visible from the camera and the indices of the acceleraction structures of the
// meshes within the cameras frustrum.


@group(0) @binding(0) var<uniform> common_uniforms: CommonUniforms;

@group(0) @binding(1) var<storage, read> faces_vertex_positions: array<array<vec3<f32>, 3>>;

@group(0) @binding(2) var<storage, read> faces_vertex_normals: array<array<vec3<f32>, 3>>;

@group(0) @binding(3) var<storage, read> faces_vertex_colors: array<array<vec3<f32>, 3>>;

@group(0) @binding(4) var<storage, read> faces_vertex_uvs: array<array<vec2<f32>, 3>>;



@group(0) @binding(5) var<storage, read> acceleraction_structures_in_view: array<u32>;

@group(0) @binding(6) var<storage, read_write> rays: array<Ray>;

@group(0) @binding(7) var<storage, read> acceleration_structures: array<AccelerationStructure>;




var<private> global_ray_id: u32;

var<private> closest_hit_intersection_info: RayIntersectFaceInfo;

var<private> closest_hit_face_index: u32;

var<private> closest_hit_acceleration_structure_index: u32;

var<private> closest_hit_initialized: bool = false;

var<private> ray_progressing: bool = true;


@compute @workgroup_size(1)
fn main(in: ComputeInput) {
    // All calculations are done in camera space with the camera at the origin
    global_ray_id = in.id.y * common_uniforms.camera.resolution.x + in.id.x;

    raygen(RayGenInput(in.id));


    while ray_progressing {
        raytrace();

        if closest_hit_initialized {
            var rayhit_info: RayHitInfo;
            rayhit_info.intersection_info = closest_hit_intersection_info;
            rayhit_info.face_index = closest_hit_face_index;
            rayhit_info.acceleration_structure_index = closest_hit_acceleration_structure_index;

            raychit(rayhit_info);
        }
    }
}

fn raygen(in: RayGenInput) {
    var ray: Ray;

    let height = sqrt(pow(
        cos(common_uniforms.camera.fov * 0.5) * common_uniforms.camera.z_near, 2.0
    ) * pow(common_uniforms.camera.z_near, 2.0)) * 2.0;

    let width = common_uniforms.camera.aspect * height;


    let offsets = vec2<f32>(
        ((f32(in.id.x) + 0.5) / f32(common_uniforms.camera.resolution.x)) * width - width * 0.5,
        ((f32(in.id.y) + 0.5) / f32(common_uniforms.camera.resolution.y)) * height - height * 0.5,
    );

    let pixelPointOnNearPlane = common_uniforms.camera.position + (common_uniforms.camera.front * common_uniforms.camera.z_near) + (common_uniforms.camera.side * offsets.x) + (common_uniforms.camera.up * offsets.y);

    ray.origin = common_uniforms.camera.position;
    ray.direction = normalize(pixelPointOnNearPlane - common_uniforms.camera.position);
    ray.payload.primiary_ray = 1u;

    rays[global_ray_id] = ray;
}

fn ray_aabb_intersection(ray: Ray, aabb: AABB) -> bool {
    let tmin = (aabb.min - ray.origin) / ray.direction;
    let tmax = (aabb.max - ray.origin) / ray.direction;

    let t1 = min(tmin, tmax);
    let t2 = max(tmin, tmax);

    let t_near = max(max(t1.x, t1.y), t1.z);
    let t_far = min(min(t2.x, t2.y), t2.z);

    return t_near <= t_far;
}


// Moller-Trumbore intersection algorithm
fn ray_faces_intersection(ray: Ray, faces_position: array<vec3<f32>, 3>) -> RayIntersectFaceInfo {
    var ray_intersect_face_info: RayIntersectFaceInfo;

    ray_intersect_face_info.t = -1.0;
    ray_intersect_face_info.u = 0.0;
    ray_intersect_face_info.v = 0.0;

    let edge1 = faces_position[1] - faces_position[0];
    let edge2 = faces_position[2] - faces_position[0];

    let h = cross(ray.direction, edge2);
    let a = dot(edge1, h);

    if a > -0.00001 && a < 0.00001 {
        ray_intersect_face_info.t = -1.0;
        ray_intersect_face_info.u = 0.0;
        ray_intersect_face_info.v = 0.0;
        return ray_intersect_face_info;
    }

    let f = 1.0 / a;
    let s = ray.origin - faces_position[0];
    let u = f * dot(s, h);

    if u < 0.0 || u > 1.0 {
        ray_intersect_face_info.t = -1.0;
        ray_intersect_face_info.u = 0.0;
        ray_intersect_face_info.v = 0.0;
        return ray_intersect_face_info;
    }

    let q = cross(s, edge1);
    let v = f * dot(ray.direction, q);

    if v < 0.0 || u + v > 1.0 {

        ray_intersect_face_info.t = -1.0;
        ray_intersect_face_info.u = 0.0;
        ray_intersect_face_info.v = 0.0;
        return ray_intersect_face_info;
    }

    let t = f * dot(edge2, q);

    if t > 0.00001 {
        ray_intersect_face_info.t = t;
        ray_intersect_face_info.u = u;
        ray_intersect_face_info.v = v;
        return ray_intersect_face_info;
    }

    return ray_intersect_face_info;
}


fn raytrace() {
    for (var i = 0u; i < common_uniforms.total_acceleration_structures_in_view; i++) {
        let current_acceleration_structure_index = acceleraction_structures_in_view[i];
        let current_acceleration_structure = acceleration_structures[current_acceleration_structure_index];

        if ray_aabb_intersection(rays[global_ray_id], current_acceleration_structure.aabb) {

            for (var j = current_acceleration_structure.face_offset ; j < current_acceleration_structure.face_count; j ++) {

                let face_vertex_positions = faces_vertex_positions[j];


                let intersection_info = ray_faces_intersection(rays[global_ray_id], face_vertex_positions);


                if intersection_info.t <= 0.0 {
                    continue;
                }

                let range = dot(rays[global_ray_id].direction * intersection_info.t, common_uniforms.camera.front);

                if range >= common_uniforms.camera.z_near && range <= common_uniforms.camera.z_far {
                    // We have an intersection
                    var rayhit_info: RayHitInfo;
                    rayhit_info.intersection_info = intersection_info;
                    rayhit_info.face_index = j;
                    rayhit_info.acceleration_structure_index = current_acceleration_structure_index;
                    rayhit(rayhit_info);
                }
            }
        }
    }
}

fn rayhit(in: RayHitInfo) {
    if !closest_hit_initialized || in.intersection_info.t < closest_hit_intersection_info.t {
        closest_hit_intersection_info = in.intersection_info;
        closest_hit_face_index = in.face_index;
        closest_hit_acceleration_structure_index = in.acceleration_structure_index;
        closest_hit_initialized = true;
    }
}


fn raychit(in: RayHitInfo) {
    // interpollate the normal, color and uv

    let face_vertex_positions = faces_vertex_positions[in.face_index];
    let face_vertex_normals = faces_vertex_normals[in.face_index];
    let face_vertex_colors = faces_vertex_colors[in.face_index];
    let face_vertex_uvs = faces_vertex_uvs[in.face_index];


    // get the barycentric interpolation for each vertex property 
    let position = rays[global_ray_id].origin + rays[global_ray_id].direction * closest_hit_intersection_info.t;
    let interpolated_normal = normalize(face_vertex_normals[0] * (1.0 - closest_hit_intersection_info.u - closest_hit_intersection_info.v) + face_vertex_normals[1] * closest_hit_intersection_info.u + face_vertex_normals[2] * closest_hit_intersection_info.v);
    let interpolated_color = face_vertex_colors[0] * (1.0 - closest_hit_intersection_info.u - closest_hit_intersection_info.v) + face_vertex_colors[1] * closest_hit_intersection_info.u + face_vertex_colors[2] * closest_hit_intersection_info.v;
    let interpolated_uv = face_vertex_uvs[0] * (1.0 - closest_hit_intersection_info.u - closest_hit_intersection_info.v) + face_vertex_uvs[1] * closest_hit_intersection_info.u + face_vertex_uvs[2] * closest_hit_intersection_info.v;


    // calculate the absorption spectrum
    if rays[global_ray_id].payload.total_absorption_spectrum_initialized == 0u {
        rays[global_ray_id].payload.total_absorption_spectrum = interpolated_color;
    } else {
        rays[global_ray_id].payload.total_absorption_spectrum *= interpolated_color;
    }

    // calculate the light spectrum
    // calculate the light reaching the intersection point
    // Two types of light
    // 1. Direct light from a known light source; will only ray intersection with all the possible light sources
    // 2. Indirect light from other surfaces; will require a call to raytrace

    ray_progressing = false;
}