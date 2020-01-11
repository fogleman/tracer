#include "gi/gi.hpp"

// #include "vendor/json.hpp"
// using json = nlohmann::json;

// std::string ReadFile(const std::string &filename) {
//     std::ifstream in(filename, std::ios::in | std::ios::binary);
//     if (in) {
//         std::string contents;
//         in.seekg(0, std::ios::end);
//         contents.resize(in.tellg());
//         in.seekg(0, std::ios::beg);
//         in.read(&contents[0], contents.size());
//         in.close();
//         return contents;
//     }
//     throw errno;
// }

int main(int argc, char **argv) {
    auto world = std::make_shared<HittableList>();

    const auto red = std::make_shared<Lambertian>(
        std::make_shared<SolidTexture>(glm::vec3(0.65, 0.05, 0.05)));
    world->Add(std::make_shared<Sphere>(glm::vec3(0, 0, 0), 1, red));
    world->Add(std::make_shared<Sphere>(glm::vec3(0, 0, -11), 10, red));

    const auto light = std::make_shared<DiffuseLight>(
        std::make_shared<SolidTexture>(Kelvin(5000) * 50.f));
    world->Add(std::make_shared<Sphere>(glm::vec3(3, 4, 5), 1, light));

    const int width = 1024;
    const int height = 1024;

    const glm::vec3 eye(4, 4, 2);
    const glm::vec3 center(0, 0, 0);
    const glm::vec3 up(0, 0, 1);
    const float fovy = 50;
    const float aspect = float(width) / height;
    const float aperture = 0;
    const float focalDistance = 1;

    Camera camera(eye, center, up, fovy, aspect, aperture, focalDistance);
    Sampler sampler(world);
    Image image(1024, 1024);
    Render(image, sampler, camera, 256, 16);
    image.SavePNG("out.png");

    // if (argc != 2) {
    //     std::cerr << "Usage: gi config.json" << std::endl;
    //     return 1;
    // }

    // const std::string filename(argv[1]);
    // const std::string contents = readFile(filename);
    // const auto config = json::parse(contents);
    // std::cout << config << std::endl;
    // std::cout << config["render"]["width"] << std::endl;
    return 0;
}
