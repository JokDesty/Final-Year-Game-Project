FSHo><     textureSampler    
tint_color     varying highp vec2 v_texcoord0;
uniform sampler2D textureSampler;
uniform highp vec4 tint_color;
void main ()
{
  lowp vec4 tmpvar_1;
  tmpvar_1.xyz = tint_color.xyz;
  tmpvar_1.w = (texture2D (textureSampler, v_texcoord0).w * tint_color.w);
  gl_FragColor = tmpvar_1;
}

 