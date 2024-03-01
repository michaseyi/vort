

#include "../cpu_mesh.hpp"

class Modifier {
public:
    virtual void modify(CPUMesh& tMesh) = 0;
};
