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

#define complex vec2
#define complex2 vec4

uniform complex maxVal;
uniform complex minVal;
uniform bool addAbsorbingBoundaries;
uniform sampler2D rawPotentialTex;

void main() {
    complex2 potential = texture2D(rawPotentialTex, UV);
    if (addAbsorbingBoundaries) {
        float x = UV.x;
        float y = UV.y;
        float s = 0.02;
        float a = 0.8;
        potential.xy -= complex(0.0, a*exp(-0.5*x*x/(s*s)));
        potential.xy -= complex(0.0, a*exp(-0.5*(x-1.0)*(x-1.0)/(s*s)));
        potential.xy -= complex(0.0, a*exp(-0.5*y*y/(s*s)));
        potential.xy -= complex(0.0, a*exp(-0.5*(y-1.0)*(y-1.0)/(s*s)));
        potential.zw -= complex(0.0, a*exp(-0.5*x*x/(s*s)));
        potential.zw -= complex(0.0, a*exp(-0.5*(x-1.0)*(x-1.0)/(s*s)));
        potential.zw -= complex(0.0, a*exp(-0.5*y*y/(s*s)));
        potential.zw -= complex(0.0, a*exp(-0.5*(y-1.0)*(y-1.0)/(s*s)));
        if (x <= 1.0*s || x > (1.0 - 1.0*s) || 
            y <= 1.0*s || y > (1.0 - 1.0*s)) {
            potential.x *= 0.0; // (a - potential.y)/a;
            potential.z *= 0.0; // (a - potential.w)/a;
        }
    }
    potential.xy = clamp(potential.xy, minVal, maxVal);
    potential.zw = clamp(potential.zw, minVal, maxVal);
    fragColor = potential;
}
