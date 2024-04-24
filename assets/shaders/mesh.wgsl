#import "light" 
// #import vort::common
// #import vort::light

struct ModelUniforms {
    model_matrix: mat4x4<f32>,
    normal_matrix: mat3x3<f32>
};


struct FaceVertex {
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
    @location(2) normal: vec3<f32>,
    @location(3) uv: vec2<f32>
}

struct FaceVSOutput {
    @builtin(position) out_position: vec4<f32>,
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
    @location(2) normal: vec3<f32>,
    @location(3) uv: vec2<f32>
}

struct EdgeVSOutput {
    @builtin(position) out_position: vec4<f32>,
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
}

struct EdgeVertex {
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
}

struct PointVSOutput {
    @builtin(position) out_position: vec4<f32>,
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
    @location(2) offset: vec2<f32>,
}

struct PointVertex {
    @builtin(vertex_index) vertex_index: u32,
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
}

@group(0) @binding(1) var<uniform> model: ModelUniforms;

@group(0) @binding(2) var texture_sampler: sampler;
@group(0) @binding(3) var albedo_map: texture_2d<f32>;
@group(0) @binding(4) var ao_map: texture_2d<f32>; 
@group(0) @binding(5) var height_map: texture_2d<f32>; 
@group(0) @binding(6) var metallic_map: texture_2d<f32>; 
@group(0) @binding(7) var normal_ogl_map: texture_2d<f32>; 
@group(0) @binding(8) var roughness_map: texture_2d<f32>; 


const PI: f32 = 3.14159265359;

@vertex
fn vs_main_face(vert: FaceVertex) -> FaceVSOutput {
    var out: FaceVSOutput;
    out.position = (model.model_matrix * vec4(vert.position, 1.0)).xyz;
    out.out_position = common_uniforms.camera.view_projection_matrix * vec4(out.position, 1.0);
    out.normal = model.normal_matrix * vert.normal;
    out.uv = vert.uv;
    out.color = vert.color;
    return out;
}

@vertex
fn vs_main_edge(vert: EdgeVertex) -> EdgeVSOutput {
    var out: EdgeVSOutput;
    out.position = (model.model_matrix * vec4(vert.position, 1.0)).xyz;
    out.out_position = common_uniforms.camera.view_projection_matrix * vec4(out.position, 1.0);
    out.out_position.z -= 0.0001;
    out.color = vert.color;
    return out;
}

@vertex
fn vs_main_point(vert: PointVertex) -> PointVSOutput {
    var out: PointVSOutput;
    out.position = (model.model_matrix * vec4(vert.position, 1.0)).xyz;
    out.out_position = common_uniforms.camera.view_projection_matrix * vec4(out.position, 1.0);
    out.color = vert.color;
    out.out_position.z -= 0.001;
    out.out_position /= out.out_position.w;

    let offsets: array<vec2<f32>, 6> = array<vec2<f32>, 6>(
        vec2<f32>(-1.0, 1.0),
        vec2<f32>(-1.0, -1.0),
        vec2<f32>(1.0, -1.0),
        vec2<f32>(1.0, -1.0),
        vec2<f32>(1.0, 1.0),
        vec2<f32>(-1.0, 1.0),
    );

    let factor_x: f32 = 5.0 / f32(common_uniforms.resolution.x);
    let factor_y: f32 = 5.0 / f32(common_uniforms.resolution.y);

    out.out_position.x += offsets[vert.vertex_index].x * factor_x;
    out.out_position.y += offsets[vert.vertex_index].y * factor_y;

    out.offset = offsets[vert.vertex_index];

    return out;
}

struct FaceFragment {
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
    @location(2) normal: vec3<f32>,
    @location(3) uv: vec2<f32>,
    @builtin(front_facing) front_facing: bool
}

struct EdgeFragment {
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
}

struct PointFragment {
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
    @location(2) offset: vec2<f32>,
}

struct FSOutput {
    @location(0) color: vec4<f32>
}


fn compute_specular_reflectance(metallicness: f32, roughness: f32, surface_normal: vec3<f32>, position: vec3<f32>, light_vector: vec3<f32>) -> vec3<f32> {
    let view_vector = normalize(common_uniforms.camera.position - position);

    let reflected_light_vector = reflect(light_vector, surface_normal);

    let specular_reflectance = pow(max(dot(reflected_light_vector, view_vector), 0.0), (1.0 - roughness) * 128.0);

    return vec3<f32>(specular_reflectance);
}
@fragment
fn fs_main_face(frag: FaceFragment) -> FSOutput {
    var out: FSOutput;

    let surface_normal = select(normalize(frag.normal), normalize(-frag.normal), !frag.front_facing);

    // let albedo = textureSample(albedo_map, texture_sampler, frag.uv).rgb;
    // let metallicness = textureSample(metallic_map, texture_sampler, frag.uv).r;
    // let ao = textureSample(ao_map, texture_sampler, frag.uv).r;
    // let roughness = textureSample(roughness_map, texture_sampler, frag.uv).r;
    let total_light = compute_total_light(frag.position, surface_normal);

    // var total_reflectance: vec3<f32> = vec3<f32>(0.0, 0.0, 0.0);

    // for (var i = 0u; i < common_uniforms.directional_light_count; i++) {
    //     var light_source = common_uniforms.directional_lights[i];
    //     let light_intensity = compute_directional_light(light_source, surface_normal) * ao;

    //     let diffuse_reflectance = albedo / f32(PI) ;

    //     let specular_reflectance = compute_specular_reflectance(metallicness, roughness, surface_normal, frag.position, light_source.direction) * metallicness;
    //     total_reflectance += light_intensity * (diffuse_reflectance + specular_reflectance);
    // }

    // for (var i = 0u; i < common_uniforms.point_light_count; i++) {
    //     var light_source = common_uniforms.point_lights[i];
    //     let light_intensity = compute_point_light(light_source, frag.position, surface_normal);

    //     let diffuse_reflectance = albedo / f32(2.0 * PI);

    //     total_reflectance += light_intensity * diffuse_reflectance;
    // }


    out.color = vec4<f32>(
        frag.color * total_light, 1.0
    );
    return out;
}

fn reinhard_tonemap(hdr_color: vec3<f32>, key: f32) -> vec3<f32> {
    // Calculate luminance of the HDR color
    let luminance = dot(hdr_color, vec3(0.2126, 0.7152, 0.0722));
    // Calculate the scaling factor based on the key value
    let scale = key / (luminance + 0.0001); // Add a small epsilon to prevent division by zero
    
    // Apply tone mapping to each channel
    let tone_mapped_color = hdr_color * scale;

    return tone_mapped_color;
}

fn aces_tonemap_(x: f32) -> f32 {
    return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
}

fn aces_tonemap(hdr_color: vec3<f32>) -> vec3<f32> {
    return vec3(aces_tonemap_(hdr_color.r), aces_tonemap_(hdr_color.g), aces_tonemap_(hdr_color.b));
}

@fragment
fn fs_main_point(frag: PointFragment) -> FSOutput {
    var out: FSOutput;

    out.color = vec4<f32>(frag.color, step(length(frag.offset), 1.0));
    return out;
}

@fragment
fn fs_main_edge(frag: EdgeFragment) -> FSOutput {
    var out: FSOutput;

    out.color = vec4<f32>(frag.color, 1.0);
    return out;
}
