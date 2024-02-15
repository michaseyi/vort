#import "common"

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
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>
}

@group(0) @binding(1) var<uniform> model: ModelUniforms;

@vertex
fn vs_main(vert: Vertex) -> VSOutput {
    var out: VSOutput;

    out.position = (model.model_matrix * vec4(vert.position, 1.0)).xyz;
    out.normal = model.normal_matrix * vert.normal;
    out.out_position = common_uniforms.camera.view_projection_matrix * vec4(out.position, 1.0);

    out.out_position.z /= 1000.0;
    out.out_position.w = 1.0;

    let a: f32 = (sin(radians(common_uniforms.camera.fov / 2)) * common_uniforms.camera.near) / sin(radians(180.0 - 90.0 - (common_uniforms.camera.fov / 2)));
    let b: f32 = (sin(radians((common_uniforms.camera.fov + common_uniforms.camera.zoom) / 2)) * common_uniforms.camera.near) / sin(radians(180.0 - 90.0 - ((common_uniforms.camera.fov + common_uniforms.camera.zoom) / 2)));

    var ratio = b / a;

    ratio /= 10.0;

    out.out_position.x *= ratio;
    out.out_position.y *= ratio;

    return out;
}
