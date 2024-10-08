/**
* @file mpfloatertuning.h
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
#ifndef LL_MPFLOATERTUNING_H
#define LL_MPFLOATERTUNING_H

#include "llfloater.h"

class MPFloaterTuning: public LLFloater
{
public:
    MPFloaterTuning(const LLSD& key);

    bool postBuild();

    void onFinalCommit();

    static void syncFromPreferenceSetting(void *user_data);

    //void updateEditEnabled();

    /*virtual*/ void onClose(bool app_quitting);
};

#endif
