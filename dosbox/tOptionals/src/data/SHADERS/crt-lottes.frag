#version 330 core

//
// PUBLIC DOMAIN CRT STYLED SCAN-LINE SHADER
//
//	by Timothy Lottes
//
// This is more along the style of a really good CGA arcade monitor.
// With RGB inputs instead of NTSC.
// The shadow mask example has the mask rotated 90 degrees for less chromatic aberration.
//
// Left it unoptimized to show the theory behind the algorithm.
//
// It is an example what I personally would want as a display option for pixel art games.
// Please take and use, change, or whatever.
//

const float hardScan = -6.0;
const float hardPix = -3.0;
const float warpX = 0.021;
const float warpY = 0.045;
const float maskDark = 0.5;
const float maskLight = 1.5;
const float scaleInLinearGamma = 1;
const float shadowMask = 1;
const float brightboost = 0.65;
#define Blackmask 1
layout (std140) uniform program
{
	vec2 video_size;
	vec2 texture_size;
	vec2 output_size;
} IN;

in vec2 tex;

uniform sampler2D s0;

out vec4 color;

//------------------------------------------------------------------------

// sRGB to Linear.
// Assuing using sRGB typed textures this should not be needed.
float ToLinear1(float c)
{
	if (scaleInLinearGamma == 0)
	{
		return c;
	}
	return (c <= 0.04045) ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}

vec3 ToLinear(vec3 c)
{
	if (scaleInLinearGamma == 0)
	{
		return c;
	}
	return vec3(ToLinear1(c.r), ToLinear1(c.g), ToLinear1(c.b));
}

// Linear to sRGB.
// Assuing using sRGB typed textures this should not be needed.
float ToSrgb1(float c)
{
	if (scaleInLinearGamma == 0)
	{
		return c;
	}
	return(c < 0.0031308 ? c *12.92 : 1.055 * pow(c, 0.41666) - 0.055);
}

vec3 ToSrgb(vec3 c)
{
	if (scaleInLinearGamma == 0)
	{
		return c;
	}
	return vec3(ToSrgb1(c.r), ToSrgb1(c.g), ToSrgb1(c.b));
}

// Nearest emulated sample given floating point position and texel offset.
// Also zero's off screen.
vec3 Fetch(vec2 pos,vec2 off)
{
	pos = (floor(pos * IN.texture_size.xy + off) + vec2(0.5, 0.5)) / IN.texture_size.xy;
	return ToLinear(brightboost * texture(s0, pos.xy).rgb);
}

// Distance in emulated pixels to nearest texel.
vec2 Dist(vec2 pos)
{
	pos = pos * IN.texture_size.xy;
	return -((pos - floor(pos)) - vec2(0.5));
}

// 1D Gaussian.
float Gaus(float pos, float scale)
{
	return exp2(scale * pos * pos);
}

// 3-tap Gaussian filter along horz line.
vec3 Horz3(vec2 pos, float off)
{
	vec3 b = Fetch(pos, vec2(-1.0, off));
	vec3 c = Fetch(pos, vec2(0.0, off));
	vec3 d = Fetch(pos, vec2(1.0, off));
	float dst = Dist(pos).x;
	// Convert distance to weight.
	float scale = hardPix;
	float wb = Gaus(dst - 1.0, scale);
	float wc = Gaus(dst + 0.0, scale);
	float wd = Gaus(dst + 1.0, scale);
	// Return filtered sample.
	return (b * wb + c * wc + d * wd) / (wb + wc + wd);
}

