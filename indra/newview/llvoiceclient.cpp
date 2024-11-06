 /**
 * @file llvoiceclient.cpp
 * @brief Voice client delegation class implementation.
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

#include "llvoiceclient.h"
#include "llvoicevivox.h"
#if !__FreeBSD__
#include "llvoicewebrtc.h"
#endif
#include "llviewernetwork.h"
#include "llviewercontrol.h"
#include "llcommandhandler.h"
#include "lldir.h"
#include "llhttpnode.h"
#include "llnotificationsutil.h"
#include "llsdserialize.h"
#include "llui.h"
#include "llkeyboard.h"
#include "llagent.h"
#include "lltrans.h"
#include "lluiusage.h"

const F32 LLVoiceClient::OVERDRIVEN_POWER_LEVEL = 0.7f;

const F32 LLVoiceClient::VOLUME_MIN = 0.f;
const F32 LLVoiceClient::VOLUME_DEFAULT = 0.5f;
const F32 LLVoiceClient::VOLUME_MAX = 1.0f;


// Support for secondlife:///app/voice SLapps
class LLVoiceHandler : public LLCommandHandler
{
public:
    // requests will be throttled from a non-trusted browser
    LLVoiceHandler() : LLCommandHandler("voice", UNTRUSTED_THROTTLE) {}

    bool handle(const LLSD& params, const LLSD& query_map, const std::string& grid, LLMediaCtrl* web)
    {
        if (params[0].asString() == "effects")
        {
            LLVoiceEffectInterface* effect_interface = LLVoiceClient::instance().getVoiceEffectInterface();
            // If the voice client doesn't support voice effects, we can't handle effects SLapps
            if (!effect_interface)
            {
                return false;
            }

            // Support secondlife:///app/voice/effects/refresh to update the voice effect list with new effects
            if (params[1].asString() == "refresh")
            {
                effect_interface->refreshVoiceEffectLists(false);
                return true;
            }
        }
        return false;
    }
};
LLVoiceHandler gVoiceHandler;



std::string LLVoiceClientStatusObserver::status2string(LLVoiceClientStatusObserver::EStatusType inStatus)
{
    std::string result = "UNTRANSLATED";

    // Prevent copy-paste errors when updating this list...
#define CASE(x)  case x:  result = #x;  break

    switch(inStatus)
    {
            CASE(STATUS_LOGIN_RETRY);
            CASE(STATUS_LOGGED_IN);
            CASE(STATUS_JOINING);
            CASE(STATUS_JOINED);
            CASE(STATUS_LEFT_CHANNEL);
            CASE(STATUS_VOICE_DISABLED);
            CASE(STATUS_VOICE_ENABLED);
            CASE(BEGIN_ERROR_STATUS);
            CASE(ERROR_CHANNEL_FULL);
            CASE(ERROR_CHANNEL_LOCKED);
            CASE(ERROR_NOT_AVAILABLE);
            CASE(ERROR_UNKNOWN);
        default:
            {
                std::ostringstream stream;
                stream << "UNKNOWN(" << (int)inStatus << ")";
                result = stream.str();
            }
            break;
    }

#undef CASE

    return result;
}

LLVoiceModuleInterface *getVoiceModule(const std::string &voice_server_type)
{
    if (voice_server_type == VIVOX_VOICE_SERVER_TYPE || voice_server_type.empty())
    {
        return (LLVoiceModuleInterface *) LLVivoxVoiceClient::getInstance();
    }
#if !__FreeBSD__
    else if (voice_server_type == WEBRTC_VOICE_SERVER_TYPE)
    {
        return (LLVoiceModuleInterface *) LLWebRTCVoiceClient::getInstance();
    }
#endif
    else
    {
        LLNotificationsUtil::add("VoiceVersionMismatch");
        return nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

LLVoiceClient::LLVoiceClient(LLPumpIO *pump)
    :
    mSpatialVoiceModule(NULL),
    mNonSpatialVoiceModule(NULL),
    m_servicePump(NULL),
    mVoiceEffectEnabled(LLCachedControl<bool>(gSavedSettings, "VoiceMorphingEnabled", true)),
    mVoiceEffectDefault(LLCachedControl<std::string>(gSavedPerAccountSettings, "VoiceEffectDefault", "00000000-0000-0000-0000-000000000000")),
    mVoiceEffectSupportNotified(false),
    mPTTDirty(true),
    mPTT(true),
    mUsePTT(true),
    mPTTMouseButton(0),
    mPTTKey(0),
    mPTTIsToggle(false),
    mUserPTTState(false),
    mMuteMic(false),
    mDisableMic(false)
{
    init(pump);
}

//---------------------------------------------------
// Basic setup/shutdown

LLVoiceClient::~LLVoiceClient()
{
}

void LLVoiceClient::init(LLPumpIO *pump)
{
    // Initialize all of the voice modules
    m_servicePump = pump;
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->init(pump);
#endif
    LLVivoxVoiceClient::getInstance()->init(pump);
}

void LLVoiceClient::userAuthorized(const std::string& user_id, const LLUUID &agentID)
{
    if (mRegionChangedCallbackSlot.connected())
    {
        mRegionChangedCallbackSlot.disconnect();
    }
    mRegionChangedCallbackSlot = gAgent.addRegionChangedCallback(boost::bind(&LLVoiceClient::onRegionChanged, this));
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->userAuthorized(user_id, agentID);
#endif
    LLVivoxVoiceClient::getInstance()->userAuthorized(user_id, agentID);
}

void LLVoiceClient::handleSimulatorFeaturesReceived(const LLSD &simulatorFeatures)
{
    std::string voiceServerType = simulatorFeatures["VoiceServerType"].asString();
    if (voiceServerType.empty())
    {
        voiceServerType = VIVOX_VOICE_SERVER_TYPE;
    }

    if (mSpatialVoiceModule && !mNonSpatialVoiceModule)
    {
        // stop processing if we're going to change voice modules
        // and we're not currently in non-spatial.
        LLVoiceVersionInfo version = mSpatialVoiceModule->getVersion();
        if (version.internalVoiceServerType != voiceServerType)
        {
            mSpatialVoiceModule->processChannels(false);
        }
    }
    setSpatialVoiceModule(simulatorFeatures["VoiceServerType"].asString());

    // if we should be in spatial voice, switch to it and set the creds
    if (mSpatialVoiceModule && !mNonSpatialVoiceModule)
    {
        if (!mSpatialCredentials.isUndefined())
        {
            mSpatialVoiceModule->setSpatialChannel(mSpatialCredentials);
        }
        mSpatialVoiceModule->processChannels(true);
    }
}

static void simulator_features_received_callback(const LLUUID& region_id)
{
    LLViewerRegion *region = gAgent.getRegion();
    if (region && (region->getRegionID() == region_id))
    {
        LLSD simulatorFeatures;
        region->getSimulatorFeatures(simulatorFeatures);
        if (LLVoiceClient::getInstance())
        {
            LLVoiceClient::getInstance()->handleSimulatorFeaturesReceived(simulatorFeatures);
        }
    }
}

void LLVoiceClient::onRegionChanged()
{
    LLViewerRegion *region = gAgent.getRegion();
    if (region && region->simulatorFeaturesReceived())
    {
        LLSD simulatorFeatures;
        region->getSimulatorFeatures(simulatorFeatures);
        if (LLVoiceClient::getInstance())
        {
            LLVoiceClient::getInstance()->handleSimulatorFeaturesReceived(simulatorFeatures);
        }
    }
    else if (region)
    {
        if (mSimulatorFeaturesReceivedSlot.connected())
        {
            mSimulatorFeaturesReceivedSlot.disconnect();
        }
        mSimulatorFeaturesReceivedSlot =
                region->setSimulatorFeaturesReceivedCallback(boost::bind(&simulator_features_received_callback, _1));
    }
}

void LLVoiceClient::setSpatialVoiceModule(const std::string &voice_server_type)
{
    LLVoiceModuleInterface *module = getVoiceModule(voice_server_type);
    if (!module)
    {
        return;
    }
    if (module != mSpatialVoiceModule)
    {
        if (inProximalChannel())
        {
            mSpatialVoiceModule->processChannels(false);
            module->processChannels(true);
        }
        mSpatialVoiceModule = module;
        mSpatialVoiceModule->updateSettings();
    }
}

void LLVoiceClient::setNonSpatialVoiceModule(const std::string &voice_server_type)
{
    mNonSpatialVoiceModule = getVoiceModule(voice_server_type);
    if (!mNonSpatialVoiceModule)
    {
        // we don't have a non-spatial voice module,
        // so revert to spatial.
        if (mSpatialVoiceModule)
        {
            mSpatialVoiceModule->processChannels(true);
        }
        return;
    }
    mNonSpatialVoiceModule->updateSettings();
}

void LLVoiceClient::setHidden(bool hidden)
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->setHidden(hidden);
#endif
    LLVivoxVoiceClient::getInstance()->setHidden(hidden);
}

void LLVoiceClient::terminate()
{
    if (mSpatialVoiceModule) mSpatialVoiceModule->terminate();
    mSpatialVoiceModule = NULL;
    m_servicePump = NULL;

    // Shutdown speaker volume storage before LLSingletonBase::deleteAll() does it
    if (LLSpeakerVolumeStorage::instanceExists())
    {
        LLSpeakerVolumeStorage::deleteSingleton();
    }
}

const LLVoiceVersionInfo LLVoiceClient::getVersion()
{
    if (mSpatialVoiceModule)
    {
        return mSpatialVoiceModule->getVersion();
    }
    else
    {
        LLVoiceVersionInfo result;
        result.serverVersion = std::string();
        result.voiceServerType = std::string();
        result.mBuildVersion = std::string();
        return result;
    }
}

void LLVoiceClient::updateSettings()
{
    setUsePTT(gSavedSettings.getBOOL("PTTCurrentlyEnabled"));
    setPTTIsToggle(gSavedSettings.getBOOL("PushToTalkToggle"));
    mDisableMic = gSavedSettings.getBOOL("VoiceDisableMic");

    updateMicMuteLogic();

#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->updateSettings();
#endif
    LLVivoxVoiceClient::getInstance()->updateSettings();
}

//--------------------------------------------------
// tuning

void LLVoiceClient::tuningStart()
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->tuningStart();
#endif
    LLVivoxVoiceClient::getInstance()->tuningStart();
}

void LLVoiceClient::tuningStop()
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->tuningStop();
#endif
    LLVivoxVoiceClient::getInstance()->tuningStop();
}

bool LLVoiceClient::inTuningMode()
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->inTuningMode();
#else
    return LLVivoxVoiceClient::getInstance()->inTuningMode();
#endif
}

void LLVoiceClient::tuningSetMicVolume(float volume)
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->tuningSetMicVolume(volume);
#else
    LLVivoxVoiceClient::getInstance()->tuningSetMicVolume(volume);
#endif
}

void LLVoiceClient::tuningSetSpeakerVolume(float volume)
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->tuningSetSpeakerVolume(volume);
#else
    LLVivoxVoiceClient::getInstance()->tuningSetSpeakerVolume(volume);
#endif
}

float LLVoiceClient::tuningGetEnergy(void)
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->tuningGetEnergy();
#else
    return LLVivoxVoiceClient::getInstance()->tuningGetEnergy();
#endif
}

//------------------------------------------------
// devices

bool LLVoiceClient::deviceSettingsAvailable()
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->deviceSettingsAvailable();
#else
    return LLVivoxVoiceClient::getInstance()->deviceSettingsAvailable();
#endif
}

bool LLVoiceClient::deviceSettingsUpdated()
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->deviceSettingsUpdated();
#else
    return LLVivoxVoiceClient::getInstance()->deviceSettingsUpdated();
#endif
}

void LLVoiceClient::refreshDeviceLists(bool clearCurrentList)
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->refreshDeviceLists(clearCurrentList);
#else
    LLVivoxVoiceClient::getInstance()->refreshDeviceLists(clearCurrentList);
#endif
}

void LLVoiceClient::setCaptureDevice(const std::string& name)
{
    LLVivoxVoiceClient::getInstance()->setCaptureDevice(name);
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->setCaptureDevice(name);
#endif
}

void LLVoiceClient::setRenderDevice(const std::string& name)
{
    LLVivoxVoiceClient::getInstance()->setRenderDevice(name);
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->setRenderDevice(name);
#endif
}

const LLVoiceDeviceList& LLVoiceClient::getCaptureDevices()
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->getCaptureDevices();
#else
    return LLVivoxVoiceClient::getInstance()->getCaptureDevices();
#endif
}


const LLVoiceDeviceList& LLVoiceClient::getRenderDevices()
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->getRenderDevices();
#else
    return LLVivoxVoiceClient::getInstance()->getRenderDevices();
#endif
}


//--------------------------------------------------
// participants

void LLVoiceClient::getParticipantList(std::set<LLUUID> &participants) const
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->getParticipantList(participants);
#endif
    LLVivoxVoiceClient::getInstance()->getParticipantList(participants);
}

bool LLVoiceClient::isParticipant(const LLUUID &speaker_id) const
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->isParticipant(speaker_id) ||
#else
    return
#endif
           LLVivoxVoiceClient::getInstance()->isParticipant(speaker_id);
}


//--------------------------------------------------
// text chat

bool LLVoiceClient::isSessionTextIMPossible(const LLUUID& id)
{
    // all sessions can do TextIM, as we no longer support PSTN
    return true;
}

bool LLVoiceClient::isSessionCallBackPossible(const LLUUID& id)
{
    // we don't support PSTN calls anymore.  (did we ever?)
    return true;
}

//----------------------------------------------
// channels

bool LLVoiceClient::inProximalChannel()
{
    if (mSpatialVoiceModule)
    {
        return mSpatialVoiceModule->inProximalChannel();
    }
    else
    {
        return false;
    }
}

void LLVoiceClient::setNonSpatialChannel(
    const LLSD& channelInfo,
    bool notify_on_first_join,
    bool hangup_on_last_leave)
{
    setNonSpatialVoiceModule(channelInfo["voice_server_type"].asString());
    if (mSpatialVoiceModule && mSpatialVoiceModule != mNonSpatialVoiceModule)
    {
        mSpatialVoiceModule->processChannels(false);
    }
    if (mNonSpatialVoiceModule)
    {
        mNonSpatialVoiceModule->processChannels(true);
        mNonSpatialVoiceModule->setNonSpatialChannel(channelInfo, notify_on_first_join, hangup_on_last_leave);
    }
}

void LLVoiceClient::setSpatialChannel(const LLSD &channelInfo)
{
    mSpatialCredentials    = channelInfo;
    LLViewerRegion *region = gAgent.getRegion();
    if (region && region->simulatorFeaturesReceived())
    {
        LLSD simulatorFeatures;
        region->getSimulatorFeatures(simulatorFeatures);
        setSpatialVoiceModule(simulatorFeatures["VoiceServerType"].asString());
    }
    else
    {
        return;
    }

    if (mSpatialVoiceModule)
    {
        mSpatialVoiceModule->setSpatialChannel(channelInfo);
    }
}

void LLVoiceClient::leaveNonSpatialChannel()
{
    if (mNonSpatialVoiceModule)
    {
        mNonSpatialVoiceModule->leaveNonSpatialChannel();
        mNonSpatialVoiceModule->processChannels(false);
        mNonSpatialVoiceModule = nullptr;
    }
}

void LLVoiceClient::activateSpatialChannel(bool activate)
{
    if (mSpatialVoiceModule)
    {
        mSpatialVoiceModule->processChannels(activate);
    }
}

bool LLVoiceClient::isCurrentChannel(const LLSD& channelInfo)
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->isCurrentChannel(channelInfo) ||
#else
    return
#endif
           LLVivoxVoiceClient::getInstance()->isCurrentChannel(channelInfo);
}

bool LLVoiceClient::compareChannels(const LLSD &channelInfo1, const LLSD &channelInfo2)
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->compareChannels(channelInfo1, channelInfo2) ||
#else
    return
#endif
           LLVivoxVoiceClient::getInstance()->compareChannels(channelInfo1, channelInfo2);
}

LLVoiceP2PIncomingCallInterfacePtr LLVoiceClient::getIncomingCallInterface(const LLSD& voice_call_info)
{
    LLVoiceModuleInterface *module = getVoiceModule(voice_call_info["voice_server_type"]);
    if (module)
    {
        return module->getIncomingCallInterface(voice_call_info);
    }
    return nullptr;

}

//---------------------------------------
// outgoing calls
LLVoiceP2POutgoingCallInterface *LLVoiceClient::getOutgoingCallInterface(const LLSD& voiceChannelInfo)
{
    std::string voice_server_type = gSavedSettings.getString("VoiceServerType");
    if (voice_server_type.empty())
    {
        // default to the server type associated with the region we're on.
        LLVoiceVersionInfo versionInfo = LLVoiceClient::getInstance()->getVersion();
        voice_server_type = versionInfo.internalVoiceServerType;
    }
    if (voiceChannelInfo.has("voice_server_type") && voiceChannelInfo["voice_server_type"] != voice_server_type)
    {
        // there's a mismatch between what the peer is offering and what our server
        // can handle, so downgrade to vivox
        voice_server_type = VIVOX_VOICE_SERVER_TYPE;
    }
    LLVoiceModuleInterface *module = getVoiceModule(voice_server_type);
    return dynamic_cast<LLVoiceP2POutgoingCallInterface *>(module);
}

//------------------------------------------
// Volume/gain


void LLVoiceClient::setVoiceVolume(F32 volume)
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->setVoiceVolume(volume);
#endif
    LLVivoxVoiceClient::getInstance()->setVoiceVolume(volume);
}

void LLVoiceClient::setMicGain(F32 gain)
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->setMicGain(gain);
#endif
    LLVivoxVoiceClient::getInstance()->setMicGain(gain);
}


//------------------------------------------
// enable/disable voice features

// static
bool LLVoiceClient::onVoiceEffectsNotSupported(const LLSD &notification, const LLSD &response)
{
    S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
    switch (option)
    {
        case 0:  // "Okay"
            gSavedPerAccountSettings.setString("VoiceEffectDefault", LLUUID::null.asString());
            break;

        case 1:  // "Cancel"
            break;

        default:
            llassert(0);
            break;
    }
    return false;
}

bool LLVoiceClient::voiceEnabled()
{
    static LLCachedControl<bool> enable_voice_chat(gSavedSettings, "EnableVoiceChat");
    static LLCachedControl<bool> cmd_line_disable_voice(gSavedSettings, "CmdLineDisableVoice");
    bool enabled = enable_voice_chat && !cmd_line_disable_voice && !gNonInteractive;
    if (enabled && !mVoiceEffectSupportNotified && getVoiceEffectEnabled() && !getVoiceEffectDefault().isNull())
    {
        static const LLSD args = llsd::map(
            "FAQ_URL", LLTrans::getString("no_voice_morphing_faq_url")
        );

        LLNotificationsUtil::add("VoiceEffectsNotSupported", args, LLSD(), &LLVoiceClient::onVoiceEffectsNotSupported);
        mVoiceEffectSupportNotified = true;
    }
    return enabled;
}

void LLVoiceClient::setVoiceEnabled(bool enabled)
{
#if !__FreeBSD__
    if (LLWebRTCVoiceClient::instanceExists())
    {
        LLWebRTCVoiceClient::getInstance()->setVoiceEnabled(enabled);
    }
#endif
    if (LLVivoxVoiceClient::instanceExists())
    {
        LLVivoxVoiceClient::getInstance()->setVoiceEnabled(enabled);
    }
}

void LLVoiceClient::updateMicMuteLogic()
{
    // If not configured to use PTT, the mic should be open (otherwise the user will be unable to speak).
    bool new_mic_mute = false;

    if(mUsePTT)
    {
        // If configured to use PTT, track the user state.
        new_mic_mute = !mUserPTTState;
    }

    if(mMuteMic || mDisableMic)
    {
        // Either of these always overrides any other PTT setting.
        new_mic_mute = true;
    }
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->setMuteMic(new_mic_mute);
#endif
    LLVivoxVoiceClient::getInstance()->setMuteMic(new_mic_mute);
}

void LLVoiceClient::setMuteMic(bool muted)
{
    if (mMuteMic != muted)
    {
        mMuteMic = muted;
        updateMicMuteLogic();
        mMicroChangedSignal();
    }
}


// ----------------------------------------------
// PTT

void LLVoiceClient::setUserPTTState(bool ptt)
{
    if (ptt)
    {
        LLUIUsage::instance().logCommand("Agent.EnableMicrophone");
    }
    mUserPTTState = ptt;
    updateMicMuteLogic();
    mMicroChangedSignal();
}

bool LLVoiceClient::getUserPTTState()
{
    return mUserPTTState;
}

void LLVoiceClient::setUsePTT(bool usePTT)
{
    if(usePTT && !mUsePTT)
    {
        // When the user turns on PTT, reset the current state.
        mUserPTTState = false;
    }
    mUsePTT = usePTT;

    updateMicMuteLogic();
}

void LLVoiceClient::setPTTIsToggle(bool PTTIsToggle)
{
    if(!PTTIsToggle && mPTTIsToggle)
    {
        // When the user turns off toggle, reset the current state.
        mUserPTTState = false;
    }

    mPTTIsToggle = PTTIsToggle;

    updateMicMuteLogic();
}

bool LLVoiceClient::getPTTIsToggle()
{
    return mPTTIsToggle;
}

void LLVoiceClient::inputUserControlState(bool down)
{
    if(mPTTIsToggle)
    {
        if(down) // toggle open-mic state on 'down'
        {
            toggleUserPTTState();
        }
    }
    else // set open-mic state as an absolute
    {
        setUserPTTState(down);
    }
}

void LLVoiceClient::toggleUserPTTState(void)
{
    setUserPTTState(!getUserPTTState());
}


//-------------------------------------------
// nearby speaker accessors

bool LLVoiceClient::getVoiceEnabled(const LLUUID& id) const
{
    return isParticipant(id);
}

std::string LLVoiceClient::getDisplayName(const LLUUID& id) const
{
#if !__FreeBSD__
    std::string result = LLWebRTCVoiceClient::getInstance()->getDisplayName(id);
    if (result.empty())
    {
        result = LLVivoxVoiceClient::getInstance()->getDisplayName(id);
    }
    return result;
#else
    return LLVivoxVoiceClient::getInstance()->getDisplayName(id);
#endif
}

bool LLVoiceClient::isVoiceWorking() const
{
#if !__FreeBSD__
    return LLVivoxVoiceClient::getInstance()->isVoiceWorking() ||
           LLWebRTCVoiceClient::getInstance()->isVoiceWorking();
#else
    return LLVivoxVoiceClient::getInstance()->isVoiceWorking();
#endif
}

bool LLVoiceClient::isParticipantAvatar(const LLUUID& id)
{
    return true;
}

bool LLVoiceClient::isOnlineSIP(const LLUUID& id)
{
    return false;
}

bool LLVoiceClient::getIsSpeaking(const LLUUID& id)
{
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->getIsSpeaking(id) ||
#else
    return
#endif
           LLVivoxVoiceClient::getInstance()->getIsSpeaking(id);
}

bool LLVoiceClient::getIsModeratorMuted(const LLUUID& id)
{
    // don't bother worrying about p2p calls, as
    // p2p calls don't have mute.
#if !__FreeBSD__
    return LLWebRTCVoiceClient::getInstance()->getIsModeratorMuted(id) ||
#else
    return
#endif
           LLVivoxVoiceClient::getInstance()->getIsModeratorMuted(id);
}

F32 LLVoiceClient::getCurrentPower(const LLUUID& id)
{
#if !__FreeBSD__
    return std::fmax(LLVivoxVoiceClient::getInstance()->getCurrentPower(id),
                     LLWebRTCVoiceClient::getInstance()->getCurrentPower(id));
#else
    return LLVivoxVoiceClient::getInstance()->getCurrentPower(id);
#endif
}

bool LLVoiceClient::getOnMuteList(const LLUUID& id)
{
    // don't bother worrying about p2p calls, as
    // p2p calls don't have mute.
    return LLMuteList::getInstance()->isMuted(id, LLMute::flagVoiceChat);
}

F32 LLVoiceClient::getUserVolume(const LLUUID& id)
{
#if !__FreeBSD__
    return std::fmax(LLVivoxVoiceClient::getInstance()->getUserVolume(id), LLWebRTCVoiceClient::getInstance()->getUserVolume(id));
#else
    return LLVivoxVoiceClient::getInstance()->getUserVolume(id);
#endif
}

void LLVoiceClient::setUserVolume(const LLUUID& id, F32 volume)
{
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->setUserVolume(id, volume);
#endif
    LLVivoxVoiceClient::getInstance()->setUserVolume(id, volume);
}

//--------------------------------------------------
// status observers

void LLVoiceClient::addObserver(LLVoiceClientStatusObserver* observer)
{
    LLVivoxVoiceClient::getInstance()->addObserver(observer);
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->addObserver(observer);
#endif
}

void LLVoiceClient::removeObserver(LLVoiceClientStatusObserver* observer)
{
    if (LLVivoxVoiceClient::instanceExists())
    {
        LLVivoxVoiceClient::getInstance()->removeObserver(observer);
    }
#if !__FreeBSD__
    if (LLWebRTCVoiceClient::instanceExists())
    {
        LLWebRTCVoiceClient::getInstance()->removeObserver(observer);
    }
#endif
}

void LLVoiceClient::addObserver(LLFriendObserver* observer)
{
    LLVivoxVoiceClient::getInstance()->addObserver(observer);
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->addObserver(observer);
#endif
}

void LLVoiceClient::removeObserver(LLFriendObserver* observer)
{
    if (LLVivoxVoiceClient::instanceExists())
    {
        LLVivoxVoiceClient::getInstance()->removeObserver(observer);
    }
#if !__FreeBSD__
    if (LLWebRTCVoiceClient::instanceExists())
    {
        LLWebRTCVoiceClient::getInstance()->removeObserver(observer);
    }
#endif
}

void LLVoiceClient::addObserver(LLVoiceClientParticipantObserver* observer)
{
    LLVivoxVoiceClient::getInstance()->addObserver(observer);
#if !__FreeBSD__
    LLWebRTCVoiceClient::getInstance()->addObserver(observer);
#endif
}

void LLVoiceClient::removeObserver(LLVoiceClientParticipantObserver* observer)
{
    if (LLVivoxVoiceClient::instanceExists())
    {
        LLVivoxVoiceClient::getInstance()->removeObserver(observer);
    }
#if !__FreeBSD__
    if (LLWebRTCVoiceClient::instanceExists())
    {
        LLWebRTCVoiceClient::getInstance()->removeObserver(observer);
    }
#endif
}

std::string LLVoiceClient::sipURIFromID(const LLUUID &id) const
{
    if (mNonSpatialVoiceModule)
    {
        return mNonSpatialVoiceModule->sipURIFromID(id);
    }
    else if (mSpatialVoiceModule)
    {
        return mSpatialVoiceModule->sipURIFromID(id);
    }
    else
    {
        return std::string();
    }
}

LLSD LLVoiceClient::getP2PChannelInfoTemplate(const LLUUID& id) const
{
    if (mNonSpatialVoiceModule)
    {
        return mNonSpatialVoiceModule->getP2PChannelInfoTemplate(id);
    }
    else if (mSpatialVoiceModule)
    {
        return mSpatialVoiceModule->getP2PChannelInfoTemplate(id);
    }
    else
    {
        return LLSD();
    }
}

LLVoiceEffectInterface* LLVoiceClient::getVoiceEffectInterface() const
{
    return NULL;
}

///////////////////
// version checking

class LLViewerRequiredVoiceVersion : public LLHTTPNode
{
    static bool sAlertedUser;
    virtual void post(
                      LLHTTPNode::ResponsePtr response,
                      const LLSD& context,
                      const LLSD& input) const
    {
        std::string voice_server_type = "vivox";
        if (input.has("body") && input["body"].has("voice_server_type"))
        {
            voice_server_type = input["body"]["voice_server_type"].asString();
        }

        LLVoiceModuleInterface *voiceModule = NULL;

        if (voice_server_type == "vivox" || voice_server_type.empty())
        {
            voiceModule = (LLVoiceModuleInterface *) LLVivoxVoiceClient::getInstance();
        }
#if !__FreeBSD__
        else if (voice_server_type == "webrtc")
        {
            voiceModule = (LLVoiceModuleInterface *) LLWebRTCVoiceClient::getInstance();
        }
#endif
        else
        {
            LL_WARNS("Voice") << "Unknown voice server type " << voice_server_type << LL_ENDL;
            if (!sAlertedUser)
            {
                // sAlertedUser = true;
                LLNotificationsUtil::add("VoiceVersionMismatch");
            }
            return;
        }

        LLVoiceVersionInfo versionInfo = voiceModule->getVersion();
        if (input.has("body") && input["body"].has("major_version") &&
            input["body"]["major_version"].asInteger() > versionInfo.majorVersion)
        {
            if (!sAlertedUser)
            {
                // sAlertedUser = true;
                LLNotificationsUtil::add("VoiceVersionMismatch");
                LL_WARNS("Voice") << "Voice server version mismatch " << input["body"]["major_version"].asInteger() << "/"
                                  << versionInfo.majorVersion
                                  << LL_ENDL;
            }
            return;
        }
    }
};

class LLViewerParcelVoiceInfo : public LLHTTPNode
{
    virtual void post(
                      LLHTTPNode::ResponsePtr response,
                      const LLSD& context,
                      const LLSD& input) const
    {
        //the parcel you are in has changed something about its
        //voice information

        //this is a misnomer, as it can also be when you are not in
        //a parcel at all.  Should really be something like
        //LLViewerVoiceInfoChanged.....
        if ( input.has("body") )
        {
            LLSD body = input["body"];

            //body has "region_name" (str), "parcel_local_id"(int),
            //"voice_credentials" (map).

            //body["voice_credentials"] has "channel_uri" (str),
            //body["voice_credentials"] has "channel_credentials" (str)

            //if we really wanted to be extra careful,
            //we'd check the supplied
            //local parcel id to make sure it's for the same parcel
            //we believe we're in
            if ( body.has("voice_credentials") )
            {
                LLVoiceClient::getInstance()->setSpatialChannel(body["voice_credentials"]);
            }
        }
    }
};

const std::string LLSpeakerVolumeStorage::SETTINGS_FILE_NAME = "volume_settings.xml";

LLSpeakerVolumeStorage::LLSpeakerVolumeStorage()
{
    load();
}

LLSpeakerVolumeStorage::~LLSpeakerVolumeStorage()
{
}

//virtual
void LLSpeakerVolumeStorage::cleanupSingleton()
{
    save();
}

void LLSpeakerVolumeStorage::storeSpeakerVolume(const LLUUID& speaker_id, F32 volume)
{
    if ((volume >= LLVoiceClient::VOLUME_MIN) && (volume <= LLVoiceClient::VOLUME_MAX))
    {
        mSpeakersData[speaker_id] = volume;

        // Enable this when debugging voice slider issues.  It's way to spammy even for debug-level logging.
        // LL_DEBUGS("Voice") << "Stored volume = " << volume <<  " for " << id << LL_ENDL;
    }
    else
    {
        LL_WARNS("Voice") << "Attempted to store out of range volume " << volume << " for " << speaker_id << LL_ENDL;
        llassert(0);
    }
}

bool LLSpeakerVolumeStorage::getSpeakerVolume(const LLUUID& speaker_id, F32& volume)
{
    speaker_data_map_t::const_iterator it = mSpeakersData.find(speaker_id);

    if (it != mSpeakersData.end())
    {
        volume = it->second;

        // Enable this when debugging voice slider issues.  It's way to spammy even for debug-level logging.
        // LL_DEBUGS("Voice") << "Retrieved stored volume = " << volume <<  " for " << id << LL_ENDL;

        return true;
    }

    return false;
}

void LLSpeakerVolumeStorage::removeSpeakerVolume(const LLUUID& speaker_id)
{
    mSpeakersData.erase(speaker_id);

    // Enable this when debugging voice slider issues.  It's way to spammy even for debug-level logging.
    // LL_DEBUGS("Voice") << "Removing stored volume for  " << id << LL_ENDL;
}

/* static */ F32 LLSpeakerVolumeStorage::transformFromLegacyVolume(F32 volume_in)
{
    // Convert to linear-logarithmic [0.0..1.0] with 0.5 = 0dB
    // from legacy characteristic composed of two square-curves
    // that intersect at volume_in = 0.5, volume_out = 0.56

    F32 volume_out = 0.f;
    volume_in = llclamp(volume_in, 0.f, 1.0f);

    if (volume_in <= 0.5f)
    {
        volume_out = volume_in * volume_in * 4.f * 0.56f;
    }
    else
    {
        volume_out = (1.f - 0.56f) * (4.f * volume_in * volume_in - 1.f) / 3.f + 0.56f;
    }

    return volume_out;
}

