//varying vec2 t;

void main()
{
	//t = gl_MultiTexCoord0.xy;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}
