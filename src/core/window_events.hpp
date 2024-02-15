#pragma once

struct PointerMove {
    float x;
    float y;
    float movementX;
    float movementY;

    int clientWidth;
    int clientHeight;
};

struct WindowResize {
    int width;
    int height;
};

struct PointerDown {
    int x;
    int y;

    int clientWidth;
    int clientHeight;
};
