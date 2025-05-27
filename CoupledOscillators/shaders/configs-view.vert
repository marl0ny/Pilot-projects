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

void main() {
    UV = position.xy;
    gl_Position = vec4(
        position.x, position.y*scaleY + yOffset,
        vec2(0.0, 1.0)
    );
}
