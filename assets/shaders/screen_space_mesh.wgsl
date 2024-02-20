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

    let origin_z = (common_uniforms.camera.view_projection_matrix * model.model_matrix * vec4(0.0, 0.0, 0.0, 1.0)).z;

    out.position = (model.model_matrix * vec4(vert.position, 1.0)).xyz;
    out.normal = model.normal_matrix * vert.normal;
    out.out_position = common_uniforms.camera.view_projection_matrix * vec4(out.position, 1.0);

    out.out_position.w /= origin_z;

    out.out_position /= out.out_position.w * 3.0;

    out.out_position.z /= 1000;




    out.out_position.x *= (1 / f32(common_uniforms.resolution.y) / common_uniforms.camera.near * 7.0);
    out.out_position.y *= (1 / f32(common_uniforms.resolution.y) / common_uniforms.camera.near * 7.0);

    return out;
}
