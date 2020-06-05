// Default fragment shader for PREMULTIPLIED blend mode

uniform sampler2D texture;

void main()
{
	// lookup the pixel in the texture
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

	// multiply it by the color
	gl_FragColor = gl_Color * pixel;
	
	// this must be the last line in this shader
	// * sign(pixel.w) fixes the white border around some transparent pngs
	gl_FragColor.xyz *= gl_Color.w * sign(pixel.w);
}
