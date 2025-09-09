in vec3 position;
out vec2 vary_texcoord0;

void main()
{
	gl_Position = vec4(position, 1.0);
	vary_texcoord0.xy = position.xy * 0.5 + 0.5;
}