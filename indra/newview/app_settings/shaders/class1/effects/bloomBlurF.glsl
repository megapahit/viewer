out vec4 frag_color;

in vec2 vary_texcoord0;

uniform sampler2D bloomEMap;

uniform bool bloomHorizontal;
uniform float bloomBlurRadius = 1.5;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 size = vec2(bloomBlurRadius, bloomBlurRadius);

    vec2 tex_offset = size / textureSize(bloomEMap, 0); // gets size of single texel
    vec3 result = texture(bloomEMap, vary_texcoord0).rgb * weight[0]; // current fragment's contribution

    if(bloomHorizontal)
    {
        for(int i = 1; i < 5; i++)
        {
            result += texture(bloomEMap, vary_texcoord0 + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(bloomEMap, vary_texcoord0 - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; i++)
        {
            result += texture(bloomEMap, vary_texcoord0 + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(bloomEMap, vary_texcoord0 - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }

    frag_color = vec4(result, 1.0);
}