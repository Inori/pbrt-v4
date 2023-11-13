#include <pbrt/cpu/precompute.h>
#include <ext/spherical-harmonics/spherical_harmonics.h>

namespace pbrt
{

std::unique_ptr<PrtProbeIntegrator> PrtProbeIntegrator::Create(
    const ParameterDictionary &parameters, Sampler sampler, Primitive aggregate,
    std::vector<Light> lights, const FileLoc *loc) {
    int maxDepth = parameters.GetOneInt("maxdepth", 5);
    return std::make_unique<PrtProbeIntegrator>(maxDepth, sampler, aggregate, lights);
}

void PrtProbeIntegrator::Render() {

}

std::string PrtProbeIntegrator::ToString() const {
    return StringPrintf("[ PrtProbeIntegrator maxDepth: %d ]", maxDepth);
}

}  // namespace pbrt