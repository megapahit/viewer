out vec4 frag_color;

uniform sampler2D diffuseMap;
uniform sampler2D bloomExtractORM; // orm
//uniform sampler2D bloomExtractEmissive; // emissive
uniform sampler2D bloomExtractEmissive2; // emissive 2

uniform float bloomExtractBrightness = 0.9;
uniform float bloomExtractMetal = 0.20;
uniform float bloomExtractNonMetal = 0.20;

in vec2 vary_texcoord0;

void main()
{
    vec4 col = texture(diffuseMap, vary_texcoord0.xy);

    if(col.a > 0.001)
    {
        discard;
        return;
    }

    //int valid = 0;
    //float brightness = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
    float brightness = dot(col.rgb, vec3(0.3, 0.5, 0.2));

    if(brightness < bloomExtractBrightness)
    {
        discard;
        return;
    }

    /*
    vec3 emi = texture(bloomExtractEmissive, vary_texcoord0.xy).rgb;
    if(emi.r + emi.g + emi.b > 0.01)
    {
        discard;
        return;
    }
    */

    vec3 emi = texture(bloomExtractEmissive2, vary_texcoord0.xy).rgb;
    if(emi.r + emi.g + emi.b > 0.01)
    {
        discard;
        return;
    }

    vec4 orm = texture(bloomExtractORM, vary_texcoord0.xy);

    if(orm.r < 0.7)
    {
        discard;
        return;
    }

    if(bloomExtractMetal == 1.0 && bloomExtractNonMetal == 1.0)
    {
        frag_color = vec4(col.rgb, 0.0);
        return;
    }

    if(orm.b < 0.15)
    {
        // non metal
        if(orm.g >= bloomExtractNonMetal)
        {
            discard;
            return;
        }
    }
    else if(orm.b > 0.8)
    {
        // metal
        if(orm.g >= bloomExtractMetal)
        {
            discard;
            return;
        }
    }
    else
    {
        discard;
        return;
    }

    frag_color = vec4(col.rgb, 0.0);
}