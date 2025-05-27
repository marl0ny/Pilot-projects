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

uniform sampler2D tex;

const float PI = 3.141592653589793;


vec3 argumentToColour(float argVal) {
    float maxCol = 1.0;
    float minCol = 50.0/255.0;
    float colRange = maxCol - minCol;
    if (argVal <= PI/3.0 && argVal >= 0.0) {
        return vec3(maxCol,
                    minCol + colRange*argVal/(PI/3.0), minCol);
    } else if (argVal > PI/3.0 && argVal <= 2.0*PI/3.0){
        return vec3(maxCol - colRange*(argVal - PI/3.0)/(PI/3.0),
                    maxCol, minCol);
    } else if (argVal > 2.0*PI/3.0 && argVal <= PI){
        return vec3(minCol, maxCol,
                    minCol + colRange*(argVal - 2.0*PI/3.0)/(PI/3.0));
    } else if (argVal < 0.0 && argVal > -PI/3.0){
        return vec3(maxCol, minCol,
                    minCol - colRange*argVal/(PI/3.0));
    } else if (argVal <= -PI/3.0 && argVal > -2.0*PI/3.0){
        return vec3(maxCol + (colRange*(argVal + PI/3.0)/(PI/3.0)),
                    minCol, maxCol);
    } else if (argVal <= -2.0*PI/3.0 && argVal >= -PI){
        return vec3(minCol,
                    minCol 
                    - (colRange*(argVal + 2.0*PI/3.0)/(PI/3.0)), maxCol);
    }
    else {
        return vec3(minCol, maxCol, maxCol);
    }
}

void main() {
    float r = texture2D(tex, UV)[0];
    float val = -PI*r/(4.0*10.0) - 2.0*PI/3.0;
    if (val < -PI) {
        val = 2.0*PI + val;
        if (val < 0.0) {
            val = 0.0;
        }
    }
    vec3 colorVal = min(r/16.0, 1.25)*argumentToColour(val);
    fragColor = vec4(
        colorVal, length(colorVal));

}
