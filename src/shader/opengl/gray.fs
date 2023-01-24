varying vec2 v_texCoord;
uniform sampler2D tex0;

void main()
{
	vec4 color = texture2D(tex0, v_texCoord);
	float gray = color.r * 0.299 + color.g * 0.587 + color.b * 0.114;
	color.rgb = vec3(gray, gray, gray);	
	gl_FragColor = color;	
}