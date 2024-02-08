#import "light"

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



@group(0) @binding(0) var<uniform> common_uniforms: CommonUniforms;
