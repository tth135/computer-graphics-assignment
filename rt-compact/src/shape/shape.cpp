//
// Created by Sam on 2020/6/15.
//

#include "shape.h"
#include "math/transform.h"
#include "interaction.h"
#include "math/transform.h"

Shape::~Shape() {}


Shape::Shape(const Transform *ObjectToWorld, const Transform *WorldToObject,
             bool reverseOrientation)
        : ObjectToWorld(ObjectToWorld),
          WorldToObject(WorldToObject),
          reverseOrientation(reverseOrientation),
          transformSwapsHandedness(ObjectToWorld->SwapsHandedness()) {
}

Bounds3f Shape::WorldBound() const { return (*ObjectToWorld)(ObjectBound()); }

Interaction Shape::Sample(const Interaction &ref, const Point2f &u,
                          Float *pdf) const {
    Interaction intr = Sample(u, pdf);
    Vector3f wi = intr.p - ref.p;
    if (wi.LengthSquared() == 0)
        *pdf = 0;
    else {
        wi = Normalize(wi);

        *pdf *= DistanceSquared(ref.p, intr.p) / AbsDot(intr.n, -wi);
        if (std::isinf(*pdf)) *pdf = 0.f;
    }
    return intr;
}

Float Shape::Pdf(const Interaction &ref, const Vector3f &wi) const {

    Ray ray = ref.SpawnRay(wi);
    Float tHit;
    SurfaceHit isectLight;

    if (!Intersect(ray, &tHit, &isectLight, false)) return 0;


    Float pdf = DistanceSquared(ref.p, isectLight.p) /
                (AbsDot(isectLight.n, -wi) * Area());
    if (std::isinf(pdf)) pdf = 0.f;
    return pdf;
}


