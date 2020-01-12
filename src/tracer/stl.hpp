#pragma once

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <string>
#include <vector>

#include "config.hpp"
#include "triangle.hpp"

using namespace boost::interprocess;

std::vector<Triangle> LoadBinarySTL(std::string path) {
    file_mapping fm(path.c_str(), read_only);
    mapped_region mr(fm, read_only);
    uint8_t *src = (uint8_t *)mr.get_address();
    const int numBytes = mr.get_size();
    const int numTriangles = std::max(0, (numBytes - 84) / 50);
    std::vector<Triangle> triangles;
    triangles.reserve(numTriangles);
    src += 96;
    for (int i = 0; i < numTriangles; i++) {
        const float *p = (float *)src;
        const vec3 v1 = vec3(p[0], p[1], p[2]);
        const vec3 v2 = vec3(p[3], p[4], p[5]);
        const vec3 v3 = vec3(p[6], p[7], p[8]);
        triangles.emplace_back(v1, v2, v3);
        src += 50;
    }
    return triangles;
}
