#ifndef CPP_MATH_H
#define CPP_MATH_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

typedef float Float;
typedef glm::vec<2, Float, glm::defaultp> Vec2;
typedef glm::vec<3, Float, glm::defaultp> Vec3;
typedef glm::vec<4, Float, glm::defaultp> Vec4;
typedef glm::qua<Float, glm::defaultp> Quat;

struct BezierPoint {
    Vec3 p, t;
    Float roll; // radians
};

// used for KeyToDistance and DistanceToKey
struct ReparamPoint {
    Float key, distance;
};

class Spline {
    std::vector<BezierPoint> m_bezierPoints;
    std::vector<ReparamPoint> m_reparamTable;
    Float m_splineLength = 0.0;

public:
    struct BerierInterp {
        const BezierPoint* points;
        const int i0, i1;
        const Float param;

        BerierInterp(const BezierPoint* _points, int _i0, int _i1, Float _param)
            : points(_points)
            , i0(_i0)
            , i1(_i1)
            , param(_param)
        {
        }

        Vec3 getPos() const;
        Vec3 getDeriv() const; // unnormalized tangent
        void getFrame(Vec3& forward, Vec3& right, Vec3& up) const;
        bool isValid() const { return !!points; }
    };

    Spline();

    Float KeyToDistance(Float key) const;
    Float DistanceToKey(Float distance) const;
    Float GetLength() const { return m_splineLength; }

    BerierInterp GetInterpAtKey(Float splineKey) const;

    // if segmentIndexPrev is INT_MAX -> search on entire spline
    // otherwise search among 3 segments (i = segmentIndexPrev) -> (i - 1,  i,  i + 1)
    Float GetKeyClosestToPosition(const Vec3& worldPos, int segmentIndexPrev = INT_MAX) const;
};

#endif // CPP_MATH_H
