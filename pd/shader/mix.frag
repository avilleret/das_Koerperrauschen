#extension GL_ARB_texture_rectangle : enable

uniform sampler2D MyTex;
uniform sampler2D MyTex1;
uniform float mixage;
uniform float colorR, colorG, colorB;

varying vec2 texcoord1;
varying vec2 texcoord2;

void main (void)
{
 vec4 color = texture2D(MyTex,  texcoord1);
 vec4 color2 = texture2D(MyTex1, texcoord1); // texcoord2 does not work.
 vec4 tmp = mix(color,color2,mixage);
 tmp.rgb *= vec3(colorR,colorG,colorB);
 gl_FragColor = tmp;
}

