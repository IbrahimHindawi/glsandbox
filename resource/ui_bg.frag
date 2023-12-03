#version 330 core
out vec4 FragColor;
in vec2 Coord;
uniform sampler2D utexture;

void main() {
    vec2 newCoord = Coord;
    // newCoord.x *= -1.f;
    newCoord.y *= -1.f; // flip texture!
    FragColor = vec4(0.1f, 0.1f, 0.1f, 1.f);
    // FragColor = texture(utexture, newCoord);
}
