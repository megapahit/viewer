/**
 * @file test.cpp
 * @author Phoenix
 * @date 2005-09-26
 * @brief Entry point for the test app.
 *
 * $LicenseInfo:firstyear=2005&license=viewerlgpl$
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

/**
 *
 * You can add tests by creating a new cpp file in this directory, and
 * rebuilding. There are at most 50 tests per testgroup without a
 * little bit of template parameter and makefile tweaking.
 *
 */

#include "linden_common.h"
#include "llerrorcontrol.h"
#include "lltut.h"
#include "chained_callback.h"
#include "stringize.h"
#include "namedtempfile.h"
#include "lltrace.h"
#include "lltracethreadrecorder.h"

#include "apr_pools.h"
#include "apr_getopt.h"

// the CTYPE_WORKAROUND is needed for linux dev stations that don't
// have the broken libc6 packages needed by our out-of-date static
// libs (such as libcrypto and libcurl). -- Leviathan 20060113
#ifdef CTYPE_WORKAROUND
#   include "ctype_workaround.h"
#endif

#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>

// On Mac, got:
// #error "Boost.Stacktrace requires `_Unwind_Backtrace` function. Define
// `_GNU_SOURCE` macro or `BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED` if
// _Unwind_Backtrace is available without `_GNU_SOURCE`."
#define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
#include <boost/stacktrace.hpp>

#include <fstream>

void wouldHaveCrashed(const std::string& message);

namespace tut
{
    std::string sSourceDir;

    test_runner_singleton runner;
}

class LLReplayLog
{
public:
    LLReplayLog() {}
    virtual ~LLReplayLog() {}

    virtual void reset() {}
    virtual void replay(std::ostream&) {}
};

class RecordToTempFile : public LLError::Recorder, public boost::noncopyable
{
public:
    RecordToTempFile()
        : LLError::Recorder(),
        boost::noncopyable(),
        mTempFile("log", ""),
        mFile(mTempFile.getName().c_str())
    {
    }

    virtual ~RecordToTempFile()
    {
        mFile.close();
    }

    virtual void recordMessage(LLError::ELevel level, const std::string& message)
    {
        LL_PROFILE_ZONE_SCOPED;
        mFile << message << std::endl;
    }

    void reset()
    {
        mFile.close();
        mFile.open(mTempFile.getName().c_str());
    }

    void replay(std::ostream& out)
    {
        mFile.close();
        std::ifstream inf(mTempFile.getName().c_str());
        std::string line;
        while (std::getline(inf, line))
        {
            out << line << std::endl;
        }
    }

private:
    NamedTempFile mTempFile;
    llofstream mFile;
};

class LLReplayLogReal: public LLReplayLog, public boost::noncopyable
{
public:
    LLReplayLogReal(LLError::ELevel level)
        : LLReplayLog(),
        boost::noncopyable(),
        mOldSettings(LLError::saveAndResetSettings()),
        mRecorder(new RecordToTempFile())
    {
        LLError::setFatalFunction(wouldHaveCrashed);
        LLError::setDefaultLevel(level);
        LLError::addRecorder(mRecorder);
    }

    virtual ~LLReplayLogReal()
    {
        LLError::removeRecorder(mRecorder);
        LLError::restoreSettings(mOldSettings);
    }

    virtual void reset()
    {
        std::dynamic_pointer_cast<RecordToTempFile>(mRecorder)->reset();
    }

    virtual void replay(std::ostream& out)
    {
        std::dynamic_pointer_cast<RecordToTempFile>(mRecorder)->replay(out);
    }

private:
    LLError::SettingsStoragePtr mOldSettings;
    LLError::RecorderPtr mRecorder;
};

class LLTestCallback : public chained_callback
{
    typedef chained_callback super;

public:
    LLTestCallback(bool verbose_mode, std::ostream *stream,
                   std::shared_ptr<LLReplayLog> replayer) :
        mVerboseMode(verbose_mode),
        // By default, capture a shared_ptr to std::cout, with a no-op "deleter"
        // so that destroying the shared_ptr makes no attempt to delete std::cout.
        mStream(std::shared_ptr<std::ostream>(&std::cout, [](std::ostream*){})),
        mReplayer(replayer)
    {
        if (stream)
        {
            // We want a boost::iostreams::tee_device that will stream to two
            // std::ostreams.
            typedef boost::iostreams::tee_device<std::ostream, std::ostream> TeeDevice;
            // More than that, though, we want an actual stream using that
            // device.
            typedef boost::iostreams::stream<TeeDevice> TeeStream;
            // Allocate and assign in two separate steps, per Herb Sutter.
            // (Until we turn on C++11 support, have to wrap *stream with
            // boost::ref() due to lack of perfect forwarding.)
            std::shared_ptr<std::ostream> pstream(new TeeStream(std::cout, boost::ref(*stream)));
            mStream = pstream;
        }
    }

