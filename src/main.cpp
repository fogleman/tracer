#include "tracer/tracer.hpp"

int main(int argc, char **argv) {
    auto world = std::make_shared<HittableList>();

    const RTCDevice device = rtcNewDevice(NULL);

    const auto triangles = LoadBinarySTL(argv[1]);

    const auto material = std::make_shared<OrenNayar>(
        std::make_shared<SolidTexture>(vec3(0.9)), 20);

    world->Add(std::make_shared<EmbreeMesh>(device, triangles, material));

    const auto light = std::make_shared<DiffuseLight>(
        std::make_shared<SolidTexture>(Kelvin(5000) * real(45)));
    world->Add(std::make_shared<Sphere>(vec3(2, 3, 2), 1, light));

    const int width = 1024;
    const int height = 1024;
    const int numFrames = 0;
    const int numSamples = 16;
    const int numThreads = 16;

    const vec3 eye(3, 0, 0);
    const vec3 center(0, 0, 0);
    const vec3 up(0, 0, 1);
    const real fovy = 22;
    const real aspect = real(width) / height;
    const real aperture = 0.01;
    const real focalDistance = 2.75;

    Camera camera(eye, center, up, fovy, aspect, aperture, focalDistance);
    Sampler sampler(world);
    Image image(width, height);

    std::cout << "start" << std::endl;

    for (int i = 1; ; i++) {
        Render(image, sampler, camera, numSamples, numThreads);

        char path[100];
        snprintf(path, 100, "%08d.png", i - 1);

        image.SavePNG(path);
        std::cout << path << std::endl;

        if (numFrames > 0 && i == numFrames) {
            break;
        }
    }

    return 0;
}
