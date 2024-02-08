

// #include <emscripten/bind.h>
// #include <emscripten/emscripten.h>
// #include <emscripten/fetch.h>




// template <typename T>
// struct Vector3 {
//     union {
//         struct {
//             T x;
//             T y;
//             T z;
//         };
//         T data[3];
//     };

//     void setX(T value) { x = value; };
//     void setY(T value) { y = value; };
//     void setZ(T value) { z = value; };

//     T getX() const {
//         std::cout << this << std::endl;
//         return x;
//     };
//     T getY() const { return y; };
//     T getZ() const { return z; };
// };

// EM_JS(int, getDisplayCanvasWidth, (), { return document.querySelector("#canvas").width; });

// EM_JS(int, getDisplayCanvasHeight, (), { return document.querySelector("#canvas").height; });


// #if defined(EMSCRIPTEN)
// using namespace emscripten;

// std::vector<int> shout() { return {1, 2, 3}; }

// EMSCRIPTEN_BINDINGS(bindings) {
//     class_<Vector3<float>>("Vector3Float")
//         .constructor()
//         .property("x", &Vector3<float>::getX, &Vector3<float>::setX)
//         .property("y", &Vector3<float>::getY, &Vector3<float>::setY)
//         .property("z", &Vector3<float>::getZ, &Vector3<float>::setZ);

//     // function("getPositionComponent", &Position::getComponent, allow_raw_pointers());
//     // function("getVelocityComponent", &Velocity::getComponent, allow_raw_pointers());
//     function("shout", &shout);
//     register_vector<int>("vector<int>");
// }
// #endif
