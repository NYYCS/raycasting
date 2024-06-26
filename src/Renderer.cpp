#include "Renderer.h"

#include "ArgParser.h"
#include "Camera.h"
#include "Image.h"
#include "Ray.h"
#include "VecUtils.h"

#include <limits>
#include <random>
#include <chrono>
  
std::mt19937 generator(42);
std::uniform_real_distribution<float> distribute(0.0f, 1.0f);

Renderer::Renderer(const ArgParser &args) :
    _args(args),
    _scene(args.input_file)
{
}

void
Renderer::Render()
{
    int w = _args.width;
    int h = _args.height;

    if (_args.filter) {
        w = w * 3;
        h = h * 3;
    }

    Image image(w, h);
    Image nimage(w, h);
    Image dimage(w, h);

    // loop through all the pixels in the image
    // generate all the samples

    // This look generates camera rays and callse traceRay.
    // It also write to the color, normal, and depth images.
    // You should understand what this code does.
    Camera* cam = _scene.getCamera();
    for (int y = 0; y < h; ++y) {
        float ndcy = 2 * (y / (h - 1.0f)) - 1.0f;
        for (int x = 0; x < w; ++x) {
            float ndcx = 2 * (x / (w - 1.0f)) - 1.0f;
            // Use PerspectiveCamera to generate a ray.
            // You should understand what generateRay() does.
            Vector3f color;
            Vector3f normal;
            float t;

            if (_args.jitter) {
                for (int i = 0; i < 16; i++)
                {
                    float ox = distribute(generator) * 2 * (1.0f / (w - 1.0f));
                    float oy = distribute(generator) * 2 * (1.0f / (h - 1.0f));

                    Ray r = cam->generateRay(Vector2f(ndcx + ox, ndcy + oy));
                    Hit h;

                    color += traceRay(r, cam->getTMin(), _args.bounces, h) / 16.0f;
                    normal += h.getNormal() / 16.0f;
                    t += h.getT() / 16.0f;
                }
            } else {
                Ray r = cam->generateRay(Vector2f(ndcx, ndcy));
                Hit h;

                color = traceRay(r, cam->getTMin(), _args.bounces, h);
                normal = h.getNormal();
                t = h.getT();
            }

            image.setPixel(x, y, color);
            nimage.setPixel(x, y, (normal + 1.0f) / 2.0f);
            float range = (_args.depth_max - _args.depth_min);
            if (range) {
                dimage.setPixel(x, y, Vector3f((t - _args.depth_min) / range));
            }
        }
    }

    // END SOLN

    if (_args.filter) {
        image = image.downsample();
        dimage = dimage.downsample();
        nimage = nimage.downsample();
    }

    // save the files 
    if (_args.output_file.size()) {
        image.savePNG(_args.output_file);
    }
    if (_args.depth_file.size()) {
        dimage.savePNG(_args.depth_file);
    }
    if (_args.normals_file.size()) {
        nimage.savePNG(_args.normals_file);
    }
}



Vector3f
Renderer::traceRay(const Ray &r,
    float tmin,
    int bounces,
    Hit &h) const
{
    // The starter code only implements basic drawing of sphere primitives.
    // You will implement phong shading, recursive ray tracing, and shadow rays.

    // TODO: IMPLEMENT 
    if (_scene.getGroup()->intersect(r, tmin, h)) {

        Vector3f intensity = _scene.getAmbientLight() * h.getMaterial()->getDiffuseColor();

        Vector3f p = r.pointAtParameter(h.getT());
        Vector3f dirToLight;
        Vector3f lightIntensity;

        float distToLight;
        float epsilon = std::numeric_limits<float>::epsilon();

        for (int i = 0; i < _scene.getNumLights(); i++) {
            Light *light = _scene.getLight(i);
            light->getIllumination(p, dirToLight, lightIntensity, distToLight);

            // Check for if light is blocked by some object

            Ray newRay = Ray(p, dirToLight);
            Hit newHit;

            if (_scene.getGroup()->intersect(newRay, 0.0001, newHit))
                continue;

            intensity += h.getMaterial()->shade(r, h, dirToLight, lightIntensity);
        }

        if (bounces > 0) {
            Vector3f newOrigin = r.pointAtParameter(h.getT());
            Vector3f newDirection = r.getDirection() - 2 * h.getNormal() * Vector3f::dot(r.getDirection(), h.getNormal());
            Ray newRay = Ray(newOrigin, newDirection);
            Hit newHit;
        
            intensity += h.getMaterial()->getSpecularColor() * traceRay(newRay, 0.0001, bounces - 1, newHit);
        }

        return intensity;

    } else {
        return _scene.getBackgroundColor(r.getDirection());
    };
}

