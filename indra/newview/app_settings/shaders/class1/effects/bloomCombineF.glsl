out vec4 frag_color;

in vec2 vary_texcoord0;

uniform sampler2D diffuseMap;
uniform sampler2D bloomBlurredMap;

uniform float bloomStrength;

void main()
{
    vec4 hdrColor = texture(diffuseMap, vary_texcoord0);
    vec3 bloomColor = texture(bloomBlurredMap, vary_texcoord0).rgb;
    vec4 result = vec4(0.0);

    result.r = min(hdrColor.r + bloomStrength * bloomColor.r, 1.0);
    result.g = min(hdrColor.g + bloomStrength * bloomColor.g, 1.0);
    result.b = min(hdrColor.b + bloomStrength * bloomColor.b, 1.0);
    result.a = hdrColor.a;

    //bloomColor += hdrColor.rgb;
    frag_color = result;
}