    ~LLTestCallback()
    {
    }

    virtual void run_started()
    {
        //std::cout << "run_started" << std::endl;
        LL_INFOS("TestRunner")<<"Test Started"<< LL_ENDL;
        super::run_started();
    }

    virtual void group_started(const std::string& name) {
        LL_INFOS("TestRunner")<<"Unit test group_started name=" << name << LL_ENDL;
        *mStream << "Unit test group_started name=" << name << std::endl;
        mGroup = name;
        mGroupTests = 0;
        super::group_started(name);
    }

    virtual void group_completed(const std::string& name) {
        LL_INFOS("TestRunner")<<"Unit test group_completed name=" << name << LL_ENDL;
        *mStream << "Unit test group_completed name=" << name << std::endl;
        super::group_completed(name);
    }

    virtual void test_completed(const tut::test_result& tr)
    {
        ++mTotalTests;
        ++mGroupTests;

        // If this test failed, dump requested log messages BEFORE stating the
        // test result.
        if (tr.result != tut::test_result::ok && tr.result != tut::test_result::skip)
        {
            mReplayer->replay(*mStream);
        }
        // Either way, clear stored messages in preparation for next test.
        mReplayer->reset();

        std::ostringstream out;
        out << "[" << tr.group << ", " << tr.test;
        if (! tr.name.empty())
            out << ": " << tr.name;
        out << "] ";
        switch(tr.result)
        {
            case tut::test_result::ok:
                ++mPassedTests;
                out << "ok";
                break;
            case tut::test_result::fail:
                ++mFailedTests;
                out << "fail";
                break;
            case tut::test_result::ex:
                ++mFailedTests;
                out << "exception: " << LLError::Log::demangle(tr.exception_typeid.c_str());
                break;
            case tut::test_result::warn:
                ++mFailedTests;
                out << "test destructor throw";
                break;
            case tut::test_result::term:
                ++mFailedTests;
                out << "abnormal termination";
                break;
            case tut::test_result::skip:
                ++mSkippedTests;
                out << "skipped known failure";
                break;
            default:
                ++mFailedTests;
                out << "unknown (tr.result == " << tr.result << ")";
        }
        if(mVerboseMode || (tr.result != tut::test_result::ok))
        {
            *mStream << out.str();
            if(!tr.message.empty())
            {
                *mStream << ": '" << tr.message << "'";
                LL_WARNS("TestRunner") << "not ok : "<<tr.message << LL_ENDL;
            }
            *mStream << std::endl;
        }
        LL_INFOS("TestRunner")<<out.str()<<LL_ENDL;
        super::test_completed(tr);
    }

    virtual int getFailedTests() const { return mFailedTests; }

    virtual void run_completed()
    {
        *mStream << "\tTotal Tests:\t" << mTotalTests << std::endl;
        *mStream << "\tPassed Tests:\t" << mPassedTests;
        if (mPassedTests == mTotalTests)
        {
            *mStream << "\tYAY!! \\o/";
        }
        *mStream << std::endl;

        if (mSkippedTests > 0)
        {
            *mStream << "\tSkipped known failures:\t" << mSkippedTests
            << std::endl;
        }

        if(mFailedTests > 0)
        {
            *mStream << "*********************************" << std::endl;
            *mStream << "Failed Tests:\t" << mFailedTests << std::endl;
            *mStream << "Please report or fix the problem." << std::endl;
            *mStream << "*********************************" << std::endl;
        }
        super::run_completed();
    }

    std::string mGroup;
    int mGroupTests{ 0 };

protected:
    bool mVerboseMode{ false };
    int mTotalTests{ 0 };
    int mPassedTests{ 0 };
    int mFailedTests{ 0 };
    int mSkippedTests{ 0 };
    std::shared_ptr<std::ostream> mStream;
    std::shared_ptr<LLReplayLog> mReplayer;
};

// TeamCity specific class which emits service messages
// http://confluence.jetbrains.net/display/TCD3/Build+Script+Interaction+with+TeamCity;#BuildScriptInteractionwithTeamCity-testReporting

