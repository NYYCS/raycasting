#include "Material.h"
Vector3f Material::shade(const Ray &ray,
    const Hit &hit,
    const Vector3f &dirToLight,
    const Vector3f &lightIntensity)
{   
    Vector3f L = dirToLight;
    Vector3f N = hit.getNormal();
    Vector3f R = ray.getDirection() - 2 * hit.getNormal() * Vector3f::dot(ray.getDirection(), hit.getNormal());

    Vector3f diffuseIntensity = std::max(Vector3f::dot(L, N), 0.0f) * lightIntensity * _diffuseColor;
    Vector3f specularIntesity = std::pow(std::max(Vector3f::dot(L, R), 0.0f), _shininess) * lightIntensity * _specularColor;

    return diffuseIntensity + specularIntesity;
}
