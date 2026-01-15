#pragma once

#include <cmath>

namespace Easing {

inline double linear(double t) {
    return t;
}

inline double easeInQuad(double t) {
    return t * t;
}

inline double easeOutQuad(double t) {
    return t * (2.0 - t);
}

inline double easeInOutQuad(double t) {
    return t < 0.5 ? 2.0 * t * t : -1.0 + (4.0 - 2.0 * t) * t;
}

inline double easeInCubic(double t) {
    return t * t * t;
}

inline double easeOutCubic(double t) {
    double t1 = t - 1.0;
    return t1 * t1 * t1 + 1.0;
}

inline double easeInOutCubic(double t) {
    return t < 0.5
        ? 4.0 * t * t * t
        : (t - 1.0) * (2.0 * t - 2.0) * (2.0 * t - 2.0) + 1.0;
}

inline double easeInQuart(double t) {
    return t * t * t * t;
}

inline double easeOutQuart(double t) {
    double t1 = t - 1.0;
    return 1.0 - t1 * t1 * t1 * t1;
}

inline double easeInOutQuart(double t) {
    if (t < 0.5) {
        return 8.0 * t * t * t * t;
    } else {
        double t1 = t - 1.0;
        return 1.0 - 8.0 * t1 * t1 * t1 * t1;
    }
}

inline double easeInQuint(double t) {
    return t * t * t * t * t;
}

inline double easeOutQuint(double t) {
    double t1 = t - 1.0;
    return 1.0 + t1 * t1 * t1 * t1 * t1;
}

inline double easeInOutQuint(double t) {
    if (t < 0.5) {
        return 16.0 * t * t * t * t * t;
    } else {
        double t1 = t - 1.0;
        return 1.0 + 16.0 * t1 * t1 * t1 * t1 * t1;
    }
}

inline double easeInSine(double t) {
    return 1.0 - std::cos(t * M_PI / 2.0);
}

inline double easeOutSine(double t) {
    return std::sin(t * M_PI / 2.0);
}

inline double easeInOutSine(double t) {
    return -(std::cos(M_PI * t) - 1.0) / 2.0;
}

inline double easeInExpo(double t) {
    return t == 0.0 ? 0.0 : std::pow(2.0, 10.0 * (t - 1.0));
}

inline double easeOutExpo(double t) {
    return t == 1.0 ? 1.0 : 1.0 - std::pow(2.0, -10.0 * t);
}

inline double easeInOutExpo(double t) {
    if (t == 0.0) return 0.0;
    if (t == 1.0) return 1.0;
    if (t < 0.5) {
        return std::pow(2.0, 20.0 * t - 10.0) / 2.0;
    } else {
        return (2.0 - std::pow(2.0, -20.0 * t + 10.0)) / 2.0;
    }
}

} // namespace Easing
