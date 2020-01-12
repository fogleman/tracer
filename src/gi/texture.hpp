#pragma once

#include <memory>

#include "config.hpp"

class Texture {
public:
    virtual vec3 Sample(
        const real u, const real v, const vec3 &p) const = 0;
    virtual ~Texture() {}
};

typedef std::shared_ptr<Texture> P_Texture;

class SolidTexture : public Texture {
public:
    SolidTexture(const vec3 &color) :
        m_Color(color) {}

    virtual vec3 Sample(
        const real u, const real v, const vec3 &p) const
    {
        return m_Color;
    }

private:
    vec3 m_Color;
};
