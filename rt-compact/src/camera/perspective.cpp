//
// Created by Sam on 2020/6/16.
//

#include <core/prob.h>
#include "perspective.h"

PerspectiveCamera::PerspectiveCamera(const AnimatedTransform &CameraToWorld,
                                     const Bounds2f &screenWindow,
                                     Float shutterOpen, Float shutterClose,
                                     Float lensRadius, Float focalDistance,
                                     Float fov, Film *film,
                                     const Medium *medium)
        : ProjectiveCamera(CameraToWorld, Perspective(fov, 1e-2f, 1000.f),
                           screenWindow, shutterOpen, shutterClose, lensRadius,
                           focalDistance, film, medium) {
    dxCamera =
            (RasterToCamera(Point3f(1, 0, 0)) - RasterToCamera(Point3f(0, 0, 0)));
    dyCamera =
            (RasterToCamera(Point3f(0, 1, 0)) - RasterToCamera(Point3f(0, 0, 0)));

    Point2i res = film->fullResolution;
    Point3f pMin = RasterToCamera(Point3f(0, 0, 0));
    Point3f pMax = RasterToCamera(Point3f(res.x, res.y, 0));
    pMin /= pMin.z;
    pMax /= pMax.z;
    A = std::abs((pMax.x - pMin.x) * (pMax.y - pMin.y));
}

Float PerspectiveCamera::GenerateRay(const CameraSample &sample,
                                     Ray *ray) const {

    Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
    Point3f pCamera = RasterToCamera(pFilm);
    *ray = Ray(Point3f(0, 0, 0), Normalize(Vector3f(pCamera)));

    if (lensRadius > 0) {

        Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);

        Float ft = focalDistance / ray->d.z;
        Point3f pFocus = (*ray)(ft);

        ray->o = Point3f(pLens.x, pLens.y, 0);
        ray->d = Normalize(pFocus - ray->o);
    }
    ray->time = Lerp(sample.time, shutterOpen, shutterClose);
    ray->medium = medium;
    *ray = CameraToWorld(*ray);
    return 1;
}

Float PerspectiveCamera::GenerateRayDifferential(const CameraSample &sample,
                                                 RayDifferential *ray) const {

    Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
    Point3f pCamera = RasterToCamera(pFilm);
    Vector3f dir = Normalize(Vector3f(pCamera.x, pCamera.y, pCamera.z));
    *ray = RayDifferential(Point3f(0, 0, 0), dir);
    if (lensRadius > 0) {
        Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);

        Float ft = focalDistance / ray->d.z;
        Point3f pFocus = (*ray)(ft);

        ray->o = Point3f(pLens.x, pLens.y, 0);
        ray->d = Normalize(pFocus - ray->o);
    }

    if (lensRadius > 0) {

        Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);
        Vector3f dx = Normalize(Vector3f(pCamera + dxCamera));
        Float ft = focalDistance / dx.z;
        Point3f pFocus = Point3f(0, 0, 0) + (ft * dx);
        ray->rxOrigin = Point3f(pLens.x, pLens.y, 0);
        ray->rxDirection = Normalize(pFocus - ray->rxOrigin);

        Vector3f dy = Normalize(Vector3f(pCamera + dyCamera));
        ft = focalDistance / dy.z;
        pFocus = Point3f(0, 0, 0) + (ft * dy);
        ray->ryOrigin = Point3f(pLens.x, pLens.y, 0);
        ray->ryDirection = Normalize(pFocus - ray->ryOrigin);
    } else {
        ray->rxOrigin = ray->ryOrigin = ray->o;
        ray->rxDirection = Normalize(Vector3f(pCamera) + dxCamera);
        ray->ryDirection = Normalize(Vector3f(pCamera) + dyCamera);
    }
    ray->time = Lerp(sample.time, shutterOpen, shutterClose);
    ray->medium = medium;
    *ray = CameraToWorld(*ray);
    ray->hasDifferentials = true;
    return 1;
}



PerspectiveCamera *CreatePerspectiveCamera(const ParamSet &params,
                                           const AnimatedTransform &cam2world,
                                           Film *film, const Medium *medium) {
    // Extract common camera parameters from _ParamSet_
    Float shutteropen = params.FindOneFloat("shutteropen", 0.f);
    Float shutterclose = params.FindOneFloat("shutterclose", 1.f);
    if (shutterclose < shutteropen) {
        Warning("Shutter close time [%f] < shutter open [%f].  Swapping them.",
                shutterclose, shutteropen);
        std::swap(shutterclose, shutteropen);
    }
    Float lensradius = params.FindOneFloat("lensradius", 0.f);
    Float focaldistance = params.FindOneFloat("focaldistance", 1e6);
    Float frame = params.FindOneFloat(
            "frameaspectratio",
            Float(film->fullResolution.x) / Float(film->fullResolution.y));
    Bounds2f screen;
    if (frame > 1.f) {
        screen.pMin.x = -frame;
        screen.pMax.x = frame;
        screen.pMin.y = -1.f;
        screen.pMax.y = 1.f;
    } else {
        screen.pMin.x = -1.f;
        screen.pMax.x = 1.f;
        screen.pMin.y = -1.f / frame;
        screen.pMax.y = 1.f / frame;
    }
    int swi;
    const Float *sw = params.FindFloat("screenwindow", &swi);
    if (sw) {
        if (swi == 4) {
            screen.pMin.x = sw[0];
            screen.pMax.x = sw[1];
            screen.pMin.y = sw[2];
            screen.pMax.y = sw[3];
        } else
            Error("\"screenwindow\" should have four values");
    }
    Float fov = params.FindOneFloat("fov", 90.);
    Float halffov = params.FindOneFloat("halffov", -1.f);
    if (halffov > 0.f)
        fov = 2.f * halffov;
    return new PerspectiveCamera(cam2world, screen, shutteropen, shutterclose,
                                 lensradius, focaldistance, fov, film, medium);
}
