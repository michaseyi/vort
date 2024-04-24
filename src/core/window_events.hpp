#pragma once
namespace core {
struct PointerMove {
  float x;
  float y;
  float movement_x;
  float movement_y;

  int client_width;
  int client_height;
};

struct WindowResize {
  int width;
  int height;
};

struct PointerDown {
  int x;
  int y;

  int clieht_width;
  int client_height;
};

}  // namespace core