#pragma once

#include <glm/glm.hpp>

#include "constants.hpp"
#include "ray.hpp"
#include "util.hpp"

class Camera {
public:
    Camera(
        const glm::vec3 &eye,
        const glm::vec3 &center,
        const glm::vec3 &up,
        const float fovy,
        const float aspect,
        const float aperture,
        const float focalDistance)
    {
        const float d = focalDistance;
        const float theta = fovy * PI / 180;
        const float halfHeight = std::tan(theta / 2);
        const float halfWidth = halfHeight * aspect;
        m_W = glm::normalize(eye - center);
        m_U = glm::normalize(glm::cross(up, m_W));
        m_V = glm::cross(m_W, m_U);
        m_LowerLeft = eye - halfWidth * d * m_U - halfHeight * d * m_V - d * m_W;
        m_Horizontal = 2 * halfWidth * d * m_U;
        m_Vertical = 2 * halfHeight * d * m_V;
        m_Origin = eye;
        m_Aperture = aperture;
    }

    Ray MakeRay(const float u, const float v) const {
        const glm::vec3 rd = RandomInUnitDisk() * (m_Aperture / 2);
        const glm::vec3 offset = m_U * rd.x + m_V * rd.y;
        const glm::vec3 dir = glm::normalize(
            m_LowerLeft + m_Horizontal * u + m_Vertical * v - m_Origin - offset);
        return Ray(m_Origin + offset, dir);
    }

private:
    glm::vec3 m_Origin;
    glm::vec3 m_LowerLeft;
    glm::vec3 m_Horizontal;
    glm::vec3 m_Vertical;
    glm::vec3 m_U;
    glm::vec3 m_V;
    glm::vec3 m_W;
    float m_Aperture;
};
