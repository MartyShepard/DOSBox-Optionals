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


#define hardScan -8.0
#define hardPix -3.0
#define warpX 0.007 //0.021, 0.007
#define warpY 0.021 //0.045, 0.021 
#define maskDark 0.5
#define maskLight 1.5
#define scaleInLinearGamma 1
#define shadowMask 3
#define brightboost 0.7
#define hardBloomScan -1.8
#define hardBloomPix -1.5
#define bloomAmount 1.0/16.0
#define shape 2.0 //2.0
#define shadowmaskwight 2.0

#define DO_BLOOM 1
layout (std140) uniform program
{
	vec2 video_size;
	vec2 texture_size;
	vec2 output_size;
} IN;

in vec2 tex;

uniform sampler2D s0;

out vec4 color;
vec2 warp=vec2(warpX,warpY); 

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
vec3 Fetch(vec2 pos,vec2 off,  vec2 texture_size)
{
	pos = (floor(pos * texture_size + off) + vec2(0.5, 0.5)) / texture_size;
#ifdef SIMPLE_LINEAR_GAMMA	
	return ToLinear(brightboost * pow(texture(s0,pos.xy).rgb, 2.2));
#else
	return ToLinear(brightboost * texture(s0, pos.xy).rgb);
#endif	
}

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

// Distance in emulated pixels to nearest texel.
vec2 Dist(vec2 pos, vec2 texture_size)
{
	pos = pos * IN.texture_size.xy;
	return -((pos - floor(pos)) - vec2(0.5));
}

// 1D Gaussian.
float Gaus(float pos, float scale)
{
	return exp2(scale*pow(abs(pos),shape));
}

// 3-tap Gaussian filter along horz line.
vec3 Horz3(vec2 pos, float off, vec2 texture_size)
{
	vec3 b = Fetch(pos, vec2(-1.0,off),IN.texture_size.xy);
	vec3 c = Fetch(pos, vec2(0.0, off),IN.texture_size.xy);
	vec3 d = Fetch(pos, vec2(1.0, off),IN.texture_size.xy);
	float dst = Dist(pos,IN.texture_size.xy).x;
	// Convert distance to weight.
	float scale = hardPix;
	float wb = Gaus(dst - 1.0, scale);
	float wc = Gaus(dst + 0.0, scale);
	float wd = Gaus(dst + 1.0, scale);
	// Return filtered sample.
	return (b * wb + c * wc + d * wd) / (wb + wc + wd);
}

