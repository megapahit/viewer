/** 
 * @file lllegacyatmospherics.cpp
 * @brief LLAtmospherics class implementation
 *
 * $LicenseInfo:firstyear=2001&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "lllegacyatmospherics.h"

#include "llfeaturemanager.h"
#include "llviewercontrol.h"
#include "llframetimer.h"

#include "llagent.h"
#include "llagentcamera.h"
#include "lldrawable.h"
#include "llface.h"
#include "llglheaders.h"
#include "llsky.h"
#include "llviewercamera.h"
#include "llviewertexturelist.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
#include "llworld.h"
#include "pipeline.h"
#include "v3colorutil.h"

#include "llsettingssky.h"
#include "llenvironment.h"
#include "lldrawpoolwater.h"

class LLFastLn
{
public:
	LLFastLn() 
	{
		mTable[0] = 0;
		for( S32 i = 1; i < 257; i++ )
		{
			mTable[i] = log((F32)i);
		}
	}

	F32 ln( F32 x )
	{
		const F32 OO_255 = 0.003921568627450980392156862745098f;
		const F32 LN_255 = 5.5412635451584261462455391880218f;

		if( x < OO_255 )
		{
			return log(x);
		}
		else
		if( x < 1 )
		{
			x *= 255.f;
			S32 index = llfloor(x);
			F32 t = x - index;
			F32 low = mTable[index];
			F32 high = mTable[index + 1];
			return low + t * (high - low) - LN_255;
		}
		else
		if( x <= 255 )
		{
			S32 index = llfloor(x);
			F32 t = x - index;
			F32 low = mTable[index];
			F32 high = mTable[index + 1];
			return low + t * (high - low);
		}
		else
		{
			return log( x );
		}
	}

	F32 pow( F32 x, F32 y )
	{
		return (F32)LL_FAST_EXP(y * ln(x));
	}


private:
	F32 mTable[257]; // index 0 is unused
};

static LLFastLn gFastLn;


// Functions used a lot.

inline F32 LLHaze::calcPhase(const F32 cos_theta) const
{
	const F32 g2 = mG * mG;
	const F32 den = 1 + g2 - 2 * mG * cos_theta;
	return (1 - g2) * gFastLn.pow(den, -1.5);
}

inline void color_pow(LLColor3 &col, const F32 e)
{
	col.mV[0] = gFastLn.pow(col.mV[0], e);
	col.mV[1] = gFastLn.pow(col.mV[1], e);
	col.mV[2] = gFastLn.pow(col.mV[2], e);
}

inline LLColor3 color_norm(const LLColor3 &col)
{
	const F32 m = color_max(col);
	if (m > 1.f)
	{
		return 1.f/m * col;
	}
	else return col;
}

inline void color_gamma_correct(LLColor3 &col)
{
	const F32 gamma_inv = 1.f/1.2f;
	if (col.mV[0] != 0.f)
	{
		col.mV[0] = gFastLn.pow(col.mV[0], gamma_inv);
	}
	if (col.mV[1] != 0.f)
	{
		col.mV[1] = gFastLn.pow(col.mV[1], gamma_inv);
	}
	if (col.mV[2] != 0.f)
	{
		col.mV[2] = gFastLn.pow(col.mV[2], gamma_inv);
	}
}

static LLColor3 calc_air_sca_sea_level()
{
	static LLColor3 WAVE_LEN(675, 520, 445);
	static LLColor3 refr_ind = refr_ind_calc(WAVE_LEN);
	static LLColor3 n21 = refr_ind * refr_ind - LLColor3(1, 1, 1);
	static LLColor3 n4 = n21 * n21;
	static LLColor3 wl2 = WAVE_LEN * WAVE_LEN * 1e-6f;
	static LLColor3 wl4 = wl2 * wl2;
	static LLColor3 mult_const = fsigma * 2.0f/ 3.0f * 1e24f * (F_PI * F_PI) * n4;
	static F32 dens_div_N = F32( ATM_SEA_LEVEL_NDENS / Ndens2);
	return dens_div_N * mult_const.divide(wl4);
}

// static constants.
LLColor3 const LLHaze::sAirScaSeaLevel = calc_air_sca_sea_level();
F32 const LLHaze::sAirScaIntense = color_intens(LLHaze::sAirScaSeaLevel);	
F32 const LLHaze::sAirScaAvg = LLHaze::sAirScaIntense / 3.f;

/***************************************
		Atmospherics
***************************************/