/* static */ F32 LLSpeakerVolumeStorage::transformToLegacyVolume(F32 volume_in)
{
    // Convert from linear-logarithmic [0.0..1.0] with 0.5 = 0dB
    // to legacy characteristic composed of two square-curves
    // that intersect at volume_in = 0.56, volume_out = 0.5

    F32 volume_out = 0.f;
    volume_in = llclamp(volume_in, 0.f, 1.0f);

    if (volume_in <= 0.56f)
    {
        volume_out = sqrt(volume_in / (4.f * 0.56f));
    }
    else
    {
        volume_out = sqrt((3.f * (volume_in - 0.56f) / (1.f - 0.56f) + 1.f) / 4.f);
    }

    return volume_out;
}

void LLSpeakerVolumeStorage::load()
{
    // load per-resident voice volume information
    std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, SETTINGS_FILE_NAME);

    LL_INFOS("Voice") << "Loading stored speaker volumes from: " << filename << LL_ENDL;

    LLSD settings_llsd;
    llifstream file;
    file.open(filename.c_str());
    if (file.is_open())
    {
        if (LLSDParser::PARSE_FAILURE == LLSDSerialize::fromXML(settings_llsd, file))
        {
            LL_WARNS("Voice") << "failed to parse " << filename << LL_ENDL;

        }

    }

    for (LLSD::map_const_iterator iter = settings_llsd.beginMap();
        iter != settings_llsd.endMap(); ++iter)
    {
        // Maintain compatibility with 1.23 non-linear saved volume levels
        F32 volume = transformFromLegacyVolume((F32)iter->second.asReal());

        storeSpeakerVolume(LLUUID(iter->first), volume);
    }
}

