/**
 * @file   llquickprefs.h
 * @brief  Quick Preferences floater: hover height and bandwidth sliders.
 *
 * Ported from Firestorm Viewer (quickprefs.h / quickprefs.cpp).
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

#ifndef LL_LLQUICKPREFS_H
#define LL_LLQUICKPREFS_H

#include "llfloater.h"

class LLSliderCtrl;
class LLCheckBoxCtrl;
class LLTextBox;

/**
 * @class LLFloaterQuickPrefs
 *
 * A lightweight "Quick Preferences" panel that lets the user adjust commonly
 * tweaked settings on the fly without opening the full Preferences dialog:
 *   - Avatar hover height
 *   - Maximum network bandwidth
 *
 * Hover-height logic is ported 1:1 from Firestorm's FloaterQuickPrefs so that
 * live-preview while dragging, final commit on mouse-up, and region-feature
 * gating all work exactly the same way.
 */
class LLFloaterQuickPrefs : public LLFloater
{
public:
    LLFloaterQuickPrefs(const LLSD& key);
    virtual ~LLFloaterQuickPrefs();

    bool postBuild() override;
    void onClose(bool app_quitting) override;

private:
    // ---- Hover height -------------------------------------------------------
    LLSliderCtrl* mAvatarZOffsetSlider;

    /** Called every frame while the slider thumb is being dragged.
     *  Sends a live (non-persistent) hover offset to the avatar so the user
     *  gets immediate visual feedback. */
    void onAvatarZOffsetSliderMoved();

    /** Called when the drag ends (mouse-up) or the user types a value.
     *  Persists the value to AvatarHoverOffsetZ. */
    void onAvatarZOffsetFinalCommit();

    /** Enable/disable the slider based on whether the current region supports
     *  server-side hover height. */
    void updateAvatarZOffsetEditEnabled();

    /** Called when the region changes so we can re-evaluate the above. */
    void onRegionChanged();
    void onSimulatorFeaturesReceived(const LLUUID& region_id);

    /** Pulls AvatarHoverOffsetZ from saved settings and pushes it to the slider
     *  without triggering a commit (avoids feedback loops). */
    void syncAvatarZOffsetFromPreferenceSetting();

    boost::signals2::connection mRegionChangedSlot;

    // OTS over-the-shoulder aim
    LLCheckBoxCtrl* mOTSEnabledCheck { nullptr };
    void onOTSEnabledChanged();
};

#endif // LL_LLQUICKPREFS_H
