#pragma once

#include <pbrt/cpu/integrators.h>


namespace pbrt
{

struct Voxel {
    Bounds3f box;
    bool overlap;
    int id;

    std::string ToString() const;
};

struct RayGeometryHit {
    unsigned int hitCount = 0;
};

// PrtProbeIntegrator Definition
class PrtProbeIntegrator : public Integrator {
  public:
    // RandomWalkIntegrator Public Methods
    PrtProbeIntegrator(int maxDepth, int volUnit, Sampler sampler, Primitive aggregate,
                       std::vector<Light> lights)
        : Integrator(aggregate, lights),
          maxDepth(maxDepth),
          voxelUnit(volUnit),
          samplerPrototype(sampler) {}

    static std::unique_ptr<PrtProbeIntegrator> Create(
        const ParameterDictionary &parameters, Sampler sampler, Primitive aggregate,
        std::vector<Light> lights, const FileLoc *loc);

    void Render();

    std::string ToString() const;

  private:
    RayGeometryHit IntersectN(const Ray &ray, Float tMax = Infinity);

    pstd::vector<Voxel> VoxelizeScene();

    void SurfaceVoxelize(pstd::vector<Voxel> &voxels);

    void WriteVoxels(const pstd::vector<Voxel> &voxels);

  private:
    Sampler samplerPrototype;
    int maxDepth;
    int voxelUnit;
};

}  // namespace pbrt