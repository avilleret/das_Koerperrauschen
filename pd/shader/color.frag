uniform sampler2D texture;

uniform vec4 C1,C2,C3,C4;      // couleur au 4 cotés
uniform float powX,powY;
uniform float T, Noise;
uniform float img;

void main(void)
{
	vec4 front, back;

	float xs1 = sin(gl_TexCoord[0].s*gl_TexCoord[0].t);
	float xs2 = sin(T+xs1*533.);
	float xs3 = sin(2.*T+xs2*1013.);

	float ys2 = sin(xs1*5313.);
	float ys3 = sin(ys2*10113.);

	front = mix(C1,C2,pow(gl_TexCoord[0].t,powY));
	back = mix(C3,C4,pow(gl_TexCoord[0].t,powY));

    vec4 color = mix(front,back,pow(gl_TexCoord[0].s,powX));


    vec4 color_img = texture2D(texture, gl_TexCoord[0].st);
    color.rgb = mix(color.rgb,color_img.rgb,color_img.a*img);

    color.a += step(Noise,abs(xs3));

	gl_FragColor = color;

}
