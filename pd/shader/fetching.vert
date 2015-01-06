// Cyrille Henry 2008
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect MyTex, interaction;
uniform float K, mixage, bump_interaction;
uniform float vertex_pixelisationX, vertex_pixelisationY, courbe;
uniform float crop_minX, crop_maxX, crop_minY, crop_maxY;

uniform float taille_interaction_sur_taille_global;

uniform float pixelisationX, pixelisationY;


vec4 texture2DRect_cubic(sampler2DRect texture, vec2 coordinate)
{
    coordinate -= vec2(0.5);

	vec2 coord = floor(coordinate)+vec2(0.5,0.5);
	vec2 interpol = fract(coordinate);
	vec2 interpol2 = interpol * interpol;
	vec2 interpol3 = interpol2 * interpol;

	vec4 C00 = texture2DRect(texture, coord + vec2(-1,-1) );
	vec4 C10 = texture2DRect(texture, coord + vec2( 0,-1) );
	vec4 C20 = texture2DRect(texture, coord + vec2( 1,-1) );
	vec4 C30 = texture2DRect(texture, coord + vec2( 2,-1) );
	vec4 C01 = texture2DRect(texture, coord + vec2(-1, 0) );
	vec4 C11 = texture2DRect(texture, coord );
	vec4 C21 = texture2DRect(texture, coord + vec2( 1, 0) );
	vec4 C31 = texture2DRect(texture, coord + vec2( 2, 0) );
	vec4 C02 = texture2DRect(texture, coord + vec2(-1, 1) );
	vec4 C12 = texture2DRect(texture, coord + vec2( 0, 1) );
	vec4 C22 = texture2DRect(texture, coord + vec2( 1, 1) );
	vec4 C32 = texture2DRect(texture, coord + vec2( 2, 1) );
	vec4 C03 = texture2DRect(texture, coord + vec2(-1, 2) );
	vec4 C13 = texture2DRect(texture, coord + vec2( 0, 2) );
	vec4 C23 = texture2DRect(texture, coord + vec2( 1, 2) );
	vec4 C33 = texture2DRect(texture, coord + vec2( 2, 2) );

	vec4 w0 = C11;
	vec4 w1 = C21;
	vec4 w2 = C12;
	vec4 w3 = C22;
	// x derivative 
	vec4 x0 = (C21 - C01) / 2.; 
	vec4 x1 = (C31 - C11) / 2.; 
	vec4 x2 = (C22 - C02) / 2.; 
	vec4 x3 = (C32 - C12) / 2.; 
	// y derivative
	vec4 y0 = (C12 - C10) / 2.;
	vec4 y1 = (C22 - C20) / 2.;
	vec4 y2 = (C13 - C11) / 2.;
	vec4 y3 = (C23 - C21) / 2.;
	// xy derivative
	vec4 z0 = (C22 - C00) / 2.;
	vec4 z1 = (C32 - C10) / 2.;
	vec4 z2 = (C23 - C01) / 2.;
	vec4 z3 = (C33 - C11) / 2.;

	vec4 a00 = w0;
	vec4 a01 = y0;
	vec4 a02 = -3.*w0 + 3.*w2 -2.*y0 - y2;
	vec4 a03 = 2.*w0 - 2.*w2 + y0 + y2;
	vec4 a10 = x0;
	vec4 a11 = z0;
	vec4 a12 = -3.*x0 + 3.*x2 - 2.*z0 - z2;
	vec4 a13 = 2.*x0 - 2.*x2 + z0 + z2;
	vec4 a20 = -3.*w0 + 3.*w1 - 2.*x0 - x1;
	vec4 a21 = -3.*y0 + 3.*y1 - 2.*z0 - z1;
	vec4 a22 = 9.*w0 - 9.*w1 - 9.*w2 + 9.*w3 + 6.*x0 + 3.*x1 + -6.*x2 - 3.*x3 + 6.*y0 - 6.*y1 + 3.*y2 - 3.*y3 + 4.*z0 + 2.*z1 + 2.*z2 + z3;
	vec4 a23 = -6.*w0 + 6.*w1 + 6.*w2 - 6.*w3 -4.*x0 - 2.*x1 + 4.*x2 + 2.*x3 -3.*y0 + 3.*y1 - 3.*y2 + 3.*y3 + -2.*z0 - z1 - 2.*z2 - z3;
	vec4 a30 = 2.*w0 - 2.*w1 + x0 + x1;
	vec4 a31 = 2.*y0 - 2.*y1 + z0 + z1;
	vec4 a32 = -6.*w0 + 6.*w1 + 6.*w2 -6.*w3 -3.*x0 - 3.*x1 + 3.*x2 + 3.*x3 -4.*y0 + 4.*y1 - 2.*y2 + 2.*y3 + -2.*z0 - 2.*z1 - z2 - z3;
	vec4 a33 = 4.*w0 - 4.*w1 - 4.*w2 + 4.*w3 + 2.*x0 + 2.*x1 + -2.*x2 - 2.*x3 + 2.*y0 - 2.*y1 + 2.*y2 - 2.*y3 + z0 + z1 + z2 + z3;


	vec4 color = a00;
	color += a01 * interpol.y;
	color += a02 * interpol2.y;
	color += a03 * interpol3.y;
	color += a10 * interpol.x;
	color += a11 * interpol.x * interpol.y;
	color += a12 * interpol.x * interpol2.y;
	color += a13 * interpol.x * interpol3.y;
	color += a20 * interpol2.x;
	color += a21 * interpol2.x * interpol.y;
	color += a22 * interpol2.x * interpol2.y;
	color += a23 * interpol2.x * interpol3.y;
	color += a30 * interpol3.x;
	color += a31 * interpol3.x * interpol.y;
	color += a32 * interpol3.x * interpol2.y;
	color += a33 * interpol3.x * interpol3.y;

	return(color);
}

void main()
{
	vec4 v = vec4(gl_Vertex);

    vec2 pos = gl_MultiTexCoord0.st;
    pos.x += crop_minX;
    pos.x *= 1. - crop_maxX/255.;
    pos.y += crop_minY;
    pos.y *= 1. - crop_maxY/255.;

    if ((vertex_pixelisationX+vertex_pixelisationY) > 0.)
    {
        vertex_pixelisationX = max(0.000001, vertex_pixelisationX);
        vertex_pixelisationY = max(0.000001, vertex_pixelisationY);
        vec2 pixelisation = vec2(vertex_pixelisationX, vertex_pixelisationY);
        pos = pixelisation * floor( pos / pixelisation);
    }

	vec4 color = texture2DRect_cubic(MyTex, pos);
    float bump =  K*mix(color.r-0.5,color.g-0.1,mixage);
    bump += bump_interaction *  texture2DRect_cubic(interaction, pos* taille_interaction_sur_taille_global);

    if (courbe != 0.)
    {

        float rayon = 1./courbe;
        rayon += bump;
        float phy = v.x*courbe;

        v.x = rayon * sin(phy);
        v.z = rayon * cos(phy);
        v.z -=  1./courbe;
    }
    else
    {
	    v.z += bump;
    }

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * v;
	
}

