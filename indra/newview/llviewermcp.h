/**
 * @file llviewermcp.h
 * @brief Loopback MCP server for the running viewer session.
 */

#ifndef LL_LLVIEWERMCP_H
#define LL_LLVIEWERMCP_H

#include "lluuid.h"
#include <string>

class LLViewerMCP
{
public:
    // Start or stop the listener from the setting, and run queued tool calls.
    // Call this on the main thread.
    static void pump();
    static void shutdown();

    // Record a chat or IM line the viewer already received. Main thread only.
    static void noteChat(const std::string& kind,
                         const std::string& from,
                         const LLUUID& from_id,
                         const std::string& text);

    // Record a script debug-channel line. These open the script debug tab
    // and are not nearby chat. Main thread only.
    static void noteDebug(const std::string& from,
                          const LLUUID& from_id,
                          const std::string& text);
};

#endif
