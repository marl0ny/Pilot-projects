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

uniform int size;
uniform int type;
const int TYPE_DST = 0;
const int TYPE_DSCT = 1;

#define PI 3.141592653589793

bool isOdd(int n) {
    return mod(float(n), 2.0) > 0.5;
}

/* Get the elements of the type I DST transformation matrix.

The equations for this can be found in Scipy's documentation for the DST:

https://docs.scipy.org/doc/scipy/reference/generated/scipy.fft.dst.html

*/
float dstElement(int i, int j, int n) {
    return 2.0*sin(PI*float(i + 1)*float(j + 1)/float(n + 1))
        /sqrt(2.0*float(n + 1));
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

void main() {
    int n = size;
    int i = int(UV[0]*float(n) - 0.5);
    int j = int(UV[1]*float(n) - 0.5);
    if (type == TYPE_DST)
        fragColor = vec4(dstElement(i, j, n));
    else if (type == TYPE_DSCT)
        fragColor = vec4(dsctElement(i, j, n));

}

