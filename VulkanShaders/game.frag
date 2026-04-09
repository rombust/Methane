#version 450

layout(set = 0, binding = 0) uniform sampler2D Texture0;
layout(set = 0, binding = 1) uniform sampler2D Texture1;
layout(set = 0, binding = 2) uniform sampler2D Texture2;
layout(set = 0, binding = 3) uniform sampler2D Texture3;
layout(set = 0, binding = 4) uniform sampler2D Texture4;
layout(set = 0, binding = 5) uniform sampler2D Texture5;
layout(set = 0, binding = 6) uniform sampler2D Texture6;
layout(set = 0, binding = 7) uniform sampler2D Texture7;

layout(location = 0) flat in float WhiteFill;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) flat in int TexIndex;
layout(location = 3) flat in vec4 TexColour;

layout(location = 0) out vec4 FragColor;

vec4 sampleTexture(int index, vec2 pos) {
	switch (index) {
		case 0: return texture(Texture0, pos);
		case 1: return texture(Texture1, pos);
		case 2: return texture(Texture2, pos);
		case 3: return texture(Texture3, pos);
		case 4: return texture(Texture4, pos);
		case 5: return texture(Texture5, pos);
		case 6: return texture(Texture6, pos);
		case 7: return texture(Texture7, pos);
		default: return vec4(1.0, 1.0, 1.0, 1.0);
	}
}

void main() {
	vec4 decal = sampleTexture(TexIndex, TexCoord);
	if ((WhiteFill > 0.5) && (decal.a == 1.0))
		FragColor = vec4(clamp(1.0 + TexColour.r, 0.0, 1.0), clamp(1.0 + TexColour.g, 0.0, 1.0), clamp(1.0 + TexColour.b, 0.0, 1.0), clamp(1.0 + TexColour.a, 0.0, 1.0));
	else
		FragColor = vec4(clamp(decal.r + TexColour.r, 0.0, 1.0), clamp(decal.g + TexColour.g, 0.0, 1.0), clamp(decal.b + TexColour.b, 0.0, 1.0), clamp(decal.a + TexColour.a, 0.0, 1.0));
}
