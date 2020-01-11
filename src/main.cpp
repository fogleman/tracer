#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "vendor/stb_image_write.h"
#include "vendor/json.hpp"

#include "gi/camera.hpp"
#include "gi/colormap.hpp"
#include "gi/cube.hpp"
#include "gi/hit.hpp"
#include "gi/image.hpp"
#include "gi/material.hpp"
#include "gi/medium.hpp"
#include "gi/microfacet.hpp"
#include "gi/onb.hpp"
#include "gi/ray.hpp"
#include "gi/sampler.hpp"
#include "gi/sphere.hpp"
#include "gi/stl.hpp"
#include "gi/texture.hpp"
#include "gi/util.hpp"

using json = nlohmann::json;

std::string readFile(std::string filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return contents;
    }
    throw errno;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: gi config.json" << std::endl;
        return 1;
    }

    const std::string filename(argv[1]);
    const std::string contents = readFile(filename);
    const auto config = json::parse(contents);
    std::cout << config << std::endl;
    std::cout << config["render"]["width"] << std::endl;
    // std::string path = "out.png";
    // const int w = 1024;
    // const int h = 1024;
    // std::vector<uint8_t> data(w * h * 3);
    // stbi_write_png(path.c_str(), w, h, 3, data.data(), w * 3);
    return 0;
}
