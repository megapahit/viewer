/**
 * @file lleventnotifier.h
 * @brief Viewer code for managing event notifications
 *
 * $LicenseInfo:firstyear=2004&license=viewerlgpl$
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

#ifndef LL_LLEVENTNOTIFIER_H
#define LL_LLEVENTNOTIFIER_H

#include <utility>
#include "llframetimer.h"
#include "v3dmath.h"

class LLEventNotification;
class LLMessageSystem;

typedef struct event_st{
    U32 eventId = 0;
    F64 eventEpoch = 0.0;
    std::string eventDateStr;
    std::string eventName;
    std::string creator;
    std::string category;
    std::string desc;
    U32 duration = 0;
    U32 cover = 0;
    U32 amount = 0;
    std::string simName;
    LLVector3d globalPos;
    U32 flags = 0;
    event_st(U32 id, F64 epoch, std::string date_str, std::string name)
        : eventId(id), eventEpoch(epoch), eventDateStr(std::move(date_str)), eventName(std::move(name)){}
    event_st() = default;
} LLEventStruct;

class LLEventNotifier
{
public:
    LLEventNotifier();
    virtual ~LLEventNotifier();

    void update();  // Notify the user of the event if it's coming up
    bool add(const LLEventStruct& event);
    bool add(U32 eventId, F64 eventEpoch, const std::string& eventDateStr, const std::string &eventName);
    void add(U32 eventId);


    void load(const LLSD& event_options);   // In the format that it comes in from login
    void remove(U32 event_id);

    bool hasNotification(const U32 event_id);
    void serverPushRequest(U32 event_id, bool add);

    typedef std::map<U32, LLEventNotification *> en_map;
    bool  handleResponse(U32 eventId, const LLSD& notification, const LLSD& response);

    static void processEventInfoReply(LLMessageSystem *msg, void **);

    typedef boost::signals2::signal<bool(LLEventStruct event)> new_event_signal_t;
    new_event_signal_t mNewEventSignal;
    boost::signals2::connection setNewEventCallback(const new_event_signal_t::slot_type& cb)
    {
        return mNewEventSignal.connect(cb);
    };

protected:
    en_map  mEventNotifications;
    LLFrameTimer    mNotificationTimer;
};


class LLEventNotification
{
public:
    LLEventNotification(U32 eventId, F64 eventEpoch, std::string eventDateStr, std::string eventName);


    U32                 getEventID() const              { return mEventID; }
    const std::string   &getEventName() const           { return mEventName; }
    bool                isValid() const                 { return mEventID > 0 && mEventDateEpoch != 0 && mEventName.size() > 0; }
    const F64           &getEventDateEpoch() const      { return mEventDateEpoch; }
    const std::string   &getEventDateStr() const        { return mEventDateStr; }


protected:
    U32         mEventID;           // EventID for this event
    std::string mEventName;
    F64         mEventDateEpoch;
    std::string mEventDateStr;
};

extern LLEventNotifier gEventNotifier;

#endif //LL_LLEVENTNOTIFIER_H
