/**
* @file mpfloatertuning.cpp
* @brief Controller for viewer tuning
* @author observeur@megapahit.net
*
* $LicenseInfo:firstyear=2014&license=viewerlgpl$
* Second Life Viewer Source Code
* Copyright (C) 2014, Linden Research, Inc.
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

#include "mpfloatertuning.h"
#include "llsliderctrl.h"
#include "lltextbox.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llviewercontrol.h"
#include "llsdserialize.h"

MPFloaterTuning::MPFloaterTuning(const LLSD& key) : LLFloater(key)
{
}


bool MPFloaterTuning::postBuild()
{
    U32 fps = gSavedSettings.getU32("MaxFPS");
    if(fps==0) fps=132;

    mFpsSlider = getChild<LLSliderCtrl>("fpsSliderCtrl");
    mFpsSlider->setCommitCallback(boost::bind(&MPFloaterTuning::onFpsSliderChanged, this));

    mFpsSlider->setValue(fps, false);

    mFpsTextBox = getChild<LLTextBox>("fpsTextCtrl");
    mFpsTextBox->setValue("");

    if(fps>120) mFpsTextBox->setValue("no limit");
    else if(fps==0) mFpsTextBox->setValue("no limit");
    else mFpsTextBox->setValue(std::to_string(fps)+" fps");

    return true;
}

// Do send-to-the-server work when slider drag completes, or new
// value entered as text.
void MPFloaterTuning::onFpsSliderChanged()
{
    U32 fps = (U32)mFpsSlider->getValueF32();
    gSavedSettings.setU32("MaxFPS",fps);

    if(fps>120) mFpsTextBox->setValue("no limit");
    else if(fps==0) mFpsTextBox->setValue("no limit");
    else mFpsTextBox->setValue(std::to_string(fps)+" fps");
}

void MPFloaterTuning::onClose(bool app_quitting)
{
}