class LLTCTestCallback : public LLTestCallback
{
public:
    LLTCTestCallback(bool verbose_mode, std::ostream *stream,
                     std::shared_ptr<LLReplayLog> replayer) :
        LLTestCallback(verbose_mode, stream, replayer)
    {
    }

    ~LLTCTestCallback()
    {
    }

    virtual void group_started(const std::string& name) {
        LLTestCallback::group_started(name);
        std::cout << "\n##teamcity[testSuiteStarted name='" << escape(name) << "']" << std::endl;
    }

    virtual void group_completed(const std::string& name) {
        LLTestCallback::group_completed(name);
        std::cout << "##teamcity[testSuiteFinished name='" << escape(name) << "']" << std::endl;
    }

    virtual void test_completed(const tut::test_result& tr)
    {
        std::string testname(STRINGIZE(tr.group << "." << tr.test));
        if (! tr.name.empty())
        {
            testname.append(":");
            testname.append(tr.name);
        }
        testname = escape(testname);

        // Sadly, tut::callback doesn't give us control at test start; have to
        // backfill start message into TC output.
        std::cout << "##teamcity[testStarted name='" << testname << "']" << std::endl;

        // now forward call to base class so any output produced there is in
        // the right TC context
        LLTestCallback::test_completed(tr);

        switch(tr.result)
        {
            case tut::test_result::ok:
                break;

            case tut::test_result::fail:
            case tut::test_result::ex:
            case tut::test_result::warn:
            case tut::test_result::term:
                std::cout << "##teamcity[testFailed name='" << testname
                          << "' message='" << escape(tr.message) << "']" << std::endl;
                break;

            case tut::test_result::skip:
                std::cout << "##teamcity[testIgnored name='" << testname << "']" << std::endl;
                break;

            default:
                break;
        }

        std::cout << "##teamcity[testFinished name='" << testname << "']" << std::endl;
    }

    static std::string escape(const std::string& str)
    {
        // Per http://confluence.jetbrains.net/display/TCD65/Build+Script+Interaction+with+TeamCity#BuildScriptInteractionwithTeamCity-ServiceMessages
        std::string result;
        for (char c : str)
        {
            switch (c)
            {
            case '\'':
                result.append("|'");
                break;
            case '\n':
                result.append("|n");
                break;
            case '\r':
                result.append("|r");
                break;
/*==========================================================================*|
            // These are not possible 'char' values from a std::string.
            case '\u0085':          // next line
                result.append("|x");
                break;
            case '\u2028':          // line separator
                result.append("|l");
                break;
            case '\u2029':          // paragraph separator
                result.append("|p");
                break;
|*==========================================================================*/
            case '|':
                result.append("||");
                break;
            case '[':
                result.append("|[");
                break;
            case ']':
                result.append("|]");
                break;
            default:
                result.push_back(c);
                break;
            }
        }
        return result;
    }
};


static const apr_getopt_option_t TEST_CL_OPTIONS[] =
{
    {"help", 'h', 0, "Print the help message."},
    {"list", 'l', 0, "List available test groups."},
    {"verbose", 'v', 0, "Verbose output."},
    {"group", 'g', 1, "Run test group specified by option argument."},
    {"output", 'o', 1, "Write output to the named file."},
    {"sourcedir", 's', 1, "Project source file directory from CMake."},
    {"touch", 't', 1, "Touch the given file if all tests succeed"},
    {"wait", 'w', 0, "Wait for input before exit."},
    {"debug", 'd', 0, "Emit full debug logs."},
    {"suitename", 'x', 1, "Run tests using this suitename"},
    {0, 0, 0, 0}
};

