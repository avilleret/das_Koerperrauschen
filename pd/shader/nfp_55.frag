#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect MyTex;
uniform float Sl,Sr,St,Sb; // shade size
uniform float nlSl,nlSr,nlSt,nlSb; // shade non linearitées
uniform float calib;
varying vec2 pos;

void main (void)
{
    vec2 coord = gl_TexCoord[0].st;
    vec4 color = texture2DRect(MyTex, coord);
 
    vec4 color_calib = vec4(1.,1.,1.,1.);
    color_calib *= step(0.1,fract(pos.x*20.));
    color_calib *= step(0.1,fract(pos.y*20.));
    float color_calib2 = 1.;
    color_calib2 *= step(0.8,fract(4.*pos.x-0.1));
    color_calib2 *= step(0.8,fract(4.*pos.y-0.1));
    color_calib.gb *= 1.-color_calib2;

    color = mix(color, color_calib, calib);

    color.rgb *= pow(min(1.,pos.x/Sl),exp(nlSl));
    color.rgb *= pow(min(1.,(1.-pos.x)/Sr),exp(nlSr));
    color.rgb *= pow(min(1.,pos.y/St),exp(nlSt));
    color.rgb *= pow(min(1.,(1.-pos.y)/Sb),exp(nlSb));

    gl_FragColor = color;
}
