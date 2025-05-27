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

uniform int numberOfOscillators;
uniform ivec2 trajectoriesTexDimensions;

uniform sampler2D trajectoriesTex;
uniform bool useTransformTex;
uniform sampler2D transformTex;

uniform int boundaryType;
#define ZERO_ENDPOINTS 0
#define PERIODIC 1

#define MAX_SIZE 256
const float PI = 3.141592653589793;

bool isOdd(int n) {
    return mod(float(n), 2.0) > 0.5;
}

/* Get the elements of the type I DST transformation matrix.

The equations for this can be found in Scipy's documentation for the DST:

https://docs.scipy.org/doc/scipy/reference/generated/scipy.fft.dst.html

*/
float dstElement(float i, float j, float n) {
    float size = n + 1.0;
    float amp = 2.0/sqrt(2.0*size);
    float x = (i + 1.0)/size;
    float k = j + 1.0; 
    float angle = (PI*k)*x;
    return amp*sin(angle);
}

/* Get the elements of the discrete sine cosine transformation
matrix. See equations 14-15 of Johnson and Gutierrez
to see them explicitly written out for the n=8 case. */
float dsctElement(int i, int j, int n) {
    int k = (isOdd(n))? (j - n/2): (j - n/2 + 1);
    float norm_factor = (isOdd(n))?
        sqrt(((k != 0)? 2.0: 1.0)/float(n)):
        sqrt(((k != 0 && j != n-1)? 2.0: 1.0)/float(n));
    return norm_factor*((k < 0)? 
        -sin(2.0*PI*float(i*k)/float(n)): 
        cos(2.0*PI*float(i*k)/float(n)));
}

vec2 getOscillatorSampleIndices(vec2 uv) {
    float width = float(trajectoriesTexDimensions[0]);
    float height = float(trajectoriesTexDimensions[1]);
    float n = float(numberOfOscillators);
    float xInd = uv.x*width - 0.5;
    float yInd = uv.y*height - 0.5;
    float oscillatorInd = mod(xInd, n);
    float sampleInd = height*floor(xInd/n) + yInd;
    return vec2(oscillatorInd, sampleInd);
}

vec2 getUV(vec2 oscillatorSampleIndices) {
    float width = float(trajectoriesTexDimensions[0]);
    float height = float(trajectoriesTexDimensions[1]);
    float n = float(numberOfOscillators);
    float oscillatorInd = oscillatorSampleIndices[0];
    float sampleInd = oscillatorSampleIndices[1];
    float xInd = n*floor(sampleInd/height) + oscillatorInd;
    float yInd = mod(sampleInd, height);
    return vec2((xInd + 0.5)/width, (yInd + 0.5)/height);
}

vec4 sampleTrajectoryTex(float posInd, float sampleInd) {
    vec2 uv = getUV(vec2(posInd, sampleInd));
    return texture2D(trajectoriesTex, uv);
}

vec4 iDiscreteSineTransformNormal2Pos(vec2 oscillatorSampleIndices) {
    float posInd = oscillatorSampleIndices[0];
    float sampleInd = oscillatorSampleIndices[1];
    vec4 posOffset = vec4(0.0);
    float n = float(numberOfOscillators);
    for (float normInd = 0.0; normInd < float(MAX_SIZE); normInd++) {
        if (normInd <= float(numberOfOscillators-1))
            posOffset += dstElement(
                normInd, posInd, float(numberOfOscillators)
            )*sampleTrajectoryTex(normInd, sampleInd);
    }
    posOffset.a = 1.0;
    return posOffset;
}

vec4 identity(vec2 oscillatorSampleIndices) {
    float posInd = 2.0;
    float sampleInd = oscillatorSampleIndices[1];
    vec2 uv = getUV(vec2(posInd, sampleInd));
    return texture2D(trajectoriesTex, uv);
}

// vec4 iDiscreteSineCosineTransformNormal2Pos(ivec2 oscillatorSampleIndices) {
//     int posInd = oscillatorSampleIndices[0];
//     int sampleInd = oscillatorSampleIndices[1];
//     vec4 posOffset = vec4(0.0);
//     for (int normInd = 0; normInd < MAX_SIZE; normInd++) {
//         if (normInd >= numberOfOscillators)
//             return posOffset;
//         vec2 uv = getUV(ivec2(normInd, sampleInd));
//         uv[1] = UV[1];
//         vec4 normOffset = texture2D(trajectoriesTex, uv);
//         posOffset 
//             += dsctElement(posInd, normInd, numberOfOscillators)*normOffset;
//     }
//     return posOffset;
// }

vec4 normal2PosUseTransformTex(vec2 oscillatorSampleIndices) {
    float posInd = oscillatorSampleIndices[0];
    float sampleInd = oscillatorSampleIndices[1];
    vec4 posOffset = vec4(0.0);
    float n = float(numberOfOscillators);
    for (float normInd = 0.0; normInd < float(MAX_SIZE); normInd++) {
        if (normInd <= float(numberOfOscillators-1)) {
            vec2 uv = getUV(vec2(normInd, sampleInd));
            float normOffset = texture2D(trajectoriesTex, uv)[0];
            vec2 transformUV = vec2((posInd + 0.5)/n, (normInd + 0.5)/n);
            posOffset += texture2D(transformTex, transformUV)*normOffset;
        }
    }
    posOffset.a = 1.0;
    return posOffset;
}

void main() {
    vec2 indices = getOscillatorSampleIndices(UV);
    if (useTransformTex) {
        fragColor = normal2PosUseTransformTex(indices);
    } 
    // else {
    //     if (boundaryType == PERIODIC)
    //         fragColor = vec4(0.0);
    //         // fragColor = vec4(
    //         //     iDiscreteSineCosineTransformNormal2Pos(indices));
    //     else
    //         // fragColor = identity(indices);
    //         fragColor = iDiscreteSineTransformNormal2Pos(indices);
    // }
    // // fragColor = texture2D(trajectoriesTex, UV);
}