// 5-tap Gaussian filter along horz line.
vec3 Horz5(vec2 pos, float off, vec2 texture_size)
{
	vec3 a = Fetch(pos, vec2(-2.0, off),IN.texture_size.xy);
	vec3 b = Fetch(pos, vec2(-1.0, off),IN.texture_size.xy);
	vec3 c = Fetch(pos, vec2(0.0, off),IN.texture_size.xy);
	vec3 d = Fetch(pos, vec2(1.0, off),IN.texture_size.xy);
	vec3 e = Fetch(pos, vec2(2.0, off),IN.texture_size.xy);
	float dst = Dist(pos,IN.texture_size.xy).x;
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

// 7-tap Gaussian filter along horz line.
vec3 Horz7(vec2 pos, float off, vec2 texture_size){
  vec3 a=Fetch(pos,vec2(-3.0,off),IN.texture_size.xy);
  vec3 b=Fetch(pos,vec2(-2.0,off),IN.texture_size.xy);
  vec3 c=Fetch(pos,vec2(-1.0,off),IN.texture_size.xy);
  vec3 d=Fetch(pos,vec2( 0.0,off),IN.texture_size.xy);
  vec3 e=Fetch(pos,vec2( 1.0,off),IN.texture_size.xy);
  vec3 f=Fetch(pos,vec2( 2.0,off),IN.texture_size.xy);
  vec3 g=Fetch(pos,vec2( 3.0,off),IN.texture_size.xy);
  float dst=Dist(pos,IN.texture_size.xy).x;
  // Convert distance to weight.
  float scale=hardBloomPix;
  float wa=Gaus(dst-3.0,scale);
  float wb=Gaus(dst-2.0,scale);
  float wc=Gaus(dst-1.0,scale);
  float wd=Gaus(dst+0.0,scale);
  float we=Gaus(dst+1.0,scale);
  float wf=Gaus(dst+2.0,scale);
  float wg=Gaus(dst+3.0,scale);
  // Return filtered sample.
  return (a*wa+b*wb+c*wc+d*wd+e*we+f*wf+g*wg)/(wa+wb+wc+wd+we+wf+wg);
 }
  
// Return scanline weight.
float Scan(vec2 pos, float off, vec2 texture_size)
{
	float dst = Dist(pos,IN.texture_size.xy).y;
	return Gaus(dst + off, hardScan);
}

  // Return scanline weight for bloom.
float BloomScan(vec2 pos,float off, vec2 texture_size){
  float dst=Dist(pos,IN.texture_size.xy).y;
  return Gaus(dst+off,hardBloomScan);}
  
// Allow nearest three lines to effect pixel.
vec3 Tri(vec2 pos, vec2 texture_size)
{
	vec3 a = Horz3(pos, -1.0,IN.texture_size.xy);
	vec3 b = Horz5(pos, 0.0 ,IN.texture_size.xy);
	vec3 c = Horz3(pos, 1.0,IN.texture_size.xy);
	float wa = Scan(pos, -1.0,IN.texture_size.xy);
	float wb = Scan(pos, 0.0,IN.texture_size.xy);
	float wc = Scan(pos, 1.0,IN.texture_size.xy);
	return a * wa + b * wb + c * wc;
}

// Small bloom.
vec3 Bloom(vec2 pos, vec2 texture_size){
  vec3 a=Horz5(pos,-2.0,IN.texture_size.xy);
  vec3 b=Horz7(pos,-1.0,IN.texture_size.xy);
  vec3 c=Horz7(pos, 0.0,IN.texture_size.xy);
  vec3 d=Horz7(pos, 1.0,IN.texture_size.xy);
  vec3 e=Horz5(pos, 2.0,IN.texture_size.xy);
  float wa=BloomScan(pos,-2.0,IN.texture_size.xy);
  float wb=BloomScan(pos,-1.0,IN.texture_size.xy);
  float wc=BloomScan(pos, 0.0,IN.texture_size.xy);
  float wd=BloomScan(pos, 1.0,IN.texture_size.xy);
  float we=BloomScan(pos, 2.0,IN.texture_size.xy);
  return a * wa + b * wb + c * wc + d * wd + e * we;
}
  
// Distortion of scanlines, and end of screen alpha.
vec2 Warp(vec2 pos)
{
	pos = pos * 2.0 -1.0;
	pos *= vec2(1.0 + (pos.y * pos.y) * warp.x, 1.0 + (pos.x * pos.x) * warp.y);
	return pos * 0.5 + 0.5;
}

// Shadow mask.
vec3 Mask(vec2 pos)
{
	vec3 mask = vec3(maskDark, maskDark, maskDark);
	
	  // Very compressed TV style shadow mask.
	if (shadowMask == 1) {
		float mask_line = maskLight;
		float odd		= 0.0;
		if (fract(pos.x/6.0) <0.5)
		{
			odd = 1.0;
		}
		
		if (fract((pos.y+odd)/2.0) <0.5)
		{
			mask_line = maskDark;  
		}
		
		pos.x = fract(pos.x/3.0);
   
		if (pos.x<0.333)
		{
			mask.r=maskLight;
		}
		else if(pos.x<0.666)
		{
			mask.g=maskLight;
		}
		else
		{
			mask.b = maskLight;
		}
		mask *= mask_line;  
	} 
  
	// Aperture-grille.
	else if (shadowMask == 2)
	{
		pos.x = fract(pos.x/3.0);

		if(pos.x<0.333)
		{
			mask.r=maskLight;
		}
		else if (pos.x<0.666)
		{
			mask.g=maskLight;
		}
		else
		{
			mask.b=maskLight;
		}
	} 
  
	// Stretched VGA style shadow mask (same as prior shaders).
	else if (shadowMask == 3) {
		
		pos.x+=pos.y*3.0;
		pos.x=fract(pos.x/6.0);

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
	}
  
    // VGA style shadow mask.
	else if (shadowMask == 4)
	{
		pos.xy = floor(pos.xy*vec2(1.0,0.5));
		pos.x += pos.y*3.0;
		pos.x  = fract(pos.x/6.0);

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
	}
	
	if (shadowMask == 5) {
		float mask_line = maskLight;
		float odd		= 0.0;
		if (fract(pos.y/shadowmaskwight) <0.6666666666666667)
		{
			odd = 10.0;
		}
		
	
		if (fract(pos.y/shadowmaskwight) <0.6666666666666667) {
			mask.r = maskLight;
			mask.g = maskLight;
			mask.b = maskLight;
		} else {
			mask.r = maskDark;
			mask.g = maskDark;
			mask.b = maskDark;
		}
	
		if (fract((pos.y+odd)/shadowmaskwight) <0.6666666666666667)
		{
			mask_line += maskDark;  
		}
		
		
		mask *= mask_line;  
	} 
	
	if (shadowMask == 6) {
		float mask_line = maskLight;
		float odd		= 0.0;
		if (fract(pos.y/2.0) <0.5)
		{
			odd = 1.0;
		}
		
		if (fract(pos.y/2.0) <0.5) {
			mask.r = maskLight;
			mask.g = maskLight;
			mask.b = maskLight;
		} else {
			mask.r = maskDark;
			mask.g = maskDark;
			mask.b = maskDark;
		}
	
		if (fract((pos.y+odd)/2.0) <0.5)
		{
			mask_line = maskDark;  
		}
		
			pos.y = fract(pos.x/3.0);

		if(pos.y<0.333)
		{
			mask.r+=maskLight;
		}
		else if (pos.y<0.666)
		{
			mask.g+=maskLight;
		}
		else
		{
			mask.b+=maskLight;
		}
		
		mask *= mask_line;  
	} 	
	
	return mask;
}


void main()
{
	
	vec2 pos = Warp(tex.xy * (IN.texture_size.xy / IN.video_size.xy)) * (IN.video_size.xy / IN.texture_size.xy);
	vec3 outColor;
		
	outColor += Tri(pos, IN.texture_size.xy);
	outColor *= drawRectangle(pos);	
		
	outColor *= vec3(box(pos, vec2(0.5), 0.01));	
	
#ifdef DO_BLOOM
  //Add Bloom
		outColor.rgb+=Bloom(pos, IN.texture_size.xy)*bloomAmount;
#endif
	
	if (shadowMask != 0)
	{
		outColor.rgb *= Mask(floor(tex.xy * (IN.texture_size.xy / IN.video_size.xy) * IN.output_size.xy) + vec2(0.5, 0.5));
	}	

	color = vec4(ToSrgb(outColor.rgb), 1.0);

}