LLAtmospherics::LLAtmospherics()
: 	mCloudDensity(0.2f),
	mWind(0.f),
	mWorldScale(1.f)
{
	/// WL PARAMS
	mInitialized = FALSE;
	mAmbientScale = gSavedSettings.getF32("SkyAmbientScale");
	mNightColorShift = gSavedSettings.getColor3("SkyNightColorShift");
	mFogColor.mV[VRED] = mFogColor.mV[VGREEN] = mFogColor.mV[VBLUE] = 0.5f;
	mFogColor.mV[VALPHA] = 0.0f;
	mFogRatio = 1.2f;
	mHazeConcentration = 0.f;
	mInterpVal = 0.f;
}


LLAtmospherics::~LLAtmospherics()
{
}

void LLAtmospherics::init()
{
   	const F32 haze_int = color_intens(mHaze.calcSigSca(0));
	mHazeConcentration = haze_int / (color_intens(mHaze.calcAirSca(0)) + haze_int);
	mInitialized = true;
}

LLColor4 LLAtmospherics::calcSkyColorInDir(AtmosphericsVars& vars, const LLVector3 &dir, bool isShiny)
{
    LLSettingsSky::ptr_t psky = LLEnvironment::instance().getCurrentSky();

	F32 saturation = 0.3f;

	if (isShiny && dir.mV[VZ] < -0.02f)
	{
		LLColor4 col = LLColor4(llmax(mFogColor[0],0.2f), llmax(mFogColor[1],0.2f), llmax(mFogColor[2],0.22f),0.f);
		LLColor3 desat_fog = LLColor3(mFogColor);
		F32 brightness = desat_fog.brightness();
		// So that shiny somewhat shows up at night.
		if (brightness < 0.15f)
		{
			brightness = 0.15f;
			desat_fog = smear(0.15f);
		}
		LLColor3 greyscale = smear(brightness);
		desat_fog = desat_fog * saturation + greyscale * (1.0f - saturation);
		if (!gPipeline.canUseWindLightShaders())
		{
			col = LLColor4(desat_fog, 0.f);
		}
		else 
		{
			col = LLColor4(desat_fog * 0.5f, 0.f);
		}
		float x = 1.0f-fabsf(-0.1f-dir.mV[VZ]);
		x *= x;
		col.mV[0] *= x*x;
		col.mV[1] *= powf(x, 2.5f);
		col.mV[2] *= x*x*x;
		return col;
	}

	// undo OGL_TO_CFR_ROTATION and negate vertical direction.
	LLVector3 Pn = LLVector3(-dir[1] , -dir[2], -dir[0]);

	calcSkyColorWLVert(Pn, vars);

	LLColor3 sky_color =  isShiny ? vars.hazeColor : psky->gammaCorrect(vars.hazeColor * 2.0f);

	if (isShiny)
	{
		F32 brightness = sky_color.brightness();
		LLColor3 greyscale = smear(brightness);
		sky_color = sky_color * saturation + greyscale * (1.0f - saturation);
		sky_color *= (0.5f + 0.5f * brightness);
	}
	return LLColor4(sky_color, 0.0f);
}

const F32 NIGHTTIME_ELEVATION = -8.0f; // degrees
const F32 NIGHTTIME_ELEVATION_SIN = (F32)sinf(NIGHTTIME_ELEVATION*DEG_TO_RAD);

