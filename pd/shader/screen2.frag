// Cyrille Henry 2014
// draw only part of the texture

#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect MyTex;
//uniform sampler2D MyTex;
uniform float posX, posY; // position in the display matrix
uniform float tailleX, tailleY; // size of the display matrix
uniform float imageX, imageY; // image resolution in pixels
uniform float screen; // limitation sur 1 ecran ou non
uniform vec2 taille_masque_sur_taille_global;
uniform float sx,sy,tx,ty;

void main (void)
{
 vec2 coord = (gl_TextureMatrix[0] * gl_TexCoord[0]).st;
 vec2 pos_inf = vec2(imageX / tailleX, imageY / tailleY);
 vec2 pos_sup = pos_inf * vec2(posX, posY);
 pos_inf *= vec2(posX-1.,posY-1.);
 vec2 inside = step(pos_inf, coord) * step(coord, pos_sup);
 

 coord /= vec2(sx,sy);
 coord += vec2(tx,ty);
 vec4 color = texture2DRect(MyTex, coord );
 float limitation = inside.x * inside.y;
 limitation += abs(screen-1.);
 limitation = clamp(limitation, 0., 1.);
  color.a *= limitation; 
 gl_FragColor = color;
}
