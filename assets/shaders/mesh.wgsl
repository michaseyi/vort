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
    out.out_position = common_uniforms.camera.view_projection_matrix * vec4(out.position, 1.0);
    out.normal = model.normal_matrix * vert.normal;
    out.uv = vert.uv;
    return out;
}
