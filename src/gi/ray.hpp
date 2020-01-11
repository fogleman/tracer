#pragma once

#include <glm/glm.hpp>

class Ray {
public:
    Ray() {}

    Ray(const glm::vec3 &origin, const glm::vec3 &direction) :
        m_Origin(origin), m_Direction(direction) {}

    const glm::vec3 &Origin() const {
        return m_Origin;
    }

    const glm::vec3 &Direction() const {
        return m_Direction;
    }

    glm::vec3 At(const float t) const {
        return m_Origin + m_Direction * t;
    }

private:
    glm::vec3 m_Origin;
    glm::vec3 m_Direction;
};
