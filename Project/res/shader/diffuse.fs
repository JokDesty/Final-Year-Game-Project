FSH!8�     
tint_color      varying highp vec3 v_normal;
uniform highp vec4 tint_color;
void main ()
{
  mediump vec4 tmpvar_1;
  tmpvar_1.xyz = ((vec3(0.24, 0.24, 0.24) + (
    max (normalize(v_normal).y, 0.0)
   * vec3(0.8, 0.8, 0.8))) * tint_color.xyz);
  tmpvar_1.w = tint_color.w;
  gl_FragColor = tmpvar_1;
}

 