void LLAtmospherics::calcSkyColorWLVert(LLVector3 & Pn, AtmosphericsVars& vars)
{
    LLSettingsSky::ptr_t psky = LLEnvironment::instance().getCurrentSky();

    LLColor3    blue_density = vars.blue_density;
    LLColor3    blue_horizon = vars.blue_horizon;
    F32         haze_horizon = vars.haze_horizon;
    F32         haze_density = vars.haze_density;
    F32         density_multiplier = vars.density_multiplier;
    F32         distance_multiplier = vars.distance_multiplier;
    F32         max_y = vars.max_y;
    LLVector4   sun_norm = vars.sun_norm;

	// project the direction ray onto the sky dome.
	F32 phi = acos(Pn[1]);
	F32 sinA = sin(F_PI - phi);
	if (fabsf(sinA) < 0.01f)
	{ //avoid division by zero
		sinA = 0.01f;
	}

	F32 Plen = vars.dome_radius * sin(F_PI + phi + asin(vars.dome_offset * sinA)) / sinA;

	Pn *= Plen;

	// Set altitude
	if (Pn[1] > 0.f)
	{
		Pn *= (max_y / Pn[1]);
	}
	else
	{
		Pn *= (-32000.f / Pn[1]);
	}

	Plen = Pn.length();
	Pn /= Plen;

	// Initialize temp variables
	LLColor3 sunlight = vars.sunlight;
    LLColor3 ambient = vars.ambient;
    
    LLColor3 glow = vars.glow;
    F32 cloud_shadow = vars.cloud_shadow;

	// Sunlight attenuation effect (hue and brightness) due to atmosphere
	// this is used later for sunlight modulation at various altitudes
	LLColor3 light_atten = vars.light_atten;
    LLColor3 light_transmittance = psky->getLightTransmittance(Plen);

	// Calculate relative weights
	LLColor3 temp2(0.f, 0.f, 0.f);
	LLColor3 temp1 = vars.total_density;

	LLColor3 blue_weight = componentDiv(blue_density, temp1);
	LLColor3 haze_weight = componentDiv(smear(haze_density), temp1);

    F32 lighty = sun_norm.mV[1];
	if(lighty < NIGHTTIME_ELEVATION_SIN)
	{
		lighty = -lighty;
	}

	// Compute sunlight from P & lightnorm (for long rays like sky)
    temp2.mV[1] = llmax(F_APPROXIMATELY_ZERO, llmax(0.f, Pn[1]) * 1.0f + sun_norm.mV[1] );

    temp2.mV[1] = 1.f / temp2.mV[1];
    componentMultBy(sunlight, componentExp((light_atten * -1.f) * temp2.mV[1]));
    //componentMultBy(sunlight, light_transmittance);

    // Distance
	temp2.mV[2] = Plen * density_multiplier;

    // Transparency (-> temp1)
	temp1 = componentExp((temp1 * -1.f) * temp2.mV[2]);// * distance_multiplier);
    (void)distance_multiplier;

	// Compute haze glow
	temp2.mV[0] = Pn * LLVector3(sun_norm);

	temp2.mV[0] = 1.f - temp2.mV[0];
		// temp2.x is 0 at the sun and increases away from sun
	temp2.mV[0] = llmax(temp2.mV[0], .001f);	
		// Set a minimum "angle" (smaller glow.y allows tighter, brighter hotspot)

	// Higher glow.x gives dimmer glow (because next step is 1 / "angle")
	temp2.mV[0] *= glow.mV[0];

	temp2.mV[0] = pow(temp2.mV[0], glow.mV[2]);
		// glow.z should be negative, so we're doing a sort of (1 / "angle") function

	// Add "minimum anti-solar illumination"
	temp2.mV[0] += .25f;


	// Haze color above cloud
	vars.hazeColor = (blue_horizon * blue_weight * (sunlight + ambient) + componentMult(haze_horizon * haze_weight, sunlight * temp2.mV[0] + ambient));	

	// Increase ambient when there are more clouds
	LLColor3 tmpAmbient = ambient + (LLColor3::white - ambient) * cloud_shadow * 0.5f;

	// Dim sunlight by cloud shadow percentage
	sunlight *= (1.f - cloud_shadow);

	// Haze color below cloud
	vars.hazeColorBelowCloud = (blue_horizon * blue_weight * (sunlight + tmpAmbient) + componentMult(haze_horizon * haze_weight, sunlight * temp2.mV[0] + tmpAmbient));	

    LLColor3 final_atten = LLColor3::white - temp1;
    final_atten.mV[0] = llmax(final_atten.mV[0], 0.0f);
    final_atten.mV[1] = llmax(final_atten.mV[1], 0.0f);
    final_atten.mV[2] = llmax(final_atten.mV[2], 0.0f);

	// Final atmosphere additive
	componentMultBy(vars.hazeColor, LLColor3::white - temp1);

    // Attenuate cloud color by atmosphere
	temp1 = componentSqrt(temp1);	//less atmos opacity (more transparency) below clouds

	// At horizon, blend high altitude sky color towards the darker color below the clouds
	vars.hazeColor += componentMult(vars.hazeColorBelowCloud - vars.hazeColor, LLColor3::white - componentSqrt(temp1));
}

