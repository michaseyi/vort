#pragma once
#include "src/renderer/render_pass.hpp"

namespace core {
struct RenderTargetOutput {
  renderer::RenderPass render_pass;
  renderer::HtmlCanvasRenderTarget html_render_target;
};
struct RenderTargetOutputs {
  std::vector<RenderTargetOutput> outputs;
};
}  // namespace core