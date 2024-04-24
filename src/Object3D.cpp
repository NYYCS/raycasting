#include "Object3D.h"
#include "VecUtils.h"

bool Sphere::intersect(const Ray &r, float tmin, Hit &h) const
{
    // BEGIN STARTER

    // We provide sphere intersection code for you.
    // You should model other intersection implementations after this one.

    // Locate intersection point ( 2 pts )
    const Vector3f &rayOrigin = r.getOrigin(); //Ray origin in the world coordinate
    const Vector3f &dir = r.getDirection();

    Vector3f origin = rayOrigin - _center;      //Ray origin in the sphere coordinate

    float a = dir.absSquared();
    float b = 2 * Vector3f::dot(dir, origin);
    float c = origin.absSquared() - _radius * _radius;

    // no intersection
    if (b * b - 4 * a * c < 0) {
        return false;
    }

    float d = sqrt(b * b - 4 * a * c);

    float tplus = (-b + d) / (2.0f*a);
    float tminus = (-b - d) / (2.0f*a);

    // the two intersections are at the camera back
    if ((tplus < tmin) && (tminus < tmin)) {
        return false;
    }

    float t = 10000;
    // the two intersections are at the camera front
    if (tminus > tmin) {
        t = tminus;
    }

    // one intersection at the front. one at the back 
    if ((tplus > tmin) && (tminus < tmin)) {
        t = tplus;
    }

    if (t < h.getT()) {
        Vector3f normal = r.pointAtParameter(t) - _center;
        normal = normal.normalized();
        h.set(t, this->material, normal);
        return true;
    }
    // END STARTER
    return false;
}

// Add object to group
void Group::addObject(Object3D *obj) {
    m_members.push_back(obj);
}

// Return number of objects in group
int Group::getGroupSize() const {
    return (int)m_members.size();
}

bool Group::intersect(const Ray &r, float tmin, Hit &h) const
{
    // BEGIN STARTER
    // we implemented this for you
    bool hit = false;
    for (Object3D* o : m_members) {
        if (o->intersect(r, tmin, h)) {
            hit = true;
        }
    }
    return hit;
    // END STARTER
}


Plane::Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
    _normal = normal;
    _p = _normal * d;
    _d = d;
}
bool Plane::intersect(const Ray &r, float tmin, Hit &h) const
{
    // TODO implement
    Vector3f o = r.getOrigin();
    Vector3f d = r.getDirection();

    float t = Vector3f::dot(_p - o, _normal) / Vector3f::dot(d, _normal);
    
    if (t < 0 || t < tmin)
        return false;

    if (t > h.getT())
        return false;

    h.set(t, material, _normal);

    return true;
}
bool Triangle::intersect(const Ray &r, float tmin, Hit &h) const 
{
    // TODO implement
    Vector3f o = r.getOrigin();
    Vector3f d = r.getDirection();

    Vector3f 
        v1 = getVertex(0),
        v2 = getVertex(1),
        v3 = getVertex(2);

    Vector3f 
        n1 = getNormal(0),
        n2 = getNormal(1),
        n3 = getNormal(2);

    // Ray parallel to plane check
    if (Vector3f::dot(d, n1) == 0)
        return false;
    
    Matrix3f A = { -d, v2 - v1, v3 - v1 };
    Vector3f B = o - v1;

    float det = A.determinant();

    Vector3f x = Vector3f(0, 0, 0);

    for (int i = 0; i < 3; i++)
    {
        Matrix3f Ai = Matrix3f(A);
        Ai.setCol(i, B);

        x[i] = Ai.determinant() / det;
    }

    float t = x[0];
    float u = x[1];
    float w = x[2];

    if (t < 0 || t < tmin)
        return false;

    if (u < 0 || u > 1 || w < 0 || w > 1)
        return false;

    if (u + w > 1)
        return false;

    if (t > h.getT())
        return false;

    Vector3f normal = (1 - u - w) * n1 + u * n2 + w * n3;

    h.set(t, material, normal);

    return true;
}


Transform::Transform(const Matrix4f &m,
    Object3D *obj) : _object(obj) {
    // TODO implement Transform constructor
    _m = m;
}
bool Transform::intersect(const Ray &r0, float tmin, Hit &h) const
{
    Vector3f o0 = r0.getOrigin();
    Vector3f d0 = r0.getDirection();

    Vector3f o1 = VecUtils::transformPoint(_m.inverse() ,o0);
    Vector3f d1 = VecUtils::transformDirection(_m.inverse(), d0);

    Ray r1 = Ray(o1, d1);
    Hit h1;
    
    bool hit = _object->intersect(r1, tmin, h1);

    if (!hit)
        return false;


    Vector3f n0 = VecUtils::transformDirection(_m.inverse().transposed(), h1.getNormal()).normalized();

    if (h1.getT() > h.getT())
        return false;
    
    h.set(h1.getT(), h1.getMaterial(), n0);

    return true;
}