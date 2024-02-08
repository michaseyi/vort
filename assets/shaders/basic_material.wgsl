#import "light"

struct BasicMaterialUniforms {
    base_color: vec3<f32>,
    alpha: f32,
    emissive_color: vec3<f32>,
    specular: f32,
    shininess: f32,
};


struct Fragment {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>
}

struct FSOutput {
    @location(0) color: vec4<f32>
}

@group(1) @binding(0) var<uniform> material: BasicMaterialUniforms;

@fragment
fn fs_main(frag: Fragment) -> FSOutput {
    var out: FSOutput;

    let normal = normalize(frag.normal);

    let total_light = compute_total_light(frag.position, normal);

    out.color = vec4<f32>(material.base_color * total_light, 1.0);
    return out;
}