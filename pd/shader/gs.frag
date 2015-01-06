#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect texture;
uniform sampler2DRect interaction;

uniform float Kni, Fni, dx1ni, dx2ni, dx3ni, dx4ni;
uniform float Ki, Fi, dx1i, dx2i, dx3i, dx4i;
uniform float tmp, tmp2, tmp3, tmp4;
uniform float aff_interaction, calcul_interaction;
uniform float dim, taille_interaction_sur_taille_global;
uniform float pascal;

// const float dx = 1.; 
const float DA = 0.16;
const float DB = 0.08;
const float G = 1.;


vec4 labla(vec2 pos, float dx1, float dx2, float dx3, float dx4)
{
	vec4 grad = vec4(0.);

	
	grad += texture2DRect(texture, vec2(pos.x, mod(pos.y-dx1, dim)));
	grad += texture2DRect(texture, vec2(mod(pos.x-dx2, dim), pos.y));
	grad += texture2DRect(texture, vec2(mod(pos.x+dx3, dim), pos.y));
	grad += texture2DRect(texture, vec2(pos.x, mod(pos.y+dx4, dim)));
	grad -= 4. * texture2DRect(texture, pos);
	return(grad);
}

void main (void)
{
	vec2 pos = gl_TexCoord[0].st;
	vec4 color = texture2DRect(texture, pos);
	vec4 interact = texture2DRect(interaction, pos* taille_interaction_sur_taille_global);

	interact.b += pascal;
	interact.b = min(1.,interact.b);

	float K   = mix(Kni,   Ki,   interact.b);
	float F   = mix(Fni,   Fi,   interact.b);
	float dx1 = mix(dx1ni, dx1i, interact.b);
	float dx2 = mix(dx2ni, dx2i, interact.b);
	float dx3 = mix(dx3ni, dx3i, interact.b);
	float dx4 = mix(dx4ni, dx4i, interact.b);



	// Gray Scott
	vec4 courbure = labla(pos, dx1, dx2, dx3, dx4);
	color.r += G * ( DA * sign(courbure.r)*pow(abs(courbure.r), 1.+tmp) - sign(color.g)*pow(abs(color.g), 1.+tmp2)*color.r*color.g +  F * (1. - color.r) );
	color.g += G * ( DB * courbure.g + (1.+tmp3) *color.r*color.g*sign(color.g)*pow(abs(color.g), 1.+tmp4) - (F + K) * color.g  );
	
    color.b = 0.;
	color.a = 1.;

	gl_FragColor = mix(color, interact, aff_interaction);
}
