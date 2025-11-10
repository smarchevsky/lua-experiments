#version 430

out vec3 finalColor;
in vec2 inUV;

uniform vec2 carPos;
uniform vec2 carDir= vec2(1,0);
uniform vec3 carInput;
uniform float cameraRotation;
uniform float cameraZoom = 500.01;


#define PI 3.1415926535

#define ROAD_SEGMENT_NUM     8
vec2 roadSplineSegments[32] = vec2[32] (
vec2(4665.0, 59665.0),   vec2(6812.0, 81281.0),   vec2(-789.3, 120696.0),  vec2(28765.0, 116565.0),
vec2(28765.0, 116565.0), vec2(58319.3, 112434.0), vec2(8936.1, 16317.1),   vec2(33480.0, 6885.0),
vec2(33480.0, 6885.0),   vec2(58023.9, -2547.1),  vec2(36059.8, -16637.8), vec2(62045.0, -25690.0),
vec2(62045.0, -25690.0), vec2(88030.2, -34742.2), vec2(83957.4, -71527.8), vec2(52280.0, -69975.0),
vec2(52280.0, -69975.0), vec2(20602.6, -68422.2), vec2(8614.2, -55210.4),  vec2(6260.0, -35315.0),
vec2(6260.0, -35315.0),  vec2(3905.8, -15419.6),  vec2(18141.0, -7554.9),  vec2(14725.0, 15065.0),
vec2(14725.0, 15065.0),  vec2(11309.0, 37684.9),  vec2(-21242.1, 16235.7), vec2(-24075.0, 30843.5),
vec2(-24075.0, 30843.5), vec2(-26907.9, 45451.3), vec2(2518.0, 38049.0),   vec2(4665.0, 59665.0)
);




#define FROM_KPH(x) (float(x) / 0.036f)
const float gravityMag = 980.; // cm/s
const float carMass = 1000.;
const float carGravityForce = gravityMag * carMass;
const float maxPower = 1200. * 1000. * 10000.; // KW * 1000 * unit system scale

const float roadHalfWidth = 800.;
const float steeringScale = 7.f;
const float maxZoom = 60000.;
const float minZoom = 400.;
const float initZoom = 1800.;

const float trackLength = 512730.;
const vec2 trackCenter = vec2(28000, -40000);


float saturate( float x ) { return clamp( x, 0.0, 1.0 ); }
vec3 saturate( vec3 x ) { return clamp( x, vec3(0), vec3(1) ); }

vec3 heatMap(float val) { float level = val * PI / 2.; return vec3( sin(level), sin(level*2.), cos(level)); }

float sigmoid(float x, float power) {
    float tmp = pow(abs(x), power);
    return isinf(tmp) ? sign(x) : (x / pow(1. + tmp, 1. / power));
}

vec3 sigmoid(vec3 x, float power) {
    vec3 tmp = pow(abs(x), vec3(power));
    return (x / pow(vec3(1.) + tmp, vec3(1. / power))); // no check for inf
}

mat2 rotate2d(float angle) { float c = cos(angle); float s = sin(angle); return mat2(c,-s,s,c); }
mat2 matFromDir(vec2 dir) { return mat2(dir.x, -dir.y, dir.y, dir.x); }
float toAngle(vec2 dir) { return atan(dir.y, dir.x); }

float sdSegment( in vec2 p, in vec2 a, in vec2 b ) {vec2 pa = p-a, ba = b-a; float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 ); return length( pa - ba*h ); }
float sdBox(vec2 p, vec2 b) { vec2 d = abs(p)-b; return length(max(d,0.0)) + min(max(d.x,d.y),0.0); }
float sdCircle(vec2 p, float r) { return length(p) - r; }
float sUnion(float a, float b, float k) { float h = max( k-abs(a-b), 0.0 ) / k; return min( a, b ) - h*h*h*k*(1.0/6.0); }


// for city and road noise
vec2 hash22(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}

