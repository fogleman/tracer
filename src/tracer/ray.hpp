#pragma once

#include "config.hpp"

class Ray {
public:
    Ray() {}

    Ray(const vec3 &origin, const vec3 &direction) :
        m_Origin(origin), m_Direction(direction) {}

    const vec3 &Origin() const {
        return m_Origin;
    }

    const vec3 &Direction() const {
        return m_Direction;
    }

    vec3 At(const real t) const {
        return m_Origin + m_Direction * t;
    }

private:
    vec3 m_Origin;
    vec3 m_Direction;
};
