#pragma once

#include <chrono>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <random>

#include "config.hpp"

inline real Random() {
    static thread_local std::mt19937 gen(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<real> dist(0, 1);
    return dist(gen);
}

inline vec3 RandomInUnitSphere() {
    while (true) {
        const vec3 p = vec3(
            Random() * 2 - 1,
            Random() * 2 - 1,
            Random() * 2 - 1);
        if (glm::length2(p) < 1) {
            return p;
        }
    }
}

inline vec3 RandomInUnitDisk() {
    while (true) {
        const vec3 p = vec3(
            Random() * 2 - 1,
            Random() * 2 - 1,
            0);
        if (glm::length2(p) < 1) {
            return p;
        }
    }
}

inline vec3 CosineSampleHemisphere() {
    const vec3 d = RandomInUnitDisk();
    const real z = std::sqrt(std::max(real(0), 1 - d.x * d.x - d.y * d.y));
    return vec3(d.x, d.y, z);
}

inline vec3 Reflect(const vec3 &v, const vec3 &n) {
    return v - 2 * glm::dot(v, n) * n;
}

inline bool Refract(const vec3 &v, const vec3 &n, const real ratio, vec3 &refracted) {
    const vec3 uv = glm::normalize(v);
    const real dt = glm::dot(uv, n);
    const real discriminant = 1 - ratio * ratio * (1 - dt * dt);
    if (discriminant <= 0) {
        return false;
    }
    refracted = ratio * (uv - n * dt) - n * std::sqrt(discriminant);
    return true;
}

inline real Schlick(const real cosine, const real index) {
    real r0 = (1 - index) / (1 + index);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow((1 - cosine), 5);
}

inline vec3 HexColor(const int hex) {
    const real r = real((hex >> 16) & 0xff) / 255;
    const real g = real((hex >> 8) & 0xff) / 255;
    const real b = real((hex >> 0) & 0xff) / 255;
    return glm::pow(vec3(r, g, b), vec3(2.2));
}

inline vec3 Kelvin(const real K) {
    real red, green, blue;
    // red
    if (K >= 6600) {
        const real a = 351.97690566805693;
        const real b = 0.114206453784165;
        const real c = -40.25366309332127;
        const real x = K / 100 - 55;
        red = a + b * x + c * std::log(x);
    } else {
        red = 255;
    }
    // green
    if (K >= 6600) {
        const real a = 325.4494125711974;
        const real b = 0.07943456536662342;
        const real c = -28.0852963507957;
        const real x = K / 100 - 50;
        green = a + b * x + c * std::log(x);
    } else if (K >= 1000) {
        const real a = -155.25485562709179;
        const real b = -0.44596950469579133;
        const real c = 104.49216199393888;
        const real x = K / 100 - 2;
        green = a + b * x + c * std::log(x);
    } else {
        green = 0;
    }
    // blue
    if (K >= 6600) {
        blue = 255;
    } else if (K >= 2000) {
        const real a = -254.76935184120902;
        const real b = 0.8274096064007395;
        const real c = 115.67994401066147;
        const real x = K / 100 - 10;
        blue = a + b * x + c * std::log(x);
    } else {
        blue = 0;
    }
    red = std::min(real(1), red / 255);
    green = std::min(real(1), green / 255);
    blue = std::min(real(1), blue / 255);
    return vec3(red, green, blue);
}

inline real Clamp(const real value, const real lo, const real hi) {
    if (value <= lo) {
        return lo;
    }
    if (value >= hi) {
        return hi;
    }
    return value;
}

inline real CosTheta(const vec3 &w) {
    return w.z;
}

inline real AbsCosTheta(const vec3 &w) {
    return std::abs(w.z);
}

inline real SinTheta2(const vec3 &w) {
    return std::max(real(0), 1 - CosTheta(w) * CosTheta(w));
}

inline real SinTheta(const vec3 &w) {
    return std::sqrt(SinTheta2(w));
}

inline real CosPhi(const vec3 &w) {
    const real sintheta = SinTheta(w);
    if (sintheta == 0) {
        return 1;
    }
    return Clamp(w.x / sintheta, -1, 1);
}

inline real SinPhi(const vec3 &w) {
    const real sintheta = SinTheta(w);
    if (sintheta == 0) {
        return 0;
    }
    return Clamp(w.y / sintheta, -1, 1);
}

inline bool SameHemisphere(const vec3 &a, const vec3 &b) {
    return a.z * b.z > 0;
}

inline vec3 Barycentric(
    const vec3 &p1, const vec3 &p2, const vec3 &p3, const vec3 &p)
{
    const vec3 v0 = p2 - p1;
    const vec3 v1 = p3 - p1;
    const vec3 v2 = p - p1;
    const real d00 = glm::dot(v0, v0);
    const real d01 = glm::dot(v0, v1);
    const real d11 = glm::dot(v1, v1);
    const real d20 = glm::dot(v2, v0);
    const real d21 = glm::dot(v2, v1);
    const real d = d00 * d11 - d01 * d01;
    const real v = (d11 * d20 - d01 * d21) / d;
    const real w = (d00 * d21 - d01 * d20) / d;
    const real u = 1 - v - w;
    return vec3(u, v, w);
}
