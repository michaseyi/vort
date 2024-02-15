
struct PlainMaterialUniform {
    color: vec4<f32>,
};

struct FSOutput {
    @location(0) color: vec4<f32>
}

@group(1) @binding(0) var<uniform> material: PlainMaterialUniform;

@fragment
fn fs_main() -> FSOutput {
    var out: FSOutput;
    out.color = material.color;
    return out;
}