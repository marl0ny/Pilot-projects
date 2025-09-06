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

uniform float m;
uniform float hbar;
uniform float dt;

uniform sampler2D psi0Tex;
uniform sampler2D psi1Tex;
uniform sampler2D potentialTex;

uniform vec2 dimensions2D;
uniform ivec2 textureDimensions2D;

#define complex vec2
#define complex2 vec4


complex mul(complex a, complex b) {
    return complex(a[0]*b[0] - a[1]*b[1], a[0]*b[1] + a[1]*b[0]);
}

complex2 mul(complex2 a, complex2 b) {
    return complex2(mul(a.xy, b.xy), mul(a.zw, b.zw));
}

complex2 laplacian4thOrder9pt(sampler2D tex) {
    float du = 1.0/float(textureDimensions2D[0]);
    float dv = 1.0/float(textureDimensions2D[1]);
    float dx = dimensions2D[0]/float(textureDimensions2D[0]);
    float dy = dimensions2D[1]/float(textureDimensions2D[1]);
    complex2 up2, up1, center, down1, down2;
    complex2 left2, left1, right1, right2;
    up2 = texture2D(tex, UV + vec2(0.0, 2.0*dv));
    up1 = texture2D(tex, UV + vec2(0.0, dv));
    down1 = texture2D(tex, UV + vec2(0.0, -dv));
    down2 = texture2D(tex, UV + vec2(0.0, -2.0*dv));
    left2 = texture2D(tex, UV + vec2(-2.0*du, 0.0));
    left1 = texture2D(tex, UV + vec2(-du, 0.0));
    right1 = texture2D(tex, UV + vec2(du, 0.0));
    right2 = texture2D(tex, UV + vec2(2.0*du, 0.0));
    center = texture2D(tex, UV);
    return (-right2/12.0 + 4.0*right1/3.0 - 5.0*center/2.0 
	         + 4.0*left1/3.0 - left2/12.0)/(dx*dx)
    	    + (-up2/12.0 + 4.0*up1/3.0 - 5.0*center/2.0
	            + 4.0*down1/3.0 - down2/12.0)/(dy*dy);
}

complex2 hamiltonian(sampler2D psiTex, sampler2D potentialTex) {
    complex2 psi = texture2D(psiTex, UV);
    complex2 laplacianPsi = laplacian4thOrder9pt(psiTex);
    float potential = texture2D(potentialTex, UV)[0];
    return (-hbar*hbar)/(2.0*m)*laplacianPsi + psi*potential;
}

void main() {
    complex2 psi0 = texture2D(psi0Tex, UV);
    complex2 iDt = complex2(0.0, dt, 0.0, dt);
    complex2 imV = complex2(
        complex(0.0, texture2D(potentialTex, UV)[1]), 
        complex(0.0, texture2D(potentialTex, UV)[1]));
    complex2 psi2 = psi0 
        - mul(iDt/hbar, hamiltonian(psi1Tex, potentialTex) + mul(psi0, imV));
    fragColor = psi2;
}
