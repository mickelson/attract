// Default fragment shader for OVERLAY blend mode
// Texture color higher than 127,127,127 brightens the scene, lower than 127,127,127 darkens it.

uniform sampler2D texture;

void main()
{
	// lookup the pixel in the texture
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

	// multiply it by the color
	gl_FragColor = gl_Color * pixel;
	
	// this must be the last line in this shader
	gl_FragColor = mix(vec4(0.5,0.5,0.5,1.0), gl_FragColor, gl_FragColor.w);
}
