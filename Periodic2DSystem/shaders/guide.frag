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

uniform vec2 dimensions2D;
uniform ivec2 textureDimensions2D;

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

void main() {
    vec2 position = texture2D(qTex, UV).xy + dt*texture2D(qDotTex, UV).xy;
    vec2 texPosition = vec2(
        position.x/dimensions2D[0], position.y/dimensions2D[1]);
    complex2 gradPsi = gradient4thOr(psiTex, texPosition);
    complex gradPsiX = gradPsi.xy;
    complex gradPsiY = gradPsi.zw;
    complex invPsi = inv(texture2D(psiTex, texPosition).xy);
    vec2 dQDt = (hbar/m)*vec2(
        imag(mul(gradPsiX, invPsi)),
        imag(mul(gradPsiY, invPsi)));
    fragColor = vec4(dQDt, dQDt);
}