void stream_usage(std::ostream& s, const char* app)
{
    s << "Usage: " << app << " [OPTIONS]" << std::endl
    << std::endl;

    s << "This application runs the unit tests." << std::endl << std::endl;

    s << "Options: " << std::endl;
    const apr_getopt_option_t* option = &TEST_CL_OPTIONS[0];
    while(option->name)
    {
        s << "  ";
        s << "  -" << (char)option->optch << ", --" << option->name
        << std::endl;
        s << "\t" << option->description << std::endl << std::endl;
        ++option;
    }

    s << app << " is also sensitive to environment variables:\n"
      << "LOGTEST=level : for all tests, emit log messages at level 'level'\n"
      << "LOGFAIL=level : only for failed tests, emit log messages at level 'level'\n"
      << "where 'level' is one of ALL, DEBUG, INFO, WARN, ERROR, NONE.\n"
      << "--debug is like LOGTEST=DEBUG, but --debug overrides LOGTEST,\n"
      << "while LOGTEST overrides LOGFAIL.\n\n";

    s << "Examples:" << std::endl;
    s << "  " << app << " --verbose" << std::endl;
    s << "\tRun all the tests and report all results." << std::endl;
    s << "  " << app << " --list" << std::endl;
    s << "\tList all available test groups." << std::endl;
    s << "  " << app << " --group=uuid" << std::endl;
    s << "\tRun the test group 'uuid'." << std::endl;

    s << "\n\n"
      << "In any event, logs are recorded in the build directory by appending\n"
      << "the suffix '.log' to the full path name of this application.\n"
      << "If no level is specified as described above, these log files are at\n"
      << "DEBUG level.\n"
        ;
}

void stream_groups(std::ostream& s, const char* app)
{
    s << "Registered test groups:" << std::endl;
    tut::groupnames gl = tut::runner.get().list_groups();
    tut::groupnames::const_iterator it = gl.begin();
    tut::groupnames::const_iterator end = gl.end();
    for(; it != end; ++it)
    {
        s << "  " << *(it) << std::endl;
    }
}

void wouldHaveCrashed(const std::string& message)
{
    tut::fail("llerrs message: " + message);
}

static LLTrace::ThreadRecorder* sMasterThreadRecorder = NULL;

// this is used in platform-generic code -- define outside #if LL_WINDOWS
struct Windows_SEH_exception: public std::runtime_error
{
    Windows_SEH_exception(const std::string& what): std::runtime_error(what) {}
};

#if LL_WINDOWS

static constexpr U32 STATUS_MSC_EXCEPTION = 0xE06D7363; // compiler specific
static constexpr U32 STATUS_STACK_FULL    = 0xC00000FD;

U32 seh_filter(U32 code, struct _EXCEPTION_POINTERS*)
{
    if (code == STATUS_MSC_EXCEPTION)
    {
        // C++ exception, go on -- but TUT is supposed to have caught those already?!
        return EXCEPTION_CONTINUE_SEARCH;
    }
    else
    {
        // This is a non-C++ exception, e.g. hardware check.
        // By the time the handler gets control, the stack has been unwound,
        // so report the stack trace now at filter() time.
        // Sadly, even though, at the time of this writing, stack overflow is
        // the problem we would most like to diagnose, calling another
        // function when the stack is already blown only terminates us faster.
        if (code != STATUS_STACK_FULL)
        {
            std::cerr << boost::stacktrace::stacktrace() << std::endl;
        }
        // pass control into the handler block
        return EXCEPTION_EXECUTE_HANDLER;
    }
}

template <typename CALLABLE0, typename CALLABLE1>
void seh_catcher(CALLABLE0&& trycode, CALLABLE1&& handler)
{
    __try
    {
        trycode();
    }
    __except (seh_filter(GetExceptionCode(), GetExceptionInformation()))
    {
        handler(GetExceptionCode());
    }
}

#else  // not LL_WINDOWS

template <typename CALLABLE0, typename CALLABLE1>
void seh_catcher(CALLABLE0&& trycode, CALLABLE1&&)
{
    trycode();
}

#endif // not LL_WINDOWS

