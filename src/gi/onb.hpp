#pragma once

#include <cmath>
#include <glm/glm.hpp>

const float INV_SQRT_3 = 1 / std::sqrt(3);

class ONB {
public:
    ONB(const glm::vec3 &n) {
        const glm::vec3 normal = glm::normalize(n);
        glm::vec3 majorAxis;
        if (std::abs(normal.x) < INV_SQRT_3) {
            majorAxis = glm::vec3(1, 0, 0);
        } else if (std::abs(normal.y) < INV_SQRT_3) {
            majorAxis = glm::vec3(0, 1, 0);
        } else {
            majorAxis = glm::vec3(0, 0, 1);
        }
        m_S = glm::normalize(glm::cross(normal, majorAxis));
        m_T = glm::cross(normal, m_S);
        m_N = normal;
    }

    glm::vec3 WorldToLocal(const glm::vec3 &v) const {
        return glm::vec3(glm::dot(v, m_S), glm::dot(v, m_T), glm::dot(v, m_N));
    }

    glm::vec3 LocalToWorld(const glm::vec3 &v) const {
        const float x = m_S.x * v.x + m_T.x * v.y + m_N.x * v.z;
        const float y = m_S.y * v.x + m_T.y * v.y + m_N.y * v.z;
        const float z = m_S.z * v.x + m_T.z * v.y + m_N.z * v.z;
        return glm::vec3(x, y, z);
    }

private:
    glm::vec3 m_S;
    glm::vec3 m_T;
    glm::vec3 m_N;
};
