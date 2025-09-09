/* Guide the particle using the pilot wave.
 */
#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if (__VERSION__ > 120) || defined(GL_ES)
precision highp float;
#endif
    
#if __VERSION__ <= 120
varying vec2 UV;
#define fragColor gl_FragColor
#else
in vec2 UV;
out vec4 fragColor;
#endif

uniform float hbar;
uniform float m;

uniform sampler2D psiTex;
uniform sampler2D qTex;
uniform float dt;
uniform sampler2D qDotTex;
uniform bool nearestSamplingOnly;
// uniform sampler2D potentialTex;

uniform vec2 dimensions2D;
uniform ivec2 textureDimensions2D;

uniform bool imposeAbsorbingBoundaries;

#define complex vec2
#define complex2 vec4

complex conj(complex z) {
    return complex(z.x, -z.y);
}

complex mul(complex a, complex b) {
    return complex(a[0]*b[0] - a[1]*b[1], a[0]*b[1] + a[1]*b[0]);
}

complex inv(complex z) {
    return conj(z)/(z.x*z.x + z.y*z.y);
}

float imag(complex z) {
    return z.y;
}

/*Bilinear interpolation. */
vec4 blI(vec2 r, float x0, float y0, float x1, float y1,
         vec4 w00, vec4 w10, vec4 w01, vec4 w11) {
    float dx = x1 - x0, dy = y1 - y0;
    float ax = (dx == 0.0)? 0.0: (r.x - x0)/dx;
    float ay = (dy == 0.0)? 0.0: (r.y - y0)/dy;
    return mix(mix(w00, w10, ax), mix(w01, w11, ax), ay);
}

/* Some devices do not support linear texture filtering
for floating point textures. If the sampling is restriced
to nearest (as set by the nearestSamplingOnly uniform bool),
then get the nearest four texel values to the texture
coordinate r, and perform a manual bilinear interpolation of 
the texture value at r. */
vec4 customSampler(sampler2D tex, vec2 r) {
    if (!nearestSamplingOnly)
        return texture2D(tex, r);
    float width = float(textureDimensions2D[0]);
    float height = float(textureDimensions2D[1]);
    float x0 = (floor(r.x*width - 0.5) + 0.5)/width;
    float y0 = (floor(r.y*height - 0.5) + 0.5)/height;
    float x1 = (ceil(r.x*width - 0.5) + 0.5)/width;
    float y1 = (ceil(r.y*height - 0.5) + 0.5)/height;
    vec2 r00 = vec2(x0, y0);
    vec2 r10 = vec2(x1, y0);
    vec2 r01 = vec2(x0, y1);
    vec2 r11 = vec2(x1, y1);
    vec4 f = texture2D(tex, r);
    vec4 f00 = texture2D(tex, r00);
    vec4 f10 = texture2D(tex, r10);
    vec4 f01 = texture2D(tex, r01);
    vec4 f11 = texture2D(tex, r11);
    return blI(r.xy, x0, y0, x1, y1, f00, f10, f01, f11);
}

complex2 gradient4thOr(sampler2D tex, vec2 uv) {
    float du = 1.0/float(textureDimensions2D[0]);
    float dv = 1.0/float(textureDimensions2D[1]);
    float dx = dimensions2D[0]/float(textureDimensions2D[0]);
    float dy = dimensions2D[1]/float(textureDimensions2D[1]);
    complex up2, up1, center, down1, down2;
    complex left2, left1, right1, right2;
    up2 = customSampler(tex, uv + vec2(0.0, 2.0*dv)).xy;
    up1 = customSampler(tex, uv + vec2(0.0, dv)).xy;
    down1 = customSampler(tex, uv + vec2(0.0, -dv)).xy;
    down2 = customSampler(tex, uv + vec2(0.0, -2.0*dv)).xy;
    left2 = customSampler(tex, uv + vec2(-2.0*du, 0.0)).xy;
    left1 = customSampler(tex, uv + vec2(-du, 0.0)).xy;
    right1 = customSampler(tex, uv + vec2(du, 0.0)).xy;
    right2 = customSampler(tex, uv + vec2(2.0*du, 0.0)).xy;
    // center = texture2D(tex, UV);
    complex dFDx, dFDy; 
    dFDx = (left2/12.0 - 2.0*left1/3.0 + 2.0*right1/3.0 - right2/12.0)/dx;
    dFDy = (down2/12.0 - 2.0*down1/3.0 + 2.0*up1/3.0 - up2/12.0)/dy;  
    return complex2(dFDx, dFDy);
}

vec2 absorbingBoundaries(vec2 position, vec2 dQDt) {
    float x = position.x/dimensions2D[0];
    float y = position.y/dimensions2D[1];
    float s = 0.02;
    float a = 0.8;
    // if (x <= 2.5*s)
    //     dQDt *= max(0.0, smoothstep(0.0, 2.5*s, x)/(2.5*s))*normalize(dQDt);
    // if (y <= 2.5*s)
    //     dQDt *= max(0.0, smoothstep(0.0, 2.5*s, y)/(2.5*s))*normalize(dQDt); 
    // if (x < 2.5*s)
    //     dQDt *= 0.1*smoothstep(0.0, 2.5*s, x)/(2.5*s);
    // if (x <= 2.5*s || x > (1.0 - 2.5*s) || 
    //     y <= 2.5*s || y > (1.0 - 2.5*s))
    //     dQDt = 0.075*normalize(dQDt);
    // if (x <= s || x > (1.0 - s) || 
    //     y <= s || y > (1.0 - s))
    //     dQDt *=0.5;
    vec2 dQDt2 = dQDt;
    if (x > (1.0 - 2.5*s) || x <= 2.5*s)
        dQDt2 = 0.5*vec2(normalize(dQDt).x, 0.0);
    if (y > (1.0 - 2.5*s) || y <= 2.5*s)
        dQDt2 = 0.5*vec2(0.0, normalize(dQDt).y);
    if (x <= 0.1*s || x > (1.0 - 0.1*s) || 
        y <= 0.1*s || y > (1.0 - 0.1*s))
        dQDt2 *= 0.0;
    return dQDt2;
}

void main() {
    vec2 position = texture2D(qTex, UV).xy + dt*texture2D(qDotTex, UV).xy;
    vec2 texPosition = vec2(
        position.x/dimensions2D[0], position.y/dimensions2D[1]);
    complex2 gradPsi = gradient4thOr(psiTex, texPosition);
    complex gradPsiX = gradPsi.xy;
    complex gradPsiY = gradPsi.zw;
    complex invPsi = inv(texture2D(psiTex, texPosition).xy);
    // complex2 gradientV = gradient4thOr(potentialTex, texPosition);
    // vec2 gradImV = vec2(gradientV.y, gradientV.w);
    // complex invImV = 1.0/(texture2D(potentialTex, texPosition).y);
    vec2 dQDt = (hbar/m)*vec2(
        imag(mul(gradPsiX, invPsi)),
        imag(mul(gradPsiY, invPsi)));
    if (imposeAbsorbingBoundaries)
        dQDt = absorbingBoundaries(position, dQDt);
    fragColor = vec4(dQDt, dQDt);
}


