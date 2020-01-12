#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/normal.hpp>

#include "config.hpp"
#include "util.hpp"

class Triangle {
public:
    Triangle(const vec3 &p1, const vec3 &p2, const vec3 &p3) :
        v1(p1), v2(p2), v3(p3)
    {
        n1 = n2 = n3 = glm::triangleNormal(p1, p2, p3);
    }

    vec3 NormalAt(const vec3 &p) const {
        const vec3 b = Barycentric(v1, v2, v3, p);
        return glm::normalize(b.x * n1 + b.y * n2 + b.z * n3);
    }

    vec3 v1, v2, v3;
    vec3 n1, n2, n3;

private:
};
