#include <iostream>

#include "mesh_structure.hpp"

using namespace vort::mesh;

int32_t main() {
    MeshStructure mesh;

    auto topLeft = mesh.addVertex(math::vec3(-1, 1, 0));
    auto bottomLeft = mesh.addVertex(math::vec3(-1, -1, 0));
    auto bottomRight = mesh.addVertex(math::vec3(1, -1, 0));
    auto topRight = mesh.addVertex(math::vec3(1, 1, 0));
    auto center = mesh.addVertex(math::vec3(0));

    // auto face1 = mesh.addFace({topLeft, bottomLeft, center});
    // auto face2 = mesh.addFace({bottomLeft, bottomRight, center});
    // auto face3 = mesh.addFace({bottomRight, topRight, topRight});
    // auto face4 = mesh.addFace({center, topRight, bottomLeft});

    auto face1 = mesh.addFace({topLeft, bottomLeft, center});
    auto face2 = mesh.addFace({bottomLeft, bottomRight, center});
    auto face3 = mesh.addFace({bottomRight, topRight, center});
    auto face4 = mesh.addFace({center, topRight, topLeft});

    for (auto halfedge : mesh.halfedges(face4)) {
        auto halfedgeTwin = mesh.halfedge(halfedge).twin;
        if (halfedgeTwin.valid() == false) {
            continue;
        }

        std::cout << mesh.halfedge(halfedgeTwin).face.idx() << std::endl;
    }
    return 0;
}
