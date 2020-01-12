#pragma once

#include <glm/glm.hpp>

#include "config.hpp"

class Box {
public:
    Box() :
        m_Min(vec3(0)), m_Max(vec3(0)) {}

    Box(const vec3 &min, const vec3 &max) :
        m_Min(glm::min(min, max)), m_Max(glm::max(min, max)) {}

    const vec3 &Min() const {
        return m_Min;
    }

    const vec3 &Max() const {
        return m_Max;
    }

    vec3 Size() const {
        return m_Max - m_Min;
    }

    real Volume() const {
        const vec3 size = Size();
        return size.x * size.y * size.z;
    }

    bool Empty() const {
        return Volume() == 0;
    }

    vec3 Anchor(const vec3 &anchor) const {
        return m_Min + Size() * anchor;
    }

private:
    vec3 m_Min;
    vec3 m_Max;
};
