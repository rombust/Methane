#version 450

layout(location = 0) in vec4 Position;
layout(location = 1) in float WhiteFill0;
layout(location = 2) in vec2 TexCoord0;
layout(location = 3) in int TexIndex0;
layout(location = 4) in vec4 TexColour0;

layout(location = 0) flat out float WhiteFill;
layout(location = 1) out vec2 TexCoord;
layout(location = 2) flat out int TexIndex;
layout(location = 3) flat out vec4 TexColour;

void main() {
	gl_Position = Position;
	WhiteFill = WhiteFill0;
	TexCoord = TexCoord0;
	TexIndex = TexIndex0;
	TexColour = TexColour0;
}
