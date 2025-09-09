out vec4 frag_color;

in vec2 vary_texcoord0;

uniform sampler2D diffuseMap;
uniform sampler2D bloomBlurredMap;

uniform float bloomStrength;
uniform float bloomClampValue;

void main()
{
    vec4 hdrColor = texture(diffuseMap, vary_texcoord0);
    vec4 bloomColor = texture(bloomBlurredMap, vary_texcoord0);
    vec4 result = hdrColor;

    result.rgb += bloomStrength * bloomColor.rgb;
    result.rgb = clamp(result.rgb, vec3(0.0), vec3(bloomClampValue));

    frag_color = result;
}