float noise( in vec2 p )
{
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

        vec2  i = floor( p + (p.x+p.y)*K1 );
    vec2  a = p - i + (i.x+i.y)*K2;
    float m = step(a.y,a.x);
    vec2  o = vec2(m,1.0-m);
    vec2  b = a - o + K2;
        vec2  c = a - 1.0 + 2.0*K2;
    vec3  h = max( 0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
        vec3  n = h*h*h*h*vec3( dot(a,hash22(i+0.0)), dot(b,hash22(i+o)), dot(c,hash22(i+1.0)));
    return dot( n, vec3(70.0) );
}

vec2 bezierPos(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t) {
    float invT = 1.0 - t; float t2 = t * t; float invT2 = invT * invT;
    float t3 = t2 * t; float invT3 = invT2 * invT;
    return  invT3 * p0 + 3.0 * invT2 * t * p1 + 3.0 * invT * t2 * p2 + t3 * p3;
}

vec2 bezierDeriv(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t) {
    float invT = 1.0 - t;
    return 3.0 * invT * invT * (p1 - p0) + 6.0 * invT * t * (p2 - p1) + 3.0 * t * t * (p3 - p2);
}

float normalizeRange(float inMin, float inMax, float v)
{
    float d = inMax - inMin;
    return (d == 0.) ? (v >= inMax ? 1. : 0.) : (v - inMin) / d;
}

float normalizeRangeClamped(float inMin, float inMax, float v)
{
    return clamp(normalizeRange(inMin, inMax, v), 0., 1.);
}

float remapRangeClamped(float inMin, float inMax, float outMin, float outMax, float v)
{
    float r = normalizeRangeClamped(inMin, inMax, v);
    return mix(outMin, outMax, r);
}

// https://www.shadertoy.com/view/7lsBW2
float cbrt(in float x) { return sign(x) * pow(abs(x), 1.0 / 3.0); }
int solveQuartic(in float a, in float b, in float c, in float d, in float e, inout vec4 roots) {
    b /= a; c /= a; d /= a; e /= a; // Divide by leading coefficient to make it 1

    // Depress the quartic to x^4 + px^2 + qx + r by substituting x-b/4a
    // This can be found by substituting x+u and the solving for the value
    // of u that makes the t^3 term go away
    float bb = b * b;
    float p = (8.0 * c - 3.0 * bb) / 8.0;
    float q = (8.0 * d - 4.0 * c * b + bb * b) / 8.0;
    float r = (256.0 * e - 64.0 * d * b + 16.0 * c * bb - 3.0 * bb * bb) / 256.0;
    int n = 0; // Root counter

    // Solve for a root to (t^2)^3 + 2p(t^2)^2 + (p^2 - 4r)(t^2) - q^2 which resolves the
    // system of equations relating the product of two quadratics to the depressed quartic
    float ra =  2.0 * p;
    float rb =  p * p - 4.0 * r;
    float rc = -q * q;

    // Depress using the method above
    float ru = ra / 3.0;
    float rp = rb - ra * ru;
    float rq = rc - (rb - 2.0 * ra * ra / 9.0) * ru;

    float lambda;
    float rh = 0.25 * rq * rq + rp * rp * rp / 27.0;
    if (rh > 0.0) { // Use Cardano's formula in the case of one real root
        rh = sqrt(rh);
        float ro = -0.5 * rq;
        lambda = cbrt(ro - rh) + cbrt(ro + rh) - ru;
    }

    else { // Use complex arithmetic in the case of three real roots
        float rm = sqrt(-rp / 3.0);
        lambda = -2.0 * rm * sin(asin(1.5 * rq / (rp * rm)) / 3.0) - ru;
    }

    // Newton iteration to fix numerical problems (using Horners method)
    // Suggested by @NinjaKoala
    for(int i=0; i < 2; i++) {
        float a_2 = ra + lambda;
        float a_1 = rb + lambda * a_2;
        float b_2 = a_2 + lambda;

        float f = rc + lambda * a_1; // Evaluation of λ^3 + ra * λ^2 + rb * λ + rc
        float f1 = a_1 + lambda * b_2; // Derivative

        lambda -= f / f1; // Newton iteration step
    }

    // Solve two quadratics factored from the quartic using the cubic root
    if (lambda < 0.0) return n;
    float t = sqrt(lambda); // Because we solved for t^2 but want t
    float alpha = 2.0 * q / t, beta = lambda + ra;

    float u = 0.25 * b;
    t *= 0.5;

    float z = -alpha - beta;
    if (z > 0.0) {
        z = sqrt(z) * 0.5;
        float h = +t - u;
        roots.xy = vec2(h + z, h - z);
        n += 2;
    }

    float w = +alpha - beta;
    if (w > 0.0) {
        w = sqrt(w) * 0.5;
        float h = -t - u;
        roots.zw = vec2(h + w, h - w);
        if (n == 0) roots.xy = roots.zw;
        n += 2;
    }

    return n;
}

vec2 getDistSquaredAndParam(vec2 a, vec2 b, vec2 c, vec2 d, vec2 uv ) {
    float s1 = -1.0;  vec2 S1 = 3.0*(b-c)-a+d;
    float s2 = +1.0;  vec2 S2 = 3.0*(c+a-b-b);
    float H1 = -1.0;  vec2 S3 = 3.0*(b-a);
    float H2 = +1.0;  vec2 S4 = a-uv;

    float U1 = 3.0*dot(S1,S1);
    float U2 = 5.0*dot(S1,S2);
    float U3 = 4.0*dot(S1,S3) + 2.0*dot(S2,S2);
    float U4 = 3.0*dot(S1,S4) + 3.0*dot(S2,S3);
    float U5 = 2.0*dot(S2,S4) + 1.0*dot(S3,S3);
    float U6 = 1.0*dot(S3,S4);

    for ( int i = 0; i < 10; i += 1 ) {
        float s3 = (s1+s2)/2.0; float k = s3/(1.0-abs(s3));
        float H3 = k*(k*(k*(k*(U1*k+U2)+U3)+U4)+U5)+U6;
        ( H1*H3 <= 0.0 ) ? ( s2 = s3, H2 = H3 ) : ( s1 = s3, H1 = H3 );

    }

    float params[5];
    params[0] = (s1*H2-s2*H1) / (H2-H1); params[0] /= 1.0 - abs(params[0]);

    float B1 = U1, B2 = U2 + params[0] * B1,
    B3 = U3+params[0] * B2, B4 = U4 + params[0] * B3, B5 = U5 + params[0] * B4;

    vec4 roots; solveQuartic( B1, B2, B3, B4, B5, roots );
    params[1] = roots.x, params[2] = roots.y, params[3] = roots.z, params[4] = roots.w;


    for (int i = 0; i < 5; ++i) params[i] = clamp(params[i], 0., 1.);

    float dist2 = 1e38f, minParam = params[0];
    for (int i = 0; i < 5; ++i) {
        vec2 tmp = params[i] * (params[i] * (params[i] * S1 + S2) + S3) + S4;
        float tmp2 = dot(tmp, tmp);
        if(tmp2 < dist2) {
            dist2 = tmp2;
            minParam = params[i];
        }
    }

    return vec2(dist2, minParam);
}

vec2 GetPosOnSpline(float splineKey)
{
    splineKey = clamp(splineKey, 0., float(ROAD_SEGMENT_NUM));
    int segmentIndex = int(floor(splineKey));
    float param = fract(splineKey);

    vec2 p0 = roadSplineSegments[segmentIndex * 4 + 0].xy;
    vec2 p1 = roadSplineSegments[segmentIndex * 4 + 1].xy;
    vec2 p2 = roadSplineSegments[segmentIndex * 4 + 2].xy;
    vec2 p3 = roadSplineSegments[segmentIndex * 4 + 3].xy;
    return bezierPos(p0, p1, p2, p3, param);
}

vec2 GetDerivOnSpline(float splineKey)
{
    splineKey = clamp(splineKey, 0., float(ROAD_SEGMENT_NUM));
    int segmentIndex = int(floor(splineKey));
    float param = fract(splineKey);

    vec2 p0 = roadSplineSegments[segmentIndex * 4 + 0].xy;
    vec2 p1 = roadSplineSegments[segmentIndex * 4 + 1].xy;
    vec2 p2 = roadSplineSegments[segmentIndex * 4 + 2].xy;
    vec2 p3 = roadSplineSegments[segmentIndex * 4 + 3].xy;
    return bezierDeriv(p0, p1, p2, p3, param);
}

vec2 SignedDistAndParam(vec2 uv)
{
    float minDist2 = 1e38f, param, _sign = 1.;
    for (int segmentIndex = 0; segmentIndex < ROAD_SEGMENT_NUM; ++segmentIndex) {
        vec2 p0 = roadSplineSegments[segmentIndex * 4 + 0].xy;
        vec2 p1 = roadSplineSegments[segmentIndex * 4 + 1].xy;
        vec2 p2 = roadSplineSegments[segmentIndex * 4 + 2].xy;
        vec2 p3 = roadSplineSegments[segmentIndex * 4 + 3].xy;

        vec2 bez = getDistSquaredAndParam(p0, p1, p2, p3, uv);

        float dist2 = bez.x;
        float localParam = bez.y;

        if (dist2 < minDist2) {
            minDist2 = dist2;
            param = float(segmentIndex) + localParam;

            vec2 pos = bezierPos(p0, p1, p2, p3, localParam);
            vec2 der = bezierDeriv(p0, p1, p2, p3, localParam);
            _sign = sign(dot(vec2(-der.y, der.x), uv - pos));
        }
    }
    return vec2(_sign * sqrt(minDist2), param);
}

vec4 cityMap(vec2 p)
{
    const float maxLOD = 3.;
    float h = 1.;
        vec2 uv = p;
    float fwScale = 1.;
    vec2 o = vec2(0);
    for (float i = maxLOD; i >= 0.; i--)
    {
        float s = exp2(maxLOD - i);
        o = floor(p * s) / s;
        fwScale = s;
                uv = (p-o) * s;

        vec2 r = hash22(o);
        float k = r.x;
        if (i == maxLOD) {
            h = (k * 0.25 + 0.75) * 0.9999;
        } else {
            k = mix(k, 1.0, 0.5);
            h *= k;
        }

        if (i != maxLOD && r.y < 0.1 + (maxLOD - i) * 0.1)
            break;
    }

    return vec4(uv, fwScale, h);
}

float getRoadLight(vec2 roadUV) {
    float light = abs(sin(roadUV.x * 0.0010046 + step(roadUV.y, 0.) * PI / 2.)) * abs(sin(roadUV.y * 0.001));
    return light * light;
}

float carHeight(vec2 p) {
    p = sqrt(p * p + 1000.);
    vec2 p0 = (p - vec2(140, 90)) * vec2(1, 1.3);
    float hWheel = dot(p0, p0);

    p *= vec2(.5,1);
    float hCenter = dot(p, p);
    return hWheel /  (hWheel + 8000.) * -4000.
         + hCenter / (hCenter + 3000.) * -9000.
        + dot(p,p) * 0.01;
}

vec3 getCarNormal(vec2 p) {
    const float eps = 1.;
    float v = carHeight(p);
    float vx = carHeight(p + vec2(eps, 0));
    float vy = carHeight(p + vec2(0, eps));
    return normalize(vec3(v - vx, v - vy, 30));
}

float SplineKeyToDist(float key) { return key * 100000;}

void main()
{
    vec2 uv = inUV * cameraZoom;
    float fw = length(fwidth(uv));



    float cameraDistantFactor = smoothstep(maxZoom, 20000., cameraZoom);
    // set car view
    uv.x += cameraZoom;

    uv = uv * rotate2d(mix(PI / 2., cameraRotation, cameraDistantFactor));
    uv += mix(trackCenter, carPos, cameraDistantFactor);

    finalColor = vec3(uv, 0);


    { // CITY
        vec4 cityData = cityMap(uv * 0.00002);
        vec2 cityUV = cityData.xy * 2. - 1.;
        float blockScale = cityData.z;

        float buildingDist = max(abs(cityUV).x, abs(cityUV).y);
        float distFromEdge = (1. - buildingDist) / blockScale;
        float building = smoothstep(fw, -fw, (distFromEdge * 17000. - 300.) * .2);

        vec2 cityDir = normalize(cityUV);
        float c = abs(dot(cityDir, normalize(vec2(1,.3))));
        float dir = atan(cityUV.y, cityUV.x);
        float redColor =   sin(dir * 128. / blockScale);      redColor *= redColor; redColor *= redColor;
        float whiteColor = sin(dir * 128. / blockScale - 2.); whiteColor *= whiteColor; whiteColor *= whiteColor;
        float trafficColorMag = max(sin(distFromEdge * 200.), 0.)
            * pow(abs(dot(cityDir, normalize(vec2(1,.5)))), 16.);

        finalColor = (redColor * vec3(.5,0,0) + whiteColor * vec3(.7)) * trafficColorMag;
        finalColor += vec3(.4,.3,.2) * .3;
        finalColor *= building;
        finalColor += .1;
    }


    vec2 distParam = SignedDistAndParam(uv); // x - SDF, y - param
    float distAlong = SplineKeyToDist(distParam.y);
    float absDistFromSpline = abs(distParam.x);
    float border = smoothstep(1200., 1200. - fw, absDistFromSpline);




    { // ROAD AND BORDER
        float road = smoothstep(roadHalfWidth, roadHalfWidth - fw, absDistFromSpline);
        float smoothRoadEdge = smoothstep(1700., 1200., absDistFromSpline);

        vec3 roadAndBorder = vec3(0);
        roadAndBorder = mix(roadAndBorder, vec3(0.2) + .6, border);
        roadAndBorder = mix(roadAndBorder, vec3(0.3 + noise(vec2(distAlong, distParam.x)
        * vec2(0.001, 0.06)) * min(1. / fw, 1.) * .3), road);

        float light = getRoadLight(vec2(distAlong, distParam.x));
        finalColor = mix(finalColor, roadAndBorder * (light * .5 + .3), smoothRoadEdge);
        finalColor += vec3(saturate(1. - absDistFromSpline * 0.0004) * light * 0.2);
    }



    vec2 p = uv - carPos;
    mat2 carMat = matFromDir(carDir);
    vec2 pLocal = carMat * p;
    vec3 carColor = vec3(0);

    float carSDF = sdBox(pLocal, vec2(200., 100.));
    float carEdge = smoothstep(-fw * 2., -fw, -carSDF);


    float carShadow = 1.- pow(saturate(carSDF * -0.01 + .5), 2.);
    finalColor *= carShadow;



    if(carEdge > 0.) { // CAR
        float wing = smoothstep(-fw, fw, -sdBox(pLocal- vec2(-200,0), vec2(40., 100.)));
        wing = min(wing, carEdge);

        vec3 carNormal = getCarNormal(pLocal);
        vec3 carColor = vec3(normalize(carNormal));

        carNormal.xy = carNormal.xy * carMat;
        carNormal = normalize(carNormal);
        vec2 reflectUV = reflect(carNormal, vec3(0,0,1)).xy;
        vec2 distParamRefl = SignedDistAndParam(uv + reflectUV * 1000.);
        float distAlongRefl = SplineKeyToDist(distParamRefl.y);
        float light = getRoadLight(vec2(distAlongRefl, distParamRefl.x));



        float spec = pow(light, 10.);
        float fw2 = fw * 0.5;
        float glass = smoothstep(-fw2, fw2, length(pLocal * vec2(.6,1)) - 40.)
                    - smoothstep(-fw2, fw2, length((pLocal - vec2(30, 0)) * vec2(.6,1)) - 40.);
                    glass = saturate(glass);


        vec3 paintColor = vec3(1,.5,0);
        paintColor = mix(paintColor, vec3(.8), smoothstep(fw, -fw, abs(abs(pLocal.y) - 10.) - 5.));
        paintColor = mix(paintColor, vec3(0.1), glass);


        paintColor = mix(paintColor, vec3(.2), wing); // wing

        carColor = paintColor * (light * 1. + .4) + vec3(spec);



        carEdge = min(carEdge, smoothstep(-fw * 40., 0., carHeight(pLocal) + 8300.));
        carEdge = max(carEdge, wing);
        finalColor = mix(finalColor, carColor, carEdge);

        //finalColor = vec3(glass);
    }

    { // LIGHT
        vec2 pRed = (pLocal * vec2(.7,1.2) * .8 - vec2(-290,0)) * 0.001;
        finalColor *= 1. + float(carInput.g > 0.) *
            pow(saturate(1. - length(pRed)) * (1.-saturate(pow(saturate(pRed.x * 4. ), 2.))), 2.) * vec3(8,0,0);

        vec2 pWhite = (pLocal * vec2(.5,2) * .8 - vec2(400,0)) * 0.001;
        finalColor *= 1. +
            pow(saturate(1. - length(pWhite)) * (1.-saturate(pow(saturate(-pWhite.x * 2.), 2.))), 2.) * vec3(10);
    }


    finalColor = sigmoid(finalColor, 5.);
}