int main(int argc, char **argv)
{
    ll_init_apr();
    apr_getopt_t* os = NULL;
    if(APR_SUCCESS != apr_getopt_init(&os, gAPRPoolp, argc, argv))
    {
        std::cerr << "apr_getopt_init() failed" << std::endl;
        return 1;
    }

    // values used for controlling application
    bool verbose_mode = false;
    bool wait_at_exit = false;
    std::string test_group;
    std::string suite_name;

    // LOGTEST overrides default, but can be overridden by --debug.
    const char* LOGTEST = getenv("LOGTEST");

    // values used for options parsing
    apr_status_t apr_err;
    const char* opt_arg = NULL;
    int opt_id = 0;
    std::unique_ptr<llofstream> output;
    const char *touch = NULL;

    while(true)
    {
        apr_err = apr_getopt_long(os, TEST_CL_OPTIONS, &opt_id, &opt_arg);
        if(APR_STATUS_IS_EOF(apr_err)) break;
        if(apr_err)
        {
            char buf[255];      /* Flawfinder: ignore */
            std::cerr << "Error parsing options: "
            << apr_strerror(apr_err, buf, 255) << std::endl;
            return 1;
        }
        switch (opt_id)
        {
            case 'g':
                test_group.assign(opt_arg);
                break;
            case 'h':
                stream_usage(std::cout, argv[0]);
                return 0;
                break;
            case 'l':
                stream_groups(std::cout, argv[0]);
                return 0;
            case 'v':
                verbose_mode = true;
                break;
            case 'o':
                output.reset(new llofstream);
                output->open(opt_arg);
                break;
            case 's':   // --sourcedir
                tut::sSourceDir = opt_arg;
                // For convenience, so you can use tut::sSourceDir + "myfile"
                tut::sSourceDir += '/';
                break;
            case 't':
                touch = opt_arg;
                break;
            case 'w':
                wait_at_exit = true;
                break;
            case 'd':
                LOGTEST = "DEBUG";
                break;
            case 'x':
                suite_name.assign(opt_arg);
                break;
            default:
                stream_usage(std::cerr, argv[0]);
                return 1;
                break;
        }
    }

    // set up logging
    const char* LOGFAIL = getenv("LOGFAIL");
    std::shared_ptr<LLReplayLog> replayer{std::make_shared<LLReplayLog>()};

    // Testing environment variables for both 'set' and 'not empty' allows a
    // user to suppress a pre-existing environment variable by forcing empty.
    if (LOGTEST && *LOGTEST)
    {
        LLError::initForApplication(".", ".", true /* log to stderr */);
        LLError::setDefaultLevel(LLError::decodeLevel(LOGTEST));
    }
    else
    {
        LLError::initForApplication(".", ".", false /* do not log to stderr */);
        LLError::setDefaultLevel(LLError::LEVEL_DEBUG);
        if (LOGFAIL && *LOGFAIL)
        {
            LLError::ELevel level = LLError::decodeLevel(LOGFAIL);
            replayer.reset(new LLReplayLogReal(level));
        }
    }
    LLError::setFatalFunction(wouldHaveCrashed);
    std::string test_app_name(argv[0]);
    std::string test_log = test_app_name + ".log";
    LLFile::remove(test_log);
    LLError::logToFile(test_log);

#ifdef CTYPE_WORKAROUND
    ctype_workaround();
#endif

    if (!sMasterThreadRecorder)
    {
        sMasterThreadRecorder = new LLTrace::ThreadRecorder();
        LLTrace::set_master_thread_recorder(sMasterThreadRecorder);
    }

    // run the tests

    LLTestCallback* mycallback;
    if (getenv("TEAMCITY_PROJECT_NAME"))
    {
        mycallback = new LLTCTestCallback(verbose_mode, output.get(), replayer);
    }
    else
    {
        mycallback = new LLTestCallback(verbose_mode, output.get(), replayer);
    }

    // a chained_callback subclass must be linked with previous
    mycallback->link();

    seh_catcher(
        // __try
        [test_group]
        {
            if(test_group.empty())
            {
                tut::runner.get().run_tests();
            }
            else
            {
                tut::runner.get().run_tests(test_group);
            }
        },
        // __except
        [mycallback](U32 code)
        {
            static std::map<U32, const char*> codes = {
                { 0xC0000005, "Access Violation" },
                { 0xC00000FD, "Stack Overflow" },
                // ... continue filling in as desired
            };

            auto found{ codes.find(code) };
            const char* name = ((found == codes.end())? "unknown" : found->second);
            auto msg{ stringize("test threw ", std::hex, code, " (", name, ")") };

            // Instead of bombing the whole test run, report this as a test
            // failure. Arguably, catching structured exceptions should be
            // hacked into TUT itself.
            mycallback->test_completed(tut::test_result(
                mycallback->mGroup,
                mycallback->mGroupTests+1, // test within group
                "unknown",                 // test name
                tut::test_result::ex,      // result: exception
                // we don't have to throw this exception subclass to use it to
                // populate the test_result struct
                Windows_SEH_exception(msg)));
            // we've left the TUT framework -- finish up by hand
            mycallback->group_completed(mycallback->mGroup);
            mycallback->run_completed();
        });

    bool success = (mycallback->getFailedTests() == 0);

    if (wait_at_exit)
    {
        std::cerr << "Press return to exit..." << std::endl;
        std::cin.get();
    }

    if (touch && success)
    {
        llofstream s;
        s.open(touch);
        s << "ok" << std::endl;
        s.close();
    }

    ll_cleanup_apr();

    int retval = (success ? 0 : 1);
    return retval;

    //delete mycallback;
}
