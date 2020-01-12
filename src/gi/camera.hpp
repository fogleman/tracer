#pragma once

#include <glm/glm.hpp>

#include "config.hpp"
#include "ray.hpp"
#include "util.hpp"

class Camera {
public:
    Camera(
        const vec3 &eye,
        const vec3 &center,
        const vec3 &up,
        const real fovy,
        const real aspect,
        const real aperture,
        const real focalDistance)
    {
        const real d = focalDistance;
        const real theta = fovy * PI / 180;
        const real halfHeight = std::tan(theta / 2);
        const real halfWidth = halfHeight * aspect;
        m_W = glm::normalize(eye - center);
        m_U = glm::normalize(glm::cross(up, m_W));
        m_V = glm::cross(m_W, m_U);
        m_LowerLeft = eye - halfWidth * d * m_U - halfHeight * d * m_V - d * m_W;
        m_Horizontal = 2 * halfWidth * d * m_U;
        m_Vertical = 2 * halfHeight * d * m_V;
        m_Origin = eye;
        m_Aperture = aperture;
    }

    Ray MakeRay(const real u, const real v) const {
        const vec3 rd = RandomInUnitDisk() * (m_Aperture / 2);
        const vec3 offset = m_U * rd.x + m_V * rd.y;
        const vec3 dir = glm::normalize(
            m_LowerLeft + m_Horizontal * u + m_Vertical * v - m_Origin - offset);
        return Ray(m_Origin + offset, dir);
    }

private:
    vec3 m_Origin;
    vec3 m_LowerLeft;
    vec3 m_Horizontal;
    vec3 m_Vertical;
    vec3 m_U;
    vec3 m_V;
    vec3 m_W;
    real m_Aperture;
};
