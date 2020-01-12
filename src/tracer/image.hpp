#pragma once

#include <fstream>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb_image_write.h"

#include "config.hpp"

class Image {
public:
    Image(int width, int height) :
        m_Width(width), m_Height(height), m_Samples(0)
    {
        m_Data.resize(width * height);
    }

    int Width() const {
        return m_Width;
    }

    int Height() const {
        return m_Height;
    }

    void IncrementSampleCount(const int numSamples) {
        m_Samples += numSamples;
    }

    const vec3 &Get(int x, int y) const {
        return m_Data[y * m_Width + x];
    }

    void Set(int x, int y, const vec3 &c) {
        m_Data[y * m_Width + x] = c;
    }

    void Add(int x, int y, const vec3 &c) {
        const int i = y * m_Width + x;
        m_Data[i] = m_Data[i] + c;
    }

    void SavePNG(const std::string &path) const {
        const real multiplier = 1.f / m_Samples;
        const real exponent = 1.f / 2.2f;
        std::vector<uint8_t> data;
        data.reserve(m_Width * m_Height * 3);
        int i = 0;
        for (int y = 0; y < m_Height; y++) {
            for (int x = 0; x < m_Width; x++) {
                const vec3 &c = glm::pow(
                    m_Data[i++] * multiplier, vec3(exponent));
                data.push_back(std::min(c.r * 256, real(255)));
                data.push_back(std::min(c.g * 256, real(255)));
                data.push_back(std::min(c.b * 256, real(255)));
            }
        }
        stbi_write_png(
            path.c_str(), m_Width, m_Height, 3, data.data(), m_Width * 3);
    }

    void SavePPM(const std::string &path) const {
        const real multiplier = 1.f / m_Samples;
        const real exponent = 1.f / 2.2f;
        std::ofstream out(path);
        out << "P3\n";
        out << m_Width << " " << m_Height << "\n";
        out << 255 << "\n";
        int i = 0;
        for (int y = 0; y < m_Height; y++) {
            for (int x = 0; x < m_Width; x++) {
                const vec3 &c = glm::pow(
                    m_Data[i++] * multiplier, vec3(exponent));
                const int r = std::min(c.r * 256, real(255));
                const int g = std::min(c.g * 256, real(255));
                const int b = std::min(c.b * 256, real(255));
                out << r << " " << g << " " << b << "\n";
            }
        }
        out.close();
    }

private:
    int m_Width;
    int m_Height;
    int m_Samples;
    std::vector<vec3> m_Data;
};
