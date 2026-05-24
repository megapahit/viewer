/**
 * @file   llquickprefs.cpp
 * @brief  Quick Preferences floater: hover height and bandwidth sliders.
 *
 * Ported from Firestorm Viewer (quickprefs.cpp).
 * Original authors: WoLf Loonie, Zi Ree, Ansariel Hiller @ Second Life.
 *
 * $LicenseInfo:firstyear=2011&license=viewerlgpl$
 * Phoenix Firestorm Viewer Source Code
 * Copyright (C) 2011, WoLf Loonie @ Second Life
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
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llquickprefs.h"

#include "llagent.h"
#include "llsliderctrl.h"
#include "lltextbox.h"
#include "llviewercontrol.h"
#include "llviewerregion.h"
#include "llvoavatar.h"       // for MIN_HOVER_Z / MAX_HOVER_Z
#include "llvoavatarself.h"   // for gAgentAvatarp, isAgentAvatarValid()

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

LLFloaterQuickPrefs::LLFloaterQuickPrefs(const LLSD& key)
    : LLFloater(key)
    , mAvatarZOffsetSlider(nullptr)
{
}

LLFloaterQuickPrefs::~LLFloaterQuickPrefs()
{
    if (mRegionChangedSlot.connected())
    {
        mRegionChangedSlot.disconnect();
    }
}

// ---------------------------------------------------------------------------
// postBuild – wire up all widgets
// ---------------------------------------------------------------------------

bool LLFloaterQuickPrefs::postBuild()
{
    // ---- Hover height slider ------------------------------------------------
    mAvatarZOffsetSlider = getChild<LLSliderCtrl>("HoverHeightSlider");
    mAvatarZOffsetSlider->setMinValue(MIN_HOVER_Z);
    mAvatarZOffsetSlider->setMaxValue(MAX_HOVER_Z);

    // Live preview while dragging
    mAvatarZOffsetSlider->setCommitCallback(
        boost::bind(&LLFloaterQuickPrefs::onAvatarZOffsetSliderMoved, this));

    // Persist on release or typed entry
    mAvatarZOffsetSlider->setSliderMouseUpCallback(
        boost::bind(&LLFloaterQuickPrefs::onAvatarZOffsetFinalCommit, this));
    mAvatarZOffsetSlider->setSliderEditorCommitCallback(
        boost::bind(&LLFloaterQuickPrefs::onAvatarZOffsetFinalCommit, this));

    // Pull current value from settings
    syncAvatarZOffsetFromPreferenceSetting();

    // Keep slider in sync when something else changes the setting (e.g. RLVa,
    // the Edit Shape floater, or the standalone Hover Height floater).
    if (gSavedPerAccountSettings.getControl("AvatarHoverOffsetZ"))
    {
        gSavedPerAccountSettings.getControl("AvatarHoverOffsetZ")
            ->getCommitSignal()
            ->connect(boost::bind(
                &LLFloaterQuickPrefs::syncAvatarZOffsetFromPreferenceSetting, this));
    }
    else
    {
        LL_WARNS("QuickPrefs") << "Control 'AvatarHoverOffsetZ' not found" << LL_ENDL;
    }

    // Watch for region changes so we can enable/disable the slider
    if (!mRegionChangedSlot.connected())
    {
        mRegionChangedSlot = gAgent.addRegionChangedCallback(
            boost::bind(&LLFloaterQuickPrefs::onRegionChanged, this));
    }
    onRegionChanged();  // evaluate current region immediately

    return true;
}

// ---------------------------------------------------------------------------
// onClose
// ---------------------------------------------------------------------------

void LLFloaterQuickPrefs::onClose(bool app_quitting)
{
    if (mRegionChangedSlot.connected())
    {
        mRegionChangedSlot.disconnect();
    }
    LLFloater::onClose(app_quitting);
}

// ---------------------------------------------------------------------------
// Hover height callbacks
// ---------------------------------------------------------------------------

void LLFloaterQuickPrefs::onAvatarZOffsetSliderMoved()
{
    F32 value = mAvatarZOffsetSlider->getValueF32();
    LLVector3 offset(0.0f, 0.0f, llclamp(value, MIN_HOVER_Z, MAX_HOVER_Z));

    LL_INFOS("Avatar") << "QuickPrefs: setting hover from slider moved " << offset[VZ] << LL_ENDL;

    if (gAgent.getRegion() && gAgent.getRegion()->avatarHoverHeightEnabled())
    {
        if (mAvatarZOffsetSlider->isMouseHeldDown())
        {
            // Live preview: send to avatar but don't persist yet
            if (isAgentAvatarValid())
            {
                gAgentAvatarp->setHoverOffset(offset, false);
            }
        }
        else
        {
            // Committed (e.g. arrow-key step): persist immediately
            gSavedPerAccountSettings.setF32("AvatarHoverOffsetZ", value);
        }
    }
    else if (isAgentAvatarValid())
    {
        gSavedPerAccountSettings.setF32("AvatarHoverOffsetZ", value);
    }
}

void LLFloaterQuickPrefs::onAvatarZOffsetFinalCommit()
{
    F32 value = mAvatarZOffsetSlider->getValueF32();
    LL_INFOS("Avatar") << "QuickPrefs: setting hover from slider final commit " << value << LL_ENDL;
    gSavedPerAccountSettings.setF32("AvatarHoverOffsetZ",
        llclamp(value, MIN_HOVER_Z, MAX_HOVER_Z));
}

// ---------------------------------------------------------------------------
// Enable / disable based on region support
// ---------------------------------------------------------------------------

void LLFloaterQuickPrefs::updateAvatarZOffsetEditEnabled()
{
    bool enabled = gAgent.getRegion() && gAgent.getRegion()->avatarHoverHeightEnabled();

    if (!enabled && isAgentAvatarValid())
    {
        enabled = true;
    }

    mAvatarZOffsetSlider->setEnabled(enabled);

    if (enabled)
    {
        syncAvatarZOffsetFromPreferenceSetting();
    }
}

void LLFloaterQuickPrefs::onRegionChanged()
{
    LLViewerRegion* region = gAgent.getRegion();
    if (region && region->simulatorFeaturesReceived())
    {
        updateAvatarZOffsetEditEnabled();
    }
    else if (region)
    {
        region->setSimulatorFeaturesReceivedCallback(
            boost::bind(&LLFloaterQuickPrefs::onSimulatorFeaturesReceived, this, _1));
    }
}

void LLFloaterQuickPrefs::onSimulatorFeaturesReceived(const LLUUID& region_id)
{
    LLViewerRegion* region = gAgent.getRegion();
    if (region && region->getRegionID() == region_id)
    {
        updateAvatarZOffsetEditEnabled();
    }
}

void LLFloaterQuickPrefs::syncAvatarZOffsetFromPreferenceSetting()
{
    F32 value = gSavedPerAccountSettings.getF32("AvatarHoverOffsetZ");
    mAvatarZOffsetSlider->setValue(value, false);   // false = no commit signal
}
