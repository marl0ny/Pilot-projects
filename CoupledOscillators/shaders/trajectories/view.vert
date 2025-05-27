#if __VERSION__ <= 120
attribute vec2 position;
varying vec2 UV;
#else
in vec2 position;
out vec2 UV;
#endif

#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if (__VERSION__ > 120) || defined(GL_ES)
precision highp float;
#endif

uniform float scaleY;
uniform float yOffset;

uniform int numberOfOscillators;
uniform ivec2 trajectoryTexDimensions;
uniform sampler2D trajectoriesTex;
uniform int boundaryCond;
const int ZERO_ENDPOINTS = 0;
const int PERIODIC = 1;

/* 
This function transforms the normalized x oscillator index of
the vertex attribute array and its y Monte Carlo sample index into 
the coordinates of the trajectories input texture which contains
the actual position offsets of each oscillator.

The input vertex attribute array that dictates how the vertices are
arranged has a height that is equal to the number of Monte Carlo steps,
and a width that is equal to numberOfOscillators, or 
numberOfOscillators + 2, or 2*numberOfOscillators: the first is used for
drawing lines without the endpoints included, the second includes the
endpoints, and the last is for disconnected lines without the endpoints
included. To get the actual position offsets of the oscillators a texture
containing these must be sampled, but this texture has dimensions
(numberOfOscillators*stackW) x ((number of Monte Carlo steps)/stackW),
where stackW is a chosen integer value that minimizes the perimeter of
this texture.
*/
vec2 getTextureCoordinates(float oscillatorIndex, float trajectoryIndex) {
    float width = float(trajectoryTexDimensions[0]);
    float height = float(trajectoryTexDimensions[1]);
    float n = float(numberOfOscillators);
    oscillatorIndex += 0.5;
    return vec2(
        (oscillatorIndex + floor(trajectoryIndex/height)*n)/width,
        mod((trajectoryIndex + 0.5), height)/height);
}

void main() {
    float x = (position.x + 0.5)/float(numberOfOscillators);
    float trajectoryIndex = position.y;
    // float trajectoryTexW = float(trajectoryTexDimensions[0]);
    // float trajectoryTexH = float(trajectoryTexDimensions[1]);
    // float stackSize = trajectoryTexH/float(numberOfOscillators);
    UV = getTextureCoordinates(position.x, trajectoryIndex);
    if (boundaryCond == PERIODIC && x < 0.0)
        UV = getTextureCoordinates(
            (float(numberOfOscillators) - 0.5)/float(numberOfOscillators),
            trajectoryIndex);
    if (boundaryCond == PERIODIC && x >= 1.0)
        UV = getTextureCoordinates(
            0.5/float(numberOfOscillators), trajectoryIndex);
    float oscillatorOffset = texture2D(trajectoriesTex, UV).r;
    if (boundaryCond == ZERO_ENDPOINTS && (x < 0.0 || x >= 1.0))
        oscillatorOffset = 0.0;
    gl_Position = vec4(
        2.0*x - 1.0, oscillatorOffset*scaleY + yOffset, vec2(0.0, 1.0));
}
