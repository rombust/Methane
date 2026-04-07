#version 450

layout(location = 0) in vec4  Position;
layout(location = 1) in float Color0;
layout(location = 2) in vec2  TexCoord0;
layout(location = 3) in int   TexIndex0;
layout(location = 4) in float Lighting0;

layout(location = 0) flat out float Color;
layout(location = 1)      out vec2  TexCoord;
layout(location = 2) flat out int   TexIndex;
layout(location = 3) flat out float Lighting;

void main() {
	gl_Position = Position;
	Color       = Color0;
	TexCoord    = TexCoord0;
	TexIndex    = TexIndex0;
	Lighting    = Lighting0;
}
