#include "Material.h"
Vector3f Material::shade(const Ray &ray,
    const Hit &hit,
    const Vector3f &dirToLight,
    const Vector3f &lightIntensity)
{   
    return Vector3f(0, 0, 0);
}
