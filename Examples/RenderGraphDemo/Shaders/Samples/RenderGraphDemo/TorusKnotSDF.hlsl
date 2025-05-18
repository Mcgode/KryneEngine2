// SDF functions
// Parametric equation of the (p, q)-Torus Knot.
float3 torusKnot(in float t, in float R, in float r, in float pParam, in float qParam) {
    return float3(
        R * (2 + cos(qParam * t)) * cos(pParam * t) * 0.5,
        R * (2 + cos(qParam * t)) * sin(pParam * t) * 0.5,
        R * sin(qParam * t) * 0.5
    );
}

// First derivative of the parametric equation.
float3 torusKnotDerived(in float t, in float R, in float r, in float pParam, in float qParam) {
    const float pt = pParam * t;
    const float qt = qParam * t;

    return float3(
        -0.5 * pParam * R * sin(pt) * (cos(qt) + 2) - 0.5 * qParam * R * cos(pt) * sin(qt),
         0.5 * pParam * R * cos(pt) * (cos(qt) + 2) - 0.5 * qParam * R * sin(pt) * sin(qt),
         0.5 * qParam * R * cos(qt)
    );
}

// Distance between (0, 0, 0) and a line through p with direction d.
float distanceOriginLine(in float3 p, in float3 d) {
    return length(cross(p, p + normalize(d)));
}

// Distance between p and torusKnot(t).
// The better the estimate of t is, the more accurate this distance will be.
float distancePointCurve(in float3 p, in float R, in float r, in int pParam, in int qParam, in float s, in float t) {
    return distanceOriginLine(p - s * torusKnot(t, R, r, pParam, qParam), torusKnotDerived(t, R, r, pParam, qParam));
}

// Rough estimate of t, trying to get torusKnot(t) near p.
float roughInverseTorusKnot(in float3 p, in int pParam, in int qParam) {
    return atan2(p.y, p.x) / float(pParam);
}

// SDF for Torus Knot.
// R = major radius (size of the torus)
// r = minor radius (thickness of the wire)
// pParam, qParam = parameters defining the (p, q)-torus knot
float sdTorusKnot(in float3 p, in float R, in float r, in int pParam, in int qParam) {
    float t = roughInverseTorusKnot(p, pParam, qParam);

    // There are multiple points on the curve that match t; calculate both.
    float d1 = distancePointCurve(p, R, r, pParam, qParam, 1.0, t);
    float d2 = distancePointCurve(p, R, r, pParam, qParam, 1.0, t + 3.141592653589793); // t + PI

    // Take whichever is closest to p.
    return min(d1, d2) - r;
}