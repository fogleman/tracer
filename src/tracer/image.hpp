#pragma once

#include <fstream>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb_image_write.h"

#include "config.hpp"

class Pixel {
public:
    Pixel() :
        m_NumSamples(0), m_Mean(0), m_Variance(0) {}

    vec3 Color() const {
        return m_Mean;
    }

    vec3 Variance() const {
        if (m_NumSamples < 2) {
            return vec3(0);
        }
        return m_Variance / real(m_NumSamples - 1);
    }

    vec3 StandardDeviation() const {
        return glm::sqrt(Variance());
    }

    void AddSample(const vec3 &c) {
        m_NumSamples++;
        if (m_NumSamples == 1) {
            m_Mean = c;
            return;
        }
        const vec3 m = m_Mean;
        m_Mean += (c - m_Mean) / real(m_NumSamples);
        m_Variance += (c - m) * (c - m_Mean);
    }

private:
    int m_NumSamples;
    vec3 m_Mean;
    vec3 m_Variance;
};

class Image {
public:
    Image(int width, int height) :
        m_Width(width), m_Height(height)
    {
        m_Pixels.resize(width * height);
    }

    int Width() const {
        return m_Width;
    }

    int Height() const {
        return m_Height;
    }

    void AddSample(const int x, const int y, const vec3 &c) {
        m_Pixels[y * m_Width + x].AddSample(c);
    }

    vec3 Color(const int x, const int y) const {
        return m_Pixels[y * m_Width + x].Color();
    }

    vec3 Variance(const int x, const int y) const {
        return m_Pixels[y * m_Width + x].Variance();
    }

    vec3 StandardDeviation(const int x, const int y) const {
        return m_Pixels[y * m_Width + x].StandardDeviation();
    }

    void SavePNG(const std::string &path) const {
        const vec3 exponent = vec3(1 / 2.2);
        std::vector<uint8_t> data;
        data.reserve(m_Width * m_Height * 3);
        int i = 0;
        for (int y = 0; y < m_Height; y++) {
            for (int x = 0; x < m_Width; x++) {
                const vec3 c = glm::pow(m_Pixels[i++].Color(), exponent);
                data.push_back(std::min(c.r * 256, real(255)));
                data.push_back(std::min(c.g * 256, real(255)));
                data.push_back(std::min(c.b * 256, real(255)));
            }
        }
        stbi_write_png(
            path.c_str(), m_Width, m_Height, 3, data.data(), m_Width * 3);
    }

    void SavePPM(const std::string &path) const {
        const vec3 exponent = vec3(1 / 2.2);
        std::ofstream out(path);
        out << "P3\n";
        out << m_Width << " " << m_Height << "\n";
        out << 255 << "\n";
        int i = 0;
        for (int y = 0; y < m_Height; y++) {
            for (int x = 0; x < m_Width; x++) {
                const vec3 c = glm::pow(m_Pixels[i++].Color(), exponent);
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
    std::vector<Pixel> m_Pixels;
};
