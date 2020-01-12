#pragma once

#include <thread>
#include <pmmintrin.h>
#include <xmmintrin.h>

#include "camera.hpp"
#include "config.hpp"
#include "image.hpp"
#include "ray.hpp"
#include "sampler.hpp"
#include "util.hpp"

void Render(
    Image &image, const Sampler &sampler, const Camera &camera,
    const int numSamples, const int numThreads)
{
    const int w = image.Width();
    const int h = image.Height();
    const int wn = numThreads;

    std::vector<std::thread> threads;
    for (int wi = 0; wi < wn; wi++) {
        threads.push_back(std::thread([&](int i) {
            _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
            _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
            for (int y = i; y < h; y += wn) {
                for (int x = 0; x < w; x++) {
                    vec3 c(0);
                    for (int s = 0; s < numSamples; s++) {
                        const real u = (x + Random()) / w;
                        const real v = (y + Random()) / h;
                        const Ray ray = camera.MakeRay(u, 1 - v);
                        c += sampler.Sample(ray);
                    }
                    image.Add(x, y, c);
                }
            }
        }, wi));
    }

    for (int wi = 0; wi < wn; wi++) {
        threads[wi].join();
    }

    image.IncrementSampleCount(numSamples);
}
