#pragma once

#include <pbrt/cpu/integrators.h>


namespace pbrt
{

// PrtProbeIntegrator Definition
class PrtProbeIntegrator : public Integrator {
  public:
    // RandomWalkIntegrator Public Methods
    PrtProbeIntegrator(int maxDepth, Sampler sampler, Primitive aggregate,
                       std::vector<Light> lights)
        : Integrator(aggregate, lights), maxDepth(maxDepth), samplerPrototype(sampler) {}

    static std::unique_ptr<PrtProbeIntegrator> Create(
        const ParameterDictionary &parameters, Sampler sampler, Primitive aggregate,
        std::vector<Light> lights, const FileLoc *loc);

    void Render();

    std::string ToString() const;

  private:
    Sampler samplerPrototype;
    int maxDepth;
};

}  // namespace pbrt