// 5-tap Gaussian filter along horz line.
vec3 Horz5(vec2 pos, float off)
{
	vec3 a = Fetch(pos, vec2(-2.0, off));
	vec3 b = Fetch(pos, vec2(-1.0, off));
	vec3 c = Fetch(pos, vec2(0.0, off));
	vec3 d = Fetch(pos, vec2(1.0, off));
	vec3 e = Fetch(pos, vec2(2.0, off));
	float dst = Dist(pos).x;
	// Convert distance to weight.
	float scale = hardPix;
	float wa = Gaus(dst - 2.0, scale);
	float wb = Gaus(dst - 1.0, scale);
	float wc = Gaus(dst + 0.0, scale);
	float wd = Gaus(dst + 1.0, scale);
	float we = Gaus(dst + 2.0, scale);
	// Return filtered sample.
	return (a * wa + b * wb + c * wc + d * wd + e * we) / (wa + wb + wc + wd + we);
}

// Return scanline weight.
float Scan(vec2 pos, float off)
{
	float dst = Dist(pos).y;
	return Gaus(dst + off, hardScan);
}

// Allow nearest three lines to effect pixel.
vec3 Tri(vec2 pos)
{
	vec3 a = Horz3(pos, -1.0);
	vec3 b = Horz5(pos, 0.0);
	vec3 c = Horz3(pos, 1.0);
	float wa = Scan(pos, -1.0);
	float wb = Scan(pos, 0.0);
	float wc = Scan(pos, 1.0);
	return a * wa + b * wb + c * wc;
}

// Distortion of scanlines, and end of screen alpha.
vec2 Warp(vec2 pos)
{
	pos = pos * 2.0 -1.0;
	pos *= vec2(1.0 + (pos.y * pos.y) * warpX, 1.0 + (pos.x * pos.x) * warpY);
	return pos * 0.5 + 0.5;
}

// Shadow mask.
vec3 Mask(vec2 pos)
{
	pos.x += pos.y * 3.0;
	vec3 mask = vec3(maskDark, maskDark, maskDark);
	pos.x = fract(pos.x / 6.0);
	if (pos.x < 0.333)
	{
		mask.r = maskLight;
	}
	else if (pos.x < 0.666)
	{
		mask.g = maskLight;
	}
	else
	{
		mask.b = maskLight;
	}
	return mask;
}

uniform vec2 resolution;
uniform vec2 mouse;
uniform float time;

float box(vec2 _st, vec2 _size, float _smoothEdges) {
	_size = vec2(.1) - _size*.2;
	vec2 aa = vec2(_smoothEdges * 0.1);
	vec2 uv = smoothstep(_size, _size+aa, _st);
	uv *= smoothstep(_size, _size+aa, vec2(1.0)-_st);
	return uv.x * uv.y;
}

vec3 drawRectangle(in vec2 st) {
    // Each result will return 1.0 (white) or 0.0 (
	vec3 color = vec3(0.0);
	vec2 borders = step(vec2(0.0),st); 
    float pct = borders.x * borders.y;
	
	// top-right 
    vec2 tr = step(vec2(0.1),1.0-st);
    pct *= tr.x * tr.y;
    // The multiplication of left*bottom will be similar to the logical AND.
    color = vec3(pct); 
	return color;
}	

void main()
{
	vec2 pos = Warp(tex.xy * (IN.texture_size.xy / IN.video_size.xy)) * (IN.video_size.xy / IN.texture_size.xy);
	
#ifdef Blackmask	
	vec3 outColor = vec3(1.0);	
	outColor *= Tri(pos);
	outColor *= drawRectangle(pos);	
	outColor *= vec3(box(pos, vec2(0.5), 0.01));
#else
	vec3 outColor = Tri(pos);
#endif


	
	if (shadowMask != 0)
	{
		outColor.rgb *= Mask(floor(tex.xy * (IN.texture_size.xy / IN.video_size.xy) * IN.output_size.xy) + vec2(0.5, 0.5));
	}

		float	x=	mod(gl_FragCoord.y, 2.0);

	if(x> 1.0)	{
		color = vec4(1.0, 1.0, 1.0, 1.0);
	}
	else
	{
		color =	vec4(0.0, 0.0, 0.0, 1.0);
	}
	
	color *= vec4(ToSrgb(outColor.rgb), 1.0);
	
	color = vec4(ToSrgb(outColor.rgb), 1.0);

}
