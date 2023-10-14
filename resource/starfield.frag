// star
#version 330 core
out vec4 FragColor;
out int id;
in vec2 Coord;
uniform sampler2D utexture;

void main() {
    // FragColor = texture(utexture, Coord);
    FragColor = vec4(0.8, 0.5, 0.1f, 1.0f);
}
