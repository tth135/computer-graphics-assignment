//
// Created by Sam on 2020/6/16.
//

#include "medium.h"
#include "color.h"


// Media Definitions
PhaseFunction::~PhaseFunction() { }

bool GetMediumScatteringProperties(const std::string &name, Color *sigma_a,
                                   Color *sigma_prime_s) {
    return false;
}


Float HenyeyGreenstein::Sample_p(const Vector3f &wo, Vector3f *wi,
                                 const Point2f &u) const {

    // Compute cos theta for Henyey--Greenstein sample
    Float cosTheta;
    if (std::abs(g) < 1e-3)
        cosTheta = 1 - 2 * u[0];
    else {
        Float sqrTerm = (1 - g * g) / (1 - g + 2 * g * u[0]);
        cosTheta = (1 + g * g - sqrTerm * sqrTerm) / (2 * g);
    }

    // Compute direction wi for Henyey--Greenstein sample
    Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
    Float phi = 2 * Pi * u[1];
    Vector3f v1, v2;
    CoordinateSystem(wo, &v1, &v2);
    *wi = SphericalDirection(sinTheta, cosTheta, phi, v1, v2, -wo);
    return PhaseHG(-cosTheta, g);
}

Float HenyeyGreenstein::p(const Vector3f &wo, const Vector3f &wi) const {

    return PhaseHG(Dot(wo, wi), g);
}