void LLAtmospherics::updateFog(const F32 distance, const LLVector3& tosun_in)
{
    LLVector3 tosun = tosun_in;

	if (!gPipeline.hasRenderDebugFeatureMask(LLPipeline::RENDER_DEBUG_FEATURE_FOG))
	{
		if (!LLGLSLShader::sNoFixedFunction)
		{
			glFogf(GL_FOG_DENSITY, 0);
			glFogfv(GL_FOG_COLOR, (F32 *) &LLColor4::white.mV);
			glFogf(GL_FOG_END, 1000000.f);
		}
		return;
	}

	const BOOL hide_clip_plane = TRUE;
	LLColor4 target_fog(0.f, 0.2f, 0.5f, 0.f);

	const F32 water_height = gAgent.getRegion() ? gAgent.getRegion()->getWaterHeight() : 0.f;
	// LLWorld::getInstance()->getWaterHeight();
	F32 camera_height = gAgentCamera.getCameraPositionAgent().mV[2];

	F32 near_clip_height = LLViewerCamera::getInstance()->getAtAxis().mV[VZ] * LLViewerCamera::getInstance()->getNear();
	camera_height += near_clip_height;

	F32 fog_distance = 0.f;
	LLColor3 res_color[3];

	LLColor3 sky_fog_color = LLColor3::white;
	LLColor3 render_fog_color = LLColor3::white;

	const F32 tosun_z = tosun.mV[VZ];
	tosun.mV[VZ] = 0.f;
	tosun.normalize();
	LLVector3 perp_tosun;
	perp_tosun.mV[VX] = -tosun.mV[VY];
	perp_tosun.mV[VY] = tosun.mV[VX];
	LLVector3 tosun_45 = tosun + perp_tosun;
	tosun_45.normalize();

	F32 delta = 0.06f;
	tosun.mV[VZ] = delta;
	perp_tosun.mV[VZ] = delta;
	tosun_45.mV[VZ] = delta;
	tosun.normalize();
	perp_tosun.normalize();
	tosun_45.normalize();

	// Sky colors, just slightly above the horizon in the direction of the sun, perpendicular to the sun, and at a 45 degree angle to the sun.
    AtmosphericsVars vars;

    LLSettingsSky::ptr_t psky = LLEnvironment::instance().getCurrentSky();

    // invariants across whole sky tex process...
    vars.blue_density = psky->getBlueDensity();    
    vars.blue_horizon = psky->getBlueHorizon();
    vars.haze_density = psky->getHazeDensity();
    vars.haze_horizon = psky->getHazeHorizon();
    vars.density_multiplier = psky->getDensityMultiplier();    
    vars.distance_multiplier = psky->getDistanceMultiplier();
    vars.max_y = psky->getMaxY();
    vars.sun_norm = LLEnvironment::instance().getLightDirectionCFR();
    vars.sunlight = psky->getSunlightColor();
    vars.ambient = psky->getAmbientColor();    
    vars.glow = psky->getGlow();
    vars.cloud_shadow = psky->getCloudShadow();
    vars.dome_radius = psky->getDomeRadius();
    vars.dome_offset = psky->getDomeOffset();
    vars.light_atten = psky->getLightAttenuation(vars.max_y);
    vars.light_transmittance = psky->getLightTransmittance(vars.max_y);
    vars.total_density = psky->getTotalDensity();
    vars.gamma = psky->getGamma();

	res_color[0] = calcSkyColorInDir(vars, tosun);
	res_color[1] = calcSkyColorInDir(vars, perp_tosun);
	res_color[2] = calcSkyColorInDir(vars, tosun_45);

	sky_fog_color = color_norm(res_color[0] + res_color[1] + res_color[2]);

	F32 full_off = -0.25f;
	F32 full_on = 0.00f;
	F32 on = (tosun_z - full_off) / (full_on - full_off);
	on = llclamp(on, 0.01f, 1.f);
	sky_fog_color *= 0.5f * on;


	// We need to clamp these to non-zero, in order for the gamma correction to work. 0^y = ???
	S32 i;
	for (i = 0; i < 3; i++)
	{
		sky_fog_color.mV[i] = llmax(0.0001f, sky_fog_color.mV[i]);
	}

	color_gamma_correct(sky_fog_color);

	render_fog_color = sky_fog_color;

	F32 fog_density = 0.f;
	fog_distance = mFogRatio * distance;
	
	if (camera_height > water_height)
	{
		LLColor4 fog(render_fog_color);
		if (!LLGLSLShader::sNoFixedFunction)
		{
			glFogfv(GL_FOG_COLOR, fog.mV);
		}
		mGLFogCol = fog;

		if (hide_clip_plane)
		{
			// For now, set the density to extend to the cull distance.
			const F32 f_log = 2.14596602628934723963618357029f; // sqrt(fabs(log(0.01f)))
			fog_density = f_log/fog_distance;
			if (!LLGLSLShader::sNoFixedFunction)
			{
				glFogi(GL_FOG_MODE, GL_EXP2);
			}
		}
		else
		{
			const F32 f_log = 4.6051701859880913680359829093687f; // fabs(log(0.01f))
			fog_density = (f_log)/fog_distance;
			if (!LLGLSLShader::sNoFixedFunction)
			{
				glFogi(GL_FOG_MODE, GL_EXP);
			}
		}
	}
	else
	{
        LLSettingsWater::ptr_t pwater = LLEnvironment::instance().getCurrentWater();
		F32 depth = water_height - camera_height;
		
		// get the water param manager variables
        float water_fog_density = pwater->getModifiedWaterFogDensity(depth <= 0.0f);

		LLColor4 water_fog_color(pwater->getWaterFogColor());

		// adjust the color based on depth.  We're doing linear approximations
		float depth_scale = gSavedSettings.getF32("WaterGLFogDepthScale");
		float depth_modifier = 1.0f - llmin(llmax(depth / depth_scale, 0.01f), 
			gSavedSettings.getF32("WaterGLFogDepthFloor"));

		LLColor4 fogCol = water_fog_color * depth_modifier;
		fogCol.setAlpha(1);

		// set the gl fog color
		mGLFogCol = fogCol;

		// set the density based on what the shaders use
		fog_density = water_fog_density * gSavedSettings.getF32("WaterGLFogDensityScale");

		if (!LLGLSLShader::sNoFixedFunction)
		{
			glFogfv(GL_FOG_COLOR, (F32 *) &fogCol.mV);
			glFogi(GL_FOG_MODE, GL_EXP2);
		}
	}

	mFogColor = sky_fog_color;
	mFogColor.setAlpha(1);

	LLDrawPoolWater::sWaterFogEnd = fog_distance*2.2f;

	if (!LLGLSLShader::sNoFixedFunction)
	{
		LLGLSFog gls_fog;
		glFogf(GL_FOG_END, fog_distance*2.2f);
		glFogf(GL_FOG_DENSITY, fog_density);
		glHint(GL_FOG_HINT, GL_NICEST);
	}
	stop_glerror();
}

// Functions used a lot.
F32 color_norm_pow(LLColor3& col, F32 e, BOOL postmultiply)
{
	F32 mv = color_max(col);
	if (0 == mv)
	{
		return 0;
	}

	col *= 1.f / mv;
	color_pow(col, e);
	if (postmultiply)
	{
		col *= mv;
	}
	return mv;
}

// Returns angle (RADIANs) between the horizontal projection of "v" and the x_axis.
// Range of output is 0.0f to 2pi //359.99999...f
// Returns 0.0f when "v" = +/- z_axis.
F32 azimuth(const LLVector3 &v)
{
	F32 azimuth = 0.0f;
	if (v.mV[VX] == 0.0f)
	{
		if (v.mV[VY] > 0.0f)
		{
			azimuth = F_PI * 0.5f;
		}
		else if (v.mV[VY] < 0.0f)
		{
			azimuth = F_PI * 1.5f;// 270.f;
		}
	}
	else
	{
		azimuth = (F32) atan(v.mV[VY] / v.mV[VX]);
		if (v.mV[VX] < 0.0f)
		{
			azimuth += F_PI;
		}
		else if (v.mV[VY] < 0.0f)
		{
			azimuth += F_PI * 2;
		}
	}	
	return azimuth;
}
