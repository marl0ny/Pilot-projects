/* Generate a new wavepacket. */
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

complex2 gradient4thOr(sampler2D tex, vec2 uv) {
    float du = 1.0/float(textureDimensions2D[0]);
    float dv = 1.0/float(textureDimensions2D[1]);
    float dx = dimensions2D[0]/float(textureDimensions2D[0]);
    float dy = dimensions2D[1]/float(textureDimensions2D[1]);
    complex up2, up1, center, down1, down2;
    complex left2, left1, right1, right2;
    up2 = texture2D(tex, uv + vec2(0.0, 2.0*dv)).xy;
    up1 = texture2D(tex, uv + vec2(0.0, dv)).xy;
    down1 = texture2D(tex, uv + vec2(0.0, -dv)).xy;
    down2 = texture2D(tex, uv + vec2(0.0, -2.0*dv)).xy;
    left2 = texture2D(tex, uv + vec2(-2.0*du, 0.0)).xy;
    left1 = texture2D(tex, uv + vec2(-du, 0.0)).xy;
    right1 = texture2D(tex, uv + vec2(du, 0.0)).xy;
    right2 = texture2D(tex, uv + vec2(2.0*du, 0.0)).xy;
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


