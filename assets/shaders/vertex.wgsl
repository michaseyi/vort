
struct AmbientLight {
    base_color: vec3<f32>,
    intensity: f32
}

struct PointLight {
    position: vec3<f32>,
    intensity: f32,
    color: vec3<f32>,
    attenuation_coefficients: vec3<f32>,
};

struct DirectionalLight {
    direction: vec3<f32>,
    intensity: f32,
    color: vec3<f32>,
    attenuation_coefficients: vec3<f32>,
};

struct SpotLight {
    position: vec3<f32>,
    inner_cone_angle: f32,
    direction: vec3<f32>,
    outer_cone_angle: f32,
    color: vec3<f32>,
    intensity: f32,
    attenuation_coefficients: vec3<f32>,
};


struct CommonUniforms {
    view_projection_matrix: mat4x4<f32>,
    time: f32,
    resolution: vec2<u32>,
    ambient_lights: array<AmbientLight, 5>,
    ambient_light_count: u32,
    point_lights: array<PointLight, 5>,
    point_light_count: u32,
    directional_lights: array<DirectionalLight, 5>,
    directional_light_count: u32,
    spot_lights: array<SpotLight, 5>,
    spot_light_count: u32,
};

struct BasicMaterialUniforms {
    base_color: vec3<f32>,
    alpha: f32,
    emissive_color: vec3<f32>,
    specular: f32,
    shininess: f32,
};

struct ModelUniforms {
    model_matrix: mat4x4<f32>,
    normal_matrix: mat3x3<f32>
};

struct Vertex {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>
}

struct VSOutput {
    @builtin(position) out_position: vec4<f32>,
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>
}

struct Fragment {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>
}

struct FSOutput {
    @location(0) color: vec4<f32>
}


@group(0) @binding(0) var<uniform> common: CommonUniforms;
@group(0) @binding(1) var<uniform> model: ModelUniforms;
@group(1) @binding(0) var<uniform> material: BasicMaterialUniforms;

fn compute_attenuation_factor(attenuation_coefficients: vec3<f32>, distance: f32) -> f32 {
    return 1.0 / dot(attenuation_coefficients, vec3<f32>(distance * distance, distance, 1.0));
}

fn compute_ambient_light(light: AmbientLight) -> vec3<f32> {
    return light.base_color * light.intensity;
}


fn compute_point_light(light: PointLight, position: vec3<f32>, normal: vec3<f32>) -> vec3<f32> {
    let light_vector: vec3<f32> = normalize(light.position - position);
    let illumination_factor: f32 = max(0, dot(light_vector, normal));
    let attenuation_factor: f32 = compute_attenuation_factor(light.attenuation_coefficients, abs(light.position - position));
    return illumination_factor * attenuation_factor * light.intensity * light.color;
}

fn compute_directional_light(light: DirectionalLight, position: vec3<f32>, normal: vec3<f32>) -> vec3<f32> {
    let illumination_factor: f32 = max(0, dot(light_vector, normal));
    let attenuation_factor: f32 = compute_attenuation_factor(light.attenuation_coefficients, abs(light.position - position));
    return illumination_factor * attenuation_factor * light.intensity * light.color;
}

fn compute_spot_light(light: SpotLight, position: vec3<f32>, normal: vec3<f32>) -> vec3<f32> {
    let light_vector: vec3<f32> = normalize(light.position - position);
    let illumination_factor: f32 = max(0, dot(light_vector, normal));
    let cosine_cone_angle: f32 = acos(dot(light.direction, -light_vector));
    let falloff_factor: f32 = 1.0 - smoothstep(light.inner_cone_angle, light.outer_cone_angle, cosine_cone_angle);
    let attenuation_factor: f32 = compute_attenuation_factor(light.attenuation_coefficients, abs(light.position - position));

    return falloff_factor * illumination_factor * attenuation_factor * light.intensity * light.color;
}


fn vs_main(vert: Vertex) -> VSOutput {
    var out: VSOutput;

    out.position = (model.model_matrix * vec4(vert.position, 1.0)).xyz;
    out.normal = model.normal_matrix * vert.normal;
    out.out_position = common.view_projection_matrix * vec4(out.position, 1.0);

    return out;
}


fn fs_main(frag: Fragment) -> FSOutput {
    var out: FSOutput;

    let normal = normalize(frag.normal);

    var total_light: vec3<f32> = vec3<f32>(0.0, 0.0, 0.0);

    for (var i: i32 = 0; i < common.ambient_light_count; i++) {
        total_light += compute_ambient_light(common.ambient_lights[i]);
    }
    for (var i: i32 = 0; i < common.spot_light_count; i++) {
        total_light += compute_spot_light(common.spot_lights[i], frag.position, normal);
    }
    for (var i: i32 = 0; i < common.directional_light_count; i++) {
        total_light += compute_directional_light(common.directional_lights[i], frag.position, normal);
    }
    for (var i: i32 = 0; i < common.point_light_count; i++) {
        total_light += compute_point_light(common.point_lights[i], frag.position, normal);
    }

    out.color = material.base_color * total_light;
    return out;
}