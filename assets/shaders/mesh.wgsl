#import "common"

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
    @location(3) uv: vec2<f32>
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

@fragment
fn fs_main_face(frag: FaceFragment) -> FSOutput {
    var out: FSOutput;

    let normal = normalize(frag.normal);

    let total_light = compute_total_light(frag.position, normal);

    out.color = vec4<f32>(frag.color * total_light, 1.0);
    return out;
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