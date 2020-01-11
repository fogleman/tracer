#pragma once

#include <glm/glm.hpp>
#include <memory>

class Texture {
public:
    virtual glm::vec3 Sample(
        const float u, const float v, const glm::vec3 &p) const = 0;
    virtual ~Texture() {}
};

typedef std::shared_ptr<Texture> P_Texture;

class SolidTexture : public Texture {
public:
    SolidTexture(const glm::vec3 &color) :
        m_Color(color) {}

    virtual glm::vec3 Sample(
        const float u, const float v, const glm::vec3 &p) const
    {
        return m_Color;
    }

private:
    glm::vec3 m_Color;
};
