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
    // mesh->SmoothNormals();
    mesh->FitInUnitCube();
    mesh->Rotate(glm::radians(90.f), up);

    // auto blinn = std::make_shared<BlinnDistribution>(100);
    // auto albedo = std::make_shared<SolidTexture>(HexColor(0xFFF0A5));
    // auto material = std::make_shared<Microfacet>(albedo, blinn, 2);
    // auto material = std::make_shared<FresnelBlend>(albedo, albedo, blinn);
    // auto material = std::make_shared<SpecularReflection>(albedo, 1.5);
    auto material = std::make_shared<Lambertian>(
        std::make_shared<SolidTexture>(HexColor(0x7ECEFD) * 0.8));

    world->Add(std::make_shared<EmbreeMesh>(device, mesh, material));

    const real z = mesh->BoundingBox().Min().z;
    auto white = std::make_shared<OrenNayar>(
        std::make_shared<SolidTexture>(vec3(0.9)), 20);
    world->Add(std::make_shared<Cube>(vec3(-100), vec3(100, 100, z), white));

    auto light = std::make_shared<DiffuseLight>(
        std::make_shared<SolidTexture>(Kelvin(5000) * real(15)));
    world->Add(std::make_shared<Sphere>(vec3(3, 2, 2), 1, light));

    Camera camera(eye, center, up, fovy, aspect, aperture, focalDistance);
    Sampler sampler(world);
    Image image(width, height);

    for (int i = 1; ; i++) {
        char path[100];
        snprintf(path, 100, "%08d.png", i - 1);
        std::cout << path << std::endl;

        Render(image, sampler, camera, numSamples, numThreads);
        image.SavePNG(path);

        if (numFrames > 0 && i == numFrames) {
            break;
        }
    }

    return 0;
}
