VSH    o>< u_modelViewProj     attribute vec2 a_position;
attribute vec2 a_texcoord0;
varying vec2 v_texcoord0;
uniform mat4 u_modelViewProj;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.zw = vec2(0.0, 1.0);
  tmpvar_1.xy = a_position;
  gl_Position = (u_modelViewProj * tmpvar_1);
  v_texcoord0 = a_texcoord0;
}

 