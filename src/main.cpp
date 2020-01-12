#include "tracer/tracer.hpp"

const int width = 1600;
const int height = 1600;
const int numFrames = 0;
const int numSamples = 16;
const int numThreads = 16;

const vec3 eye(3, 0, 1);
const vec3 center(0, 0, 0);
const vec3 up(0, 0, 1);
const real fovy = 25;
const real aspect = real(width) / height;
const real aperture = 0.01;
const real focalDistance = 2.75;

int main(int argc, char **argv) {
    auto world = std::make_shared<HittableList>();

    RTCDevice device = rtcNewDevice(NULL);

    auto mesh = LoadBinarySTL(argv[1]);
    mesh->FitInUnitCube();
    mesh->Rotate(glm::radians(90.f), up);

    auto material = std::make_shared<OrenNayar>(
        std::make_shared<SolidTexture>(HexColor(0xFFF0A5)), 20);

    world->Add(std::make_shared<EmbreeMesh>(device, mesh, material));

    auto light = std::make_shared<DiffuseLight>(
        std::make_shared<SolidTexture>(Kelvin(5000) * real(30)));
    world->Add(std::make_shared<Sphere>(vec3(3, 2, 2), 1, light));

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
