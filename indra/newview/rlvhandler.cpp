/**
 * @file rlvhandler.cpp
 * @author Kitty Barnett
 * @brief RLVa helper classes for internal use only
 *
 * $LicenseInfo:firstyear=2024&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2024, Linden Research, Inc.
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
#include "llagent.h"
#include "llstartup.h"
#include "llappearancemgr.h"
#include "llinventoryfunctions.h"
#include "llinventorymodel.h"
#include "llmoveview.h"
#include "llviewercontrol.h"
#include "llviewermenu.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"

#include "rlvcommon.h"
#include "rlvhandler.h"
#include "rlvhelper.h"

#include <boost/algorithm/string.hpp>

using namespace Rlv;

// ============================================================================
// Static variable initialization
//

bool RlvHandler::mIsEnabled = false;

// ============================================================================
// Command processing functions
//

bool RlvHandler::handleSimulatorChat(std::string& message, const LLChat& chat, const LLViewerObject* chatObj)
{
    // *TODO: There's an edge case for temporary attachments when going from enabled -> disabled with restrictions already in place
    static LLCachedControl<bool> enable_temp_attach(gSavedSettings, Settings::EnableTempAttach);
    static LLCachedControl<bool> show_debug_output(gSavedSettings, Settings::Debug);
    static LLCachedControl<bool> hide_unset_dupes(gSavedSettings, Settings::DebugHideUnsetDup);

    if ( message.length() <= 3 || Constants::CmdPrefix != message[0] || CHAT_TYPE_OWNER != chat.mChatType ||
         (chatObj && chatObj->isTempAttachment() && !enable_temp_attach()) )
    {
        return false;
    }

    message.erase(0, 1);
    LLStringUtil::toLower(message);
    CommandDbgOut cmdDbgOut(message, chatObj->getID() == gAgentID);

    boost_tokenizer tokens(message, boost::char_separator<char>(",", "", boost::drop_empty_tokens));
    for (const std::string& strCmd : tokens)
    {
        ECmdRet eRet = processCommand(chat.mFromID, strCmd, true);
        if ( show_debug_output() &&
             (!hide_unset_dupes() || (ECmdRet::SuccessUnset != eRet && ECmdRet::SuccessDuplicate != eRet)) )
        {
            cmdDbgOut.add(strCmd, eRet);
        }
    }

    message = cmdDbgOut.get();
    return true;
}

ECmdRet RlvHandler::processCommand(const LLUUID& idObj, const std::string& strCmd, bool fromObj)
{
    const RlvCommand rlvCmd(idObj, strCmd);
    return processCommand(std::ref(rlvCmd), fromObj);
}

ECmdRet RlvHandler::processCommand(std::reference_wrapper<const RlvCommand> rlvCmd, bool fromObj)
{
    {
        const RlvCommand& rlvCmdTmp = rlvCmd; // Reference to the temporary with limited variable scope since we don't want it to leak below

        RLV_DEBUGS << "[" << rlvCmdTmp.getObjectID() << "]: " << rlvCmdTmp.asString() << RLV_ENDL;
        if (!rlvCmdTmp.isValid())
        {
            RLV_DEBUGS << "\t-> invalid syntax" << RLV_ENDL;
            return ECmdRet::FailedSyntax;
        }
        if (rlvCmdTmp.isBlocked())
        {
            RLV_DEBUGS << "\t-> blocked command" << RLV_ENDL;
            return ECmdRet::FailedDisabled;
        }
    }

    ECmdRet eRet = ECmdRet::Unknown;
    switch (rlvCmd.get().getParamType())
    {
        case EParamType::Reply:
        case EParamType::Force:
        case EParamType::Remove:
        case EParamType::Add:
            eRet = rlvCmd.get().processCommand();
            break;
        case EParamType::Unknown:
        default:
            eRet = ECmdRet::FailedParam;
            break;
    }
    RLV_ASSERT(ECmdRet::Unknown != eRet);

    RLV_DEBUGS << "\t--> command " << (isReturnCodeSuccess(eRet) ? "succeeded" : "failed") << RLV_ENDL;

    return eRet;
}

// ============================================================================
// Initialization helper functions
//

bool RlvHandler::canEnable()
{
    return LLStartUp::getStartupState() <= STATE_LOGIN_CLEANUP;
}

bool RlvHandler::setEnabled(bool enable)
{
    if (mIsEnabled == enable)
        return enable;

    if (enable && canEnable())
    {
        RLV_INFOS << "Enabling Restrained Love API support - " << Strings::getVersionAbout() << RLV_ENDL;
        mIsEnabled = true;
    }

    return mIsEnabled;
}

// ============================================================================
// Command handlers (RLV_TYPE_REPLY)
//

ECmdRet CommandHandlerBaseImpl<EParamType::Reply>::processCommand(const RlvCommand& rlvCmd, ReplyHandlerFunc* pHandler)
{
    // Sanity check - <param> should specify a - valid - reply channel
    S32 nChannel;
    if (!LLStringUtil::convertToS32(rlvCmd.getParam(), nChannel) || !Util::isValidReplyChannel(nChannel, rlvCmd.getObjectID() == gAgentID))
        return ECmdRet::FailedParam;

    std::string strReply;
    ECmdRet eRet = (*pHandler)(rlvCmd, strReply);

    // If we made it this far then:
    //   - the command was handled successfully so we send off the response
    //   - the command failed but we still send off an - empty - response to keep the issuing script from blocking
    if (nChannel != 0)
    {
        Util::sendChatReply(nChannel, strReply);
    }
    RlvHandler::instance().mOnCommandOutput(rlvCmd, nChannel, strReply);

    return eRet;
}

// Handles: @getcommand[:<behaviour>[;<type>[;<separator>]]]=<channel>
template<> template<>
ECmdRet ReplyHandler<EBehaviour::GetCommand>::onCommand(const RlvCommand& rlvCmd, std::string& strReply)
{
    std::vector<std::string> optionList;
    Util::parseStringList(rlvCmd.getOption(), optionList);

    // If a second parameter is present it'll specify the command type
    EParamType eType = EParamType::Unknown;
    if (optionList.size() >= 2)
    {
        if (optionList[1] == "any" || optionList[1].empty())
            eType = EParamType::Unknown;
        else if (optionList[1] == "add")
            eType = EParamType::AddRem;
        else if (optionList[1] == "force")
            eType = EParamType::Force;
        else if (optionList[1] == "reply")
            eType = EParamType::Reply;
        else
            return ECmdRet::FailedOption;
    }

    std::list<std::string> cmdList;
    if (BehaviourDictionary::instance().getCommands(!optionList.empty() ? optionList[0] : LLStringUtil::null, eType, cmdList))
        strReply = boost::algorithm::join(cmdList, optionList.size() >= 3 ? optionList[2] : Constants::OptionSeparator);
    return ECmdRet::Succeeded;
}

// Handles: @version=<chnannel> and @versionnew=<channel>
template<> template<>
ECmdRet VersionReplyHandler::onCommand(const RlvCommand& rlvCmd, std::string& strReply)
{
    strReply = Strings::getVersion(EBehaviour::Version == rlvCmd.getBehaviourType());
    return ECmdRet::Succeeded;
}

// Handles: @versionnum[:impl]=<channel>
template<> template<>
ECmdRet ReplyHandler<EBehaviour::VersionNum>::onCommand(const RlvCommand& rlvCmd, std::string& strReply)
{
    if (!rlvCmd.hasOption())
        strReply = Strings::getVersionNum();
    else if ("impl" == rlvCmd.getOption())
        strReply = Strings::getVersionImplNum();
    else
        return ECmdRet::FailedOption;
    return ECmdRet::Succeeded;
}

template<> template<>
ECmdRet ReplyHandler<EBehaviour::GetSitID>::onCommand(const RlvCommand& rlvCmd, std::string& strReply)
{
    if (gAgent.isSitting())
        gAgent.getSitObjectID().toString(strReply);
    else
        strReply = "00000000-0000-0000-0000-000000000000";
    return ECmdRet::Succeeded;
}

template<> template<>
ECmdRet ReplyHandler<EBehaviour::GetInv>::onCommand(const RlvCommand& rlvCmd, std::string& strReply)
{
    auto folderID = gInventory.getRootFolderID();
    LLNameCategoryCollector has_name("#RLV");
    if (!gInventory.hasMatchingDirectDescendent(folderID, has_name))
        return ECmdRet::FailedNoSharedRoot;
    folderID = findDescendentCategoryIDByName(folderID, "#RLV");
    strReply = "";
    LLInventoryModel::cat_array_t* cats;
    LLInventoryModel::item_array_t* items;
    std::vector<std::string> optionList;
    auto option = rlvCmd.getOption();
    if (!option.empty())
    {
        Util::parseStringList(option, optionList, "/");
        auto optIter = optionList.begin();
        for(; optionList.end() != optIter; ++optIter)
        {
            auto name = *optIter;
            if (!name.empty())
                folderID = findDescendentCategoryIDByName(folderID, name);
        }
    }
    gInventory.getDirectDescendentsOf(folderID, cats, items);
    auto iter = cats->begin();
    for(; cats->end() != iter; ++iter)
    {
        auto name = (*iter)->getName();
        if (name.front() == '.')
            continue;
        if (iter != cats->begin())
            strReply.append(",");
        strReply.append(name);
    }
    return ECmdRet::Succeeded;
}

// Force

ECmdRet CommandHandlerBaseImpl<EParamType::Force>::processCommand(const RlvCommand& rlvCmd, ForceHandlerFunc* pHandler)
{
    return (*pHandler)(rlvCmd);
}

template<> template<>
ECmdRet ForceHandler<EBehaviour::Sit>::onCommand(const RlvCommand& rlvCmd)
{
    handle_object_sit(LLUUID{rlvCmd.getOption()});
    return ECmdRet::Succeeded;
}

template<> template<>
ECmdRet ForceHandler<EBehaviour::SitGround>::onCommand(const RlvCommand& rlvCmd)
{
    gAgent.sitDown();
    return ECmdRet::Succeeded;
}

template<> template<>
ECmdRet ForceHandler<EBehaviour::Unsit>::onCommand(const RlvCommand& rlvCmd)
{
    gAgent.standUp();
    return ECmdRet::Succeeded;
}

template<> template<>
ECmdRet ForceHandler<EBehaviour::RemOutfit>::onCommand(const RlvCommand& rlvCmd)
{
    std::vector<std::string> optionList;
    auto option = rlvCmd.getOption();
    if (option.empty())
    {
        LLAppearanceMgr::instance().removeAllClothesFromAvatar();
    }
    else
    {
        LLWearableType::EType type = LLWearableType::getInstance()->typeNameToType(option);
        if (type >= LLWearableType::WT_SHAPE
            && type < LLWearableType::WT_COUNT
            && (gAgentWearables.getWearableCount(type) > 0))
        {
            U32 wearable_index = gAgentWearables.getWearableCount(type) - 1;
            LLUUID item_id = gAgentWearables.getWearableItemID(type,wearable_index);
            LLAppearanceMgr::instance().removeItemFromAvatar(item_id);
        }
    }
    return ECmdRet::Succeeded;
}

#define RESTRAINED_LOVE_OUTFIT(A) \
    auto folderID = gInventory.getRootFolderID();\
    LLNameCategoryCollector has_name("#RLV");\
    if (!gInventory.hasMatchingDirectDescendent(folderID, has_name))\
        return ECmdRet::FailedNoSharedRoot;\
    folderID = findDescendentCategoryIDByName(folderID, "#RLV");\
    std::vector<std::string> optionList;\
    auto option = rlvCmd.getOption();\
    if (!option.empty())\
    {\
        folderID = findDescendentCategoryIDByName(folderID, option);\
        if (folderID == LLUUID::null)\
        {\
            Util::parseStringList(option, optionList, "/");\
            auto iter = optionList.begin();\
            for(; optionList.end() != iter; ++iter)\
            {\
                auto name = *iter;\
                if (!name.empty())\
                    folderID = findDescendentCategoryIDByName(folderID, name);\
            }\
        }\
        A\
    }\
    return ECmdRet::Succeeded;

#define RESTRAINED_LOVE_REPLACE \
        LLAppearanceMgr::instance().replaceCurrentOutfit(folderID);

#define RESTRAINED_LOVE_ADD \
        LLAppearanceMgr::instance().addCategoryToCurrentOutfit(folderID);

template<> template<>
ECmdRet ForceHandler<EBehaviour::Attach>::onCommand(const RlvCommand& rlvCmd)
{
    RESTRAINED_LOVE_OUTFIT(RESTRAINED_LOVE_REPLACE);
}

template<> template<>
ECmdRet ForceHandler<EBehaviour::AttachOver>::onCommand(const RlvCommand& rlvCmd)
{
    RESTRAINED_LOVE_OUTFIT(RESTRAINED_LOVE_ADD);
}

template<> template<>
ECmdRet ForceHandler<EBehaviour::Detach>::onCommand(const RlvCommand& rlvCmd)
{
    auto folderID = gInventory.getRootFolderID();
    LLNameCategoryCollector has_name("#RLV");
    if (!gInventory.hasMatchingDirectDescendent(folderID, has_name))
        return ECmdRet::FailedNoSharedRoot;
    folderID = findDescendentCategoryIDByName(folderID, "#RLV");
    std::vector<std::string> optionList;
    auto option = rlvCmd.getOption();
    if (!option.empty())
    {
        LLNameCategoryCollector is_named(option);
        if (gInventory.hasMatchingDirectDescendent(folderID, is_named))
        {
            folderID = findDescendentCategoryIDByName(folderID, option);
            LLAppearanceMgr::instance().takeOffOutfit(folderID);
        }
    }
    return ECmdRet::Succeeded;
}

// AddRem

ECmdRet CommandHandlerBaseImpl<EParamType::AddRem>::processCommand(const RlvCommand& rlvCmd, BhvrHandlerFunc* pHandler, BhvrToggleHandlerFunc* pToggleHandler)
{
    auto param = rlvCmd.getParam();
    bool toggle = false;
    if (param == "y")
        toggle = true;
    else if (param != "n")
        return ECmdRet::FailedParam;
    return (*pHandler)(rlvCmd, toggle);
}

template<> template<>
ECmdRet BehaviourToggleHandler<EBehaviour::Sit>::onCommand(const RlvCommand& rlvCmd, bool& toggle)
{
    gAgent.setAllowedToSit(toggle);
    return ECmdRet::Succeeded;
}

template<> template<>
ECmdRet BehaviourToggleHandler<EBehaviour::Unsit>::onCommand(const RlvCommand& rlvCmd, bool& toggle)
{
    gAgent.setAllowedToStand(toggle);
    if (gAgent.isSitting())
        LLPanelStandStopFlying::getInstance()->setVisibleStandButton(toggle);
    return ECmdRet::Succeeded;
}

template<> template<>
ECmdRet BehaviourToggleHandler<EBehaviour::Detach>::onCommand(const RlvCommand& rlvCmd, bool& toggle)
{
    gObjectList.findObject(rlvCmd.getObjectID())->setLocked(!toggle);
    return ECmdRet::Succeeded;
}

// ============================================================================
