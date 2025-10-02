/**
 * @file postDeferredGammaCorrect.glsl
 *
 * $LicenseInfo:firstyear=2007&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2007, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

/*[EXTRA_CODE_HERE]*/

out vec4 frag_color;

uniform sampler2D diffuseRect;

uniform float gamma = 2.2;
uniform float mpHDRBoost = 1.0;

in vec2 vary_fragcoord;

vec3 clampHDRRange(vec3 color);

vec3 HDRDisplayGamma(vec3 linearRGB)
{
    bvec3 cutoff = lessThan(linearRGB, vec3(0.0031308));
    vec3 higher = vec3(1.055)*pow(linearRGB, vec3(1.0/gamma)) - vec3(0.055);
    vec3 lower = linearRGB * vec3(12.92);

    return mix(higher, lower, cutoff);
}

void main()
{
    vec4 diff = texture(diffuseRect, vary_fragcoord);
    diff.rgb = mpHDRBoost * HDRDisplayGamma(diff.rgb);
    diff.rgb = clampHDRRange(diff.rgb);
    frag_color = diff;
}

