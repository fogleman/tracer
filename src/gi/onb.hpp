#pragma once

#include <cmath>
#include <glm/glm.hpp>

#include "config.hpp"

const real INV_SQRT_3 = 1 / std::sqrt(3);

class ONB {
public:
    ONB(const vec3 &n) {
        const vec3 normal = glm::normalize(n);
        vec3 majorAxis;
        if (std::abs(normal.x) < INV_SQRT_3) {
            majorAxis = vec3(1, 0, 0);
        } else if (std::abs(normal.y) < INV_SQRT_3) {
            majorAxis = vec3(0, 1, 0);
        } else {
            majorAxis = vec3(0, 0, 1);
        }
        m_S = glm::normalize(glm::cross(normal, majorAxis));
        m_T = glm::cross(normal, m_S);
        m_N = normal;
    }

    vec3 WorldToLocal(const vec3 &v) const {
        return vec3(glm::dot(v, m_S), glm::dot(v, m_T), glm::dot(v, m_N));
    }

    vec3 LocalToWorld(const vec3 &v) const {
        const real x = m_S.x * v.x + m_T.x * v.y + m_N.x * v.z;
        const real y = m_S.y * v.x + m_T.y * v.y + m_N.y * v.z;
        const real z = m_S.z * v.x + m_T.z * v.y + m_N.z * v.z;
        return vec3(x, y, z);
    }

private:
    vec3 m_S;
    vec3 m_T;
    vec3 m_N;
};
