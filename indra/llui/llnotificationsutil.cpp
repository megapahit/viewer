/**
 * @file llnotificationsutil.cpp
 *
 * $LicenseInfo:firstyear=2008&license=viewerlgpl$
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
#include "linden_common.h"

#include "llnotificationsutil.h"

#include "llnotifications.h"
#include "llsd.h"
#include "llxmlnode.h"  // apparently needed to call LLNotifications::instance()

namespace
{
    // deleteAll() must not resurrect LLNotifications after its dependencies
    // are gone. A null return means the caller should skip the toast.
    LLNotifications* live_notifications()
    {
        if (LLNotifications::wasDeleted() || LLSingletonBase::shuttingDown())
        {
            return nullptr;
        }
        return LLNotifications::getInstance();
    }
}

LLNotificationPtr LLNotificationsUtil::add(const std::string& name)
{
    LLNotifications* notifications = live_notifications();
    if (!notifications)
    {
        return LLNotificationPtr();
    }
    LLNotification::Params::Functor functor_p;
    functor_p.name = name;
    return notifications->add(
        LLNotification::Params().name(name).substitutions(LLSD()).payload(LLSD()).functor(functor_p));
}

LLNotificationPtr LLNotificationsUtil::add(const std::string& name,
                      const LLSD& substitutions)
{
    LLNotifications* notifications = live_notifications();
    if (!notifications)
    {
        return LLNotificationPtr();
    }
    LLNotification::Params::Functor functor_p;
    functor_p.name = name;
    return notifications->add(
        LLNotification::Params().name(name).substitutions(substitutions).payload(LLSD()).functor(functor_p));
}

LLNotificationPtr LLNotificationsUtil::add(const std::string& name,
                      const LLSD& substitutions,
                      const LLSD& payload)
{
    LLNotifications* notifications = live_notifications();
    if (!notifications)
    {
        return LLNotificationPtr();
    }
    LLNotification::Params::Functor functor_p;
    functor_p.name = name;
    return notifications->add(
        LLNotification::Params().name(name).substitutions(substitutions).payload(payload).functor(functor_p));
}

LLNotificationPtr LLNotificationsUtil::add(const std::string& name,
                      const LLSD& substitutions,
                      const LLSD& payload,
                      const std::string& functor_name)
{
    LLNotifications* notifications = live_notifications();
    if (!notifications)
    {
        return LLNotificationPtr();
    }
    LLNotification::Params::Functor functor_p;
    functor_p.name = functor_name;
    return notifications->add(
        LLNotification::Params().name(name).substitutions(substitutions).payload(payload).functor(functor_p));
}

LLNotificationPtr LLNotificationsUtil::add(const std::string& name,
                      const LLSD& substitutions,
                      const LLSD& payload,
                      std::function<void (const LLSD&, const LLSD&)> functor)
{
    LLNotifications* notifications = live_notifications();
    if (!notifications)
    {
        return LLNotificationPtr();
    }
    LLNotification::Params::Functor functor_p;
    functor_p.function = functor;
    return notifications->add(
        LLNotification::Params().name(name).substitutions(substitutions).payload(payload).functor(functor_p));
}

S32 LLNotificationsUtil::getSelectedOption(const LLSD& notification, const LLSD& response)
{
    return LLNotification::getSelectedOption(notification, response);
}

void LLNotificationsUtil::cancel(LLNotificationPtr pNotif)
{
    LLNotifications* notifications = live_notifications();
    if (notifications)
    {
        notifications->cancel(pNotif);
    }
}

LLNotificationPtr LLNotificationsUtil::find(LLUUID uuid)
{
    LLNotifications* notifications = live_notifications();
    if (!notifications)
    {
        return LLNotificationPtr();
    }
    return notifications->find(uuid);
}