void LLSpeakerVolumeStorage::save()
{
    // If we quit from the login screen we will not have an SL account
    // name.  Don't try to save, otherwise we'll dump a file in
    // C:\Program Files\SecondLife\ or similar. JC
    std::string user_dir = gDirUtilp->getLindenUserDir();
    if (!user_dir.empty())
    {
        std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, SETTINGS_FILE_NAME);
        LLSD settings_llsd;

        LL_INFOS("Voice") << "Saving stored speaker volumes to: " << filename << LL_ENDL;

        for(speaker_data_map_t::const_iterator iter = mSpeakersData.begin(); iter != mSpeakersData.end(); ++iter)
        {
            // Maintain compatibility with 1.23 non-linear saved volume levels
            F32 volume = transformToLegacyVolume(iter->second);

            settings_llsd[iter->first.asString()] = volume;
        }

        llofstream file;
        file.open(filename.c_str());
        LLSDSerialize::toPrettyXML(settings_llsd, file);
    }
}

bool LLViewerRequiredVoiceVersion::sAlertedUser = false;

LLHTTPRegistration<LLViewerParcelVoiceInfo>
gHTTPRegistrationMessageParcelVoiceInfo(
                                        "/message/ParcelVoiceInfo");

LLHTTPRegistration<LLViewerRequiredVoiceVersion>
gHTTPRegistrationMessageRequiredVoiceVersion(
                                             "/message/RequiredVoiceVersion");
