#version 330

precision highp float;
precision highp int;

uniform float u_l;
uniform float u_t;
uniform float u_i_x;
uniform float u_i_y;
uniform vec2 res;
uniform int u_max_i;
uniform vec3 u_colour;
uniform int k;

out vec4 f_c;

int mod_(int n, int k) {
	// Handle edge cases for infinity or non-positive values
	if (isinf(float(n))) return 255;
	if (n <= 0) return 0;

	// As n -> inf, intensity -> 255
	// Formula: intensity = 255 * (n / (k + n))
	float intensity = 255.0f * (float(n) / (float(k) + float(n)));

	return int(clamp(intensity, 0.0f, 255.0f));
}

vec2 mult(vec2 a, vec2 b) {
    float real = a.x*b.x-a.y*b.y;
    float imag = a.x*b.y+a.y*b.x;
    return vec2(real, imag);
}

void main() {
    float val = -1.0f;

    float x = floor(gl_FragCoord.x);
    float y = floor(res.y - gl_FragCoord.y);
    float real = u_l + (u_i_x * x);
    float imag = u_t - (u_i_y * y);
    vec2 z = vec2(real, imag);

    vec2 c = vec2(-0.7, 0.35);
    int iter = 0;

    vec2 oz = vec2(0.0);
    int ot = 0;
    int nt = 1;

    while (iter < u_max_i) {
        z = mult(z, z) + c;
        iter++;
        if (z.x*z.x+z.y*z.y>4.0) break;
        if (oz == z) val = 0.0f;
        ot++;
        if (ot == nt) {
            nt *= 2;
            ot = 0;
            oz = vec2(z.x, z.y);
        }
    }
    if (val == -1.0f) val = float(mod_(iter, k));

    vec3 colour = u_colour * (val/255.0f);
    f_c = vec4(colour.r, colour.g, colour.b, 1);
}