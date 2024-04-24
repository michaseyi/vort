// #define_import_path vort::common


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
    camera: Camera,
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



struct Camera {
    position: vec3<f32>,
    view_projection_matrix: mat4x4<f32>,
    fov: f32,
    aspect: f32,
    near: f32,
    far: f32,
    zoom: f32,
};

@group(0) @binding(0) var<uniform> common_uniforms: CommonUniforms;
