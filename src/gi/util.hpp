#pragma once

#include <chrono>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <random>

inline float Random() {
    static thread_local std::mt19937 gen(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> dist(0, 1);
    return dist(gen);
}

inline glm::vec3 RandomInUnitSphere() {
    while (true) {
        const glm::vec3 p = glm::vec3(
            Random() * 2 - 1,
            Random() * 2 - 1,
            Random() * 2 - 1);
        if (glm::length2(p) < 1) {
            return p;
        }
    }
}

inline glm::vec3 RandomInUnitDisk() {
    while (true) {
        const glm::vec3 p = glm::vec3(
            Random() * 2 - 1,
            Random() * 2 - 1,
            0);
        if (glm::length2(p) < 1) {
            return p;
        }
    }
}

inline glm::vec3 CosineSampleHemisphere() {
    const glm::vec3 d = RandomInUnitDisk();
    const float z = std::sqrt(std::max(0.f, 1 - d.x * d.x - d.y * d.y));
    return glm::vec3(d.x, d.y, z);
}

inline glm::vec3 Reflect(const glm::vec3 &v, const glm::vec3 &n) {
    return v - 2 * glm::dot(v, n) * n;
}

inline bool Refract(const glm::vec3 &v, const glm::vec3 &n, const float ratio, glm::vec3 &refracted) {
    const glm::vec3 uv = glm::normalize(v);
    const float dt = glm::dot(uv, n);
    const float discriminant = 1 - ratio * ratio * (1 - dt * dt);
    if (discriminant <= 0) {
        return false;
    }
    refracted = ratio * (uv - n * dt) - n * std::sqrt(discriminant);
    return true;
}

inline float Schlick(const float cosine, const float index) {
    float r0 = (1 - index) / (1 + index);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow((1 - cosine), 5);
}

inline glm::vec3 HexColor(const int hex) {
    const float r = float((hex >> 16) & 0xff) / 255;
    const float g = float((hex >> 8) & 0xff) / 255;
    const float b = float((hex >> 0) & 0xff) / 255;
    return glm::pow(glm::vec3(r, g, b), glm::vec3(2.2));
}

inline glm::vec3 Kelvin(const float K) {
    float red, green, blue;
    // red
    if (K >= 6600) {
        const float a = 351.97690566805693;
        const float b = 0.114206453784165;
        const float c = -40.25366309332127;
        const float x = K / 100 - 55;
        red = a + b * x + c * std::log(x);
    } else {
        red = 255;
    }
    // green
    if (K >= 6600) {
        const float a = 325.4494125711974;
        const float b = 0.07943456536662342;
        const float c = -28.0852963507957;
        const float x = K / 100 - 50;
        green = a + b * x + c * std::log(x);
    } else if (K >= 1000) {
        const float a = -155.25485562709179;
        const float b = -0.44596950469579133;
        const float c = 104.49216199393888;
        const float x = K / 100 - 2;
        green = a + b * x + c * std::log(x);
    } else {
        green = 0;
    }
    // blue
    if (K >= 6600) {
        blue = 255;
    } else if (K >= 2000) {
        const float a = -254.76935184120902;
        const float b = 0.8274096064007395;
        const float c = 115.67994401066147;
        const float x = K / 100 - 10;
        blue = a + b * x + c * std::log(x);
    } else {
        blue = 0;
    }
    red = std::min(1.f, red / 255);
    green = std::min(1.f, green / 255);
    blue = std::min(1.f, blue / 255);
    return glm::vec3(red, green, blue);
}

inline float Clamp(const float value, const float lo, const float hi) {
    if (value <= lo) {
        return lo;
    }
    if (value >= hi) {
        return hi;
    }
    return value;
}

inline float CosTheta(const glm::vec3 &w) {
    return w.z;
}

inline float AbsCosTheta(const glm::vec3 &w) {
    return std::abs(w.z);
}

inline float SinTheta2(const glm::vec3 &w) {
    return std::max(0.f, 1 - CosTheta(w) * CosTheta(w));
}

inline float SinTheta(const glm::vec3 &w) {
    return std::sqrt(SinTheta2(w));
}

inline float CosPhi(const glm::vec3 &w) {
    const float sintheta = SinTheta(w);
    if (sintheta == 0) {
        return 1;
    }
    return Clamp(w.x / sintheta, -1, 1);
}

inline float SinPhi(const glm::vec3 &w) {
    const float sintheta = SinTheta(w);
    if (sintheta == 0) {
        return 0;
    }
    return Clamp(w.y / sintheta, -1, 1);
}

inline bool SameHemisphere(const glm::vec3 &a, const glm::vec3 &b) {
    return a.z * b.z > 0;
}
