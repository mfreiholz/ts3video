#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect texY;
uniform sampler2DRect texU;
uniform sampler2DRect texV;

uniform float height;

// xyzw, rgba (for colors), or stpq (for texture coordinates)

void main()
{
	highp float y, u, v;
	highp float r, g, b;
	
	float nx = gl_TexCoord[0].x;
	float ny = height - gl_TexCoord[0].y;

	y = texture2DRect( texY, vec2( nx, ny ) ).x;
	u = texture2DRect( texV, vec2( floor(nx / 2.0), floor(ny / 2.0) ) ).x;
	v = texture2DRect( texU, vec2( floor(nx / 2.0), floor(ny / 2.0) ) ).x;

	y = 1.1643 * ( y - 0.0625 );
	u = u - 0.5;
	v = v - 0.5;

	r = y + 1.5958 * v;
	g = y - 0.39173 * u - 0.81290 * v;
	b = y + 2.017 * u;

	//gl_FragColor = highp vec4( y, y, y, 1.0 );
	gl_FragColor = highp vec4( r, g, b, 1.0 );
}
