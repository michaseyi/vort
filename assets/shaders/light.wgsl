#import "common"

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

fn compute_attenuation_factor(attenuation_coefficients: vec3<f32>, distance: f32) -> f32 {
    return 1.0 / dot(attenuation_coefficients, vec3<f32>(distance * distance, distance, 1.0));
}

fn compute_ambient_light(light: AmbientLight) -> vec3<f32> {
    return light.base_color * light.intensity;
}


fn compute_point_light(light: PointLight, position: vec3<f32>, normal: vec3<f32>) -> vec3<f32> {
    let light_vector: vec3<f32> = normalize(light.position - position);
    let illumination_factor: f32 = max(0, dot(light_vector, normal));
    let attenuation_factor: f32 = compute_attenuation_factor(light.attenuation_coefficients, distance(light.position, position));
    return illumination_factor * attenuation_factor * light.intensity * light.color;
}

fn compute_directional_light(light: DirectionalLight, normal: vec3<f32>) -> vec3<f32> {
    let illumination_factor: f32 = max(0, dot(-light.direction, normal));
    return illumination_factor * light.intensity * light.color;
}

fn compute_spot_light(light: SpotLight, position: vec3<f32>, normal: vec3<f32>) -> vec3<f32> {
    let light_vector: vec3<f32> = normalize(light.position - position);
    let illumination_factor: f32 = max(0, dot(light_vector, normal));
    let cosine_cone_angle: f32 = acos(dot(light.direction, -light_vector));
    let falloff_factor: f32 = 1.0 - smoothstep(light.inner_cone_angle, light.outer_cone_angle, cosine_cone_angle);
    let attenuation_factor: f32 = compute_attenuation_factor(light.attenuation_coefficients, distance(light.position, position));

    return falloff_factor * illumination_factor * attenuation_factor * light.intensity * light.color;
}

fn compute_total_light(position: vec3<f32>, surface_normal: vec3<f32>) -> vec3<f32> {
    var total_light: vec3<f32> = vec3<f32>(0.0, 0.0, 0.0);

    for (var i: u32 = 0; i < common_uniforms.ambient_light_count; i++) {
        total_light += compute_ambient_light(common_uniforms.ambient_lights[i]);
    }

    for (var i: u32 = 0; i < common_uniforms.spot_light_count; i++) {
        total_light += compute_spot_light(common_uniforms.spot_lights[i], position, surface_normal);
    }
    for (var i: u32 = 0; i < common_uniforms.directional_light_count; i++) {
        total_light += compute_directional_light(common_uniforms.directional_lights[i], surface_normal);
    }
    for (var i: u32 = 0; i < common_uniforms.point_light_count; i++) {
        total_light += compute_point_light(common_uniforms.point_lights[i], position, surface_normal);
    }

    return total_light;
}