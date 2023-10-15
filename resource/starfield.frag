// star
#version 330 core
out vec4 FragColor;
in vec2 Coord;

uniform float time;

#define N_LAYERS 4

mat2 Rot(float a) {
    float s = sin(a), c = cos(a);
    return mat2(c, -s, s, c);
}

float Hash21(vec2 p) {
    p = fract(p*vec2(123.32, 456.21));
    p += dot(p, p+45.32);
    return fract(p.x*p.y);
}

float Star(vec2 uv, float flare) {
    float d = length(uv);
    float m = .05/d;//smoothstep(.1, .05, d);
    
    float rays = max(0., 1.-abs(uv.x*uv.y*1000.));
    m += rays*flare;
    uv *= Rot(3.141592/4.);
    rays = max(0., 1.-abs(uv.x*uv.y*1000.));
    m += rays*.3*flare;

    m *= smoothstep(1., .2, d);
    return m;
}

vec3 StarLayer(vec2 uv) {
    vec3 col = vec3(0);

    vec2 gv = fract(uv)-.5;
    vec2 id = floor(uv);

    for (int y=-1;y<=1;y++) {
        for (int x=-1;x<=1;x++) {
            vec2 offs = vec2(x, y);
            float n = Hash21(id + offs); // random between 0 and 1
            float size = fract(n*345.32);
            float star = Star(gv-offs-(vec2(n, fract(n*34.))-.5), smoothstep(.85, .9, size));
            vec3 color = vec3(.8, .2, .1);

            star *= sin(time*3.+n*6.2341)*.5+1.;
            col += star*size*color;
        }
    }
    // col.rg = gv;
    // if (gv.x>.48 || gv.y>.48) { col.r=1.; }
    // col.rg += id*.4;
    // col += Hash21(id);
    return col;
}

void main() {
    // should be uniforms
    vec2 FragCoord = gl_FragCoord.xy;
    vec2 Resolution = vec2(800, 640);

    vec2 uv = (FragCoord-.5*Resolution.xy)/Resolution.y;
    // uv *= 5.;

    vec3 col = vec3(0);

    for (float i=0; i<1.; i+=1./N_LAYERS) {
        // float depth = fract(i+time);
        float scale = mix(20., .5, i);
        // float fade = depth;
        // col += StarLayer(uv * scale);
        col += StarLayer(vec2(uv.x, uv.y + time * 0.5) * scale) * (i + .1);
    }

    FragColor = vec4(col, 1.f);
}
