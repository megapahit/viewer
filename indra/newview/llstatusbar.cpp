/**
* @file llstatusbar.cpp
* @brief LLStatusBar class implementation
*
* $LicenseInfo:firstyear=2002&license=viewerlgpl$
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

#include "llviewerprecompiledheaders.h"

#include "llstatusbar.h"

// viewer includes
#include "llagent.h"
#include "llagentcamera.h"
#include "llbutton.h"
#include "llcommandhandler.h"
#include "llfirstuse.h"
#include "llviewercontrol.h"
#include "llfloaterbuycurrency.h"
#include "llbuycurrencyhtml.h"
#include "llpanelnearbymedia.h"
#include "llpanelpresetscamerapulldown.h"
#include "llpanelpresetspulldown.h"
#include "llpanelvolumepulldown.h"
#include "llfloatermarketplace.h"
#include "llfloaterregioninfo.h"
#include "llfloaterscriptdebug.h"
#include "llhints.h"
#include "llhudicon.h"
#include "llnavigationbar.h"
#include "llkeyboard.h"
#include "lllineeditor.h"
#include "llmenugl.h"
#include "llrootview.h"
#include "llsd.h"
#include "lltextbox.h"
#include "llui.h"
#include "llviewerparceloverlay.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"
#include "llframetimer.h"
#include "llvoavatarself.h"
#include "llresmgr.h"
#include "llworld.h"
#include "llstatgraph.h"
#include "llviewermedia.h"
#include "llviewermenu.h"   // for gMenuBarView
#include "llviewerparcelmgr.h"
#include "llviewerthrottle.h"
#include "lluictrlfactory.h"

#include "lltoolmgr.h"
#include "llfocusmgr.h"
#include "llappviewer.h"
#include "lltrans.h"

// library includes
#include "llfloaterreg.h"
#include "llfontgl.h"
#include "llrect.h"
#include "llerror.h"
#include "llnotificationsutil.h"
#include "llparcel.h"
#include "llstring.h"
#include "message.h"
#include "llsearchableui.h"
#include "llsearcheditor.h"
#include "bufferarray.h"
#include "httphandler.h"
#include "httpoptions.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "llsdjson.h"

// system includes
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include <boost/json.hpp>


//
// Globals
//
LLStatusBar *gStatusBar = NULL;
S32 STATUS_BAR_HEIGHT = 26;
extern S32 MENU_BAR_HEIGHT;


// TODO: these values ought to be in the XML too
const S32 SIM_STAT_WIDTH = 8;
const LLColor4 SIM_OK_COLOR(0.f, 1.f, 0.f, 1.f);
const LLColor4 SIM_WARN_COLOR(1.f, 1.f, 0.f, 1.f);
const LLColor4 SIM_FULL_COLOR(1.f, 0.f, 0.f, 1.f);
const F32 ICON_TIMER_EXPIRY     = 3.f; // How long the balance and health icons should flash after a change.

static void onClickVolume(void*);

LLStatusBar::LLStatusBar(const LLRect& rect)
:   LLPanel(),
    mTextTime(NULL),
    mTextFps(NULL),
    mSGBandwidth(NULL),
    mSGPacketLoss(NULL),
    mBtnVolume(NULL),
    mBoxBalance(NULL),
    mBalance(0),
    mExchangeRate(0.0),
    mExchangeRateFetchInFlight(false),
    mExchangeRateRequest(nullptr),
    mTreasuryFetchInFlight(false),
    mTreasuryReady(false),
    mTreasuryRequest(nullptr),
    mBalanceClicked(false),
    mObscureBalance(false),
    mHealth(100),
    mSquareMetersCredit(0),
    mSquareMetersCommitted(0),
    mFilterEdit(NULL),          // Edit for filtering
    mSearchPanel(NULL)          // Panel for filtering
{
    setRect(rect);

    // status bar can possible overlay menus?
    setMouseOpaque(false);

    mFpsUpdateTimer = new LLFrameTimer();

    buildFromFile("panel_status_bar.xml");
}

LLStatusBar::~LLStatusBar()
{
    delete mExchangeRateRequest;
    mExchangeRateRequest = nullptr;
    delete mTreasuryRequest;
    mTreasuryRequest = nullptr;
    // LLView destructor cleans up children
}

//-----------------------------------------------------------------------
// Overrides
//-----------------------------------------------------------------------

// virtual
void LLStatusBar::draw()
{
    LL_PROFILE_ZONE_SCOPED_CATEGORY_UI;
    // Deliver the LindeX reply on this thread. update() only invokes the handler.
    if (mExchangeRateRequest && mExchangeRateFetchInFlight)
    {
        mExchangeRateRequest->update(0);
    }
    if (mTreasuryRequest && mTreasuryFetchInFlight)
    {
        mTreasuryRequest->update(0);
    }
    refresh();
    LLPanel::draw();
}

bool LLStatusBar::handleRightMouseDown(S32 x, S32 y, MASK mask)
{
    show_navbar_context_menu(this,x,y);
    return true;
}

bool LLStatusBar::postBuild()
{
    gMenuBarView->setRightMouseDownCallback(boost::bind(&show_navbar_context_menu, _1, _2, _3));

    mTextTime = getChild<LLTextBox>("TimeText" );

    mTextFps = getChild<LLButton>("FpsText");
    mTextFps->setClickedCallback( &LLStatusBar::onClickFps, this );

    getChild<LLUICtrl>("buyL")->setCommitCallback(
        boost::bind(&LLStatusBar::onClickBuyCurrency, this));

    getChild<LLUICtrl>("goShop")->setCommitCallback(
        boost::bind(&LLStatusBar::onClickShop, this));

    mBoxBalance = getChild<LLTextBox>("balance");
    mBoxBalance->setClickedCallback(&LLStatusBar::onClickRefreshBalance, this);
    mBoxBalance->setDoubleClickCallback([this](LLUICtrl*, S32 x, S32 y, MASK mask) { onClickToggleBalance(); });

    mIconPresetsCamera = getChild<LLIconCtrl>( "presets_icon_camera" );
    mIconPresetsCamera->setMouseEnterCallback(boost::bind(&LLStatusBar::onMouseEnterPresetsCamera, this));

    mIconPresetsGraphic = getChild<LLIconCtrl>( "presets_icon_graphic" );
    mIconPresetsGraphic->setMouseEnterCallback(boost::bind(&LLStatusBar::onMouseEnterPresets, this));

    mBtnVolume = getChild<LLButton>( "volume_btn" );
    mBtnVolume->setClickedCallback( onClickVolume, this );
    mBtnVolume->setMouseEnterCallback(boost::bind(&LLStatusBar::onMouseEnterVolume, this));

    mMediaToggle = getChild<LLButton>("media_toggle_btn");
    mMediaToggle->setClickedCallback( &LLStatusBar::onClickMediaToggle, this );
    mMediaToggle->setMouseEnterCallback(boost::bind(&LLStatusBar::onMouseEnterNearbyMedia, this));

    LLHints::getInstance()->registerHintTarget("linden_balance", getChild<LLView>("balance_bg")->getHandle());

    gSavedSettings.getControl("MuteAudio")->getSignal()->connect(boost::bind(&LLStatusBar::onVolumeChanged, this, _2));
    gSavedSettings.getControl("EnableVoiceChat")->getSignal()->connect(boost::bind(&LLStatusBar::onVoiceChanged, this, _2));
    gSavedSettings.getControl("ObscureBalanceInStatusBar")->getSignal()->connect(boost::bind(&LLStatusBar::onObscureBalanceChanged, this, _2));
    gSavedSettings.getControl("ShowLocalCurrency")->getSignal()->connect(boost::bind(&LLStatusBar::onLocalCurrencyChanged, this, _2));
    gSavedSettings.getControl("LocalCurrencyOverride")->getSignal()->connect(boost::bind(&LLStatusBar::onLocalCurrencyChanged, this, _2));

    if (!gSavedSettings.getBOOL("EnableVoiceChat") && LLAppViewer::instance()->isSecondInstance())
    {
        // Indicate that second instance started without sound
        mBtnVolume->setImageUnselected(LLUI::getUIImage("VoiceMute_Off"));
    }
    mObscureBalance = gSavedSettings.getBOOL("ObscureBalanceInStatusBar");
    if (gSavedSettings.getBOOL("ShowLocalCurrency"))
    {
        requestExchangeRate();
        requestTreasuryRates();
    }

    // Adding Net Stat Graph
    S32 x = getRect().getWidth() - 2;
    S32 y = 0;
    LLRect r;
    r.set( x-SIM_STAT_WIDTH, y+MENU_BAR_HEIGHT-1, x, y+1);
    LLStatGraph::Params sgp;
    sgp.name("BandwidthGraph");
    sgp.rect(r);
    sgp.follows.flags(FOLLOWS_BOTTOM | FOLLOWS_RIGHT);
    sgp.mouse_opaque(false);
    sgp.stat.count_stat_float(&LLStatViewer::ACTIVE_MESSAGE_DATA_RECEIVED);
    sgp.units("Kbps");
    sgp.precision(0);
    sgp.per_sec(true);
    mSGBandwidth = LLUICtrlFactory::create<LLStatGraph>(sgp);
    addChild(mSGBandwidth);
    x -= SIM_STAT_WIDTH + 2;

    r.set( x-SIM_STAT_WIDTH, y+MENU_BAR_HEIGHT-1, x, y+1);
    //these don't seem to like being reused
    LLStatGraph::Params pgp;
    pgp.name("PacketLossPercent");
    pgp.rect(r);
    pgp.follows.flags(FOLLOWS_BOTTOM | FOLLOWS_RIGHT);
    pgp.mouse_opaque(false);
    pgp.stat.sample_stat_float(&LLStatViewer::PACKETS_LOST_PERCENT);
    pgp.units("%");
    pgp.min(0.f);
    pgp.max(5.f);
    pgp.precision(1);
    pgp.per_sec(false);
    LLStatGraph::Thresholds thresholds;
    thresholds.threshold.add(LLStatGraph::ThresholdParams().value(0.1).color(LLColor4::green))
                        .add(LLStatGraph::ThresholdParams().value(0.25f).color(LLColor4::yellow))
                        .add(LLStatGraph::ThresholdParams().value(0.6f).color(LLColor4::red));

    pgp.thresholds(thresholds);

    mSGPacketLoss = LLUICtrlFactory::create<LLStatGraph>(pgp);
    addChild(mSGPacketLoss);

    mPanelPresetsCameraPulldown = new LLPanelPresetsCameraPulldown();
    addChild(mPanelPresetsCameraPulldown);
    mPanelPresetsCameraPulldown->setFollows(FOLLOWS_TOP|FOLLOWS_RIGHT);
    mPanelPresetsCameraPulldown->setVisible(false);

    mPanelPresetsPulldown = new LLPanelPresetsPulldown();
    addChild(mPanelPresetsPulldown);
    mPanelPresetsPulldown->setFollows(FOLLOWS_TOP|FOLLOWS_RIGHT);
    mPanelPresetsPulldown->setVisible(false);

    mPanelVolumePulldown = new LLPanelVolumePulldown();
    addChild(mPanelVolumePulldown);
    mPanelVolumePulldown->setFollows(FOLLOWS_TOP|FOLLOWS_RIGHT);
    mPanelVolumePulldown->setVisible(false);

    mPanelNearByMedia = new LLPanelNearByMedia();
    addChild(mPanelNearByMedia);
    mPanelNearByMedia->setFollows(FOLLOWS_TOP|FOLLOWS_RIGHT);
    mPanelNearByMedia->setVisible(false);

    updateBalancePanelPosition();

    // Hook up and init for filtering
    mFilterEdit = getChild<LLSearchEditor>( "search_menu_edit" );
    mSearchPanel = getChild<LLPanel>( "menu_search_panel" );

    bool search_panel_visible = gSavedSettings.getBOOL("MenuSearch");
    mSearchPanel->setVisible(search_panel_visible);
    mFilterEdit->setKeystrokeCallback(boost::bind(&LLStatusBar::onUpdateFilterTerm, this));
    mFilterEdit->setCommitCallback(boost::bind(&LLStatusBar::onUpdateFilterTerm, this));
    collectSearchableItems();
    gSavedSettings.getControl("MenuSearch")->getCommitSignal()->connect(boost::bind(&LLStatusBar::updateMenuSearchVisibility, this, _2));

    if (search_panel_visible)
    {
        updateMenuSearchPosition();
    }

    return true;
}

// Per-frame updates of visibility
void LLStatusBar::refresh()
{
    static LLCachedControl<bool> show_net_stats(gSavedSettings, "ShowNetStats", false);
    bool net_stats_visible = show_net_stats;

    if (net_stats_visible)
    {
        // Adding Net Stat Meter back in
        F32 bwtotal = gViewerThrottle.getMaxBandwidth() / 1000.f;
        mSGBandwidth->setMin(0.f);
        mSGBandwidth->setMax(bwtotal*1.25f);
        //mSGBandwidth->setThreshold(0, bwtotal*0.75f);
        //mSGBandwidth->setThreshold(1, bwtotal);
        //mSGBandwidth->setThreshold(2, bwtotal);
    }

    if(mFpsUpdateTimer->getElapsedTimeF32() >= 1.0f)
    {
        mFpsUpdateTimer->reset();

        S32 fps = (S32) llround(LLTrace::get_frame_recording().getPeriodMedianPerSec(LLStatViewer::FPS, 50));
        if(fps < 1) fps = 1;

        S32 minFps = (S32) llround(LLTrace::get_frame_recording().getPeriodMinPerSec(LLStatViewer::FPS, 50));
        S32 maxFps = (S32) llround(LLTrace::get_frame_recording().getPeriodMaxPerSec(LLStatViewer::FPS, 50));

        F32 fpsQuality = (F32)minFps / (F32)fps;

        std::string fpsStr = std::to_string(fps);

        mTextFps->setLabel(fpsStr);

        if(fpsQuality < 0.5) mTextFps->setColor(LLColor4(0.6,0.0,0.0));
        else if(fpsQuality < 0.7) mTextFps->setColor(LLColor4(0.7,0.5,0.0));
        else if(fpsQuality < 0.9) mTextFps->setColor(LLColor4(0.5,0.7,0.0));
        else mTextFps->setColor(LLColor4(0.0, 0.7, 0.15));
    }

    // update clock every 10 seconds
    if(mClockUpdateTimer.getElapsedTimeF32() > 10.f)
    {
        mClockUpdateTimer.reset();

        // Get current UTC time, adjusted for the user's clock
        // being off.
        time_t utc_time;
        utc_time = time_corrected();

        static bool use_24h = gSavedSettings.getBOOL("Use24HourClock");
        std::string timeStr = use_24h ? getString("time") : getString("time_ampm");

        LLSD substitution;
        substitution["datetime"] = (S32) utc_time;
        LLStringUtil::format (timeStr, substitution);
        mTextTime->setText(timeStr);

        // set the tooltip to have the date
        std::string dtStr = getString("timeTooltip");
        LLStringUtil::format (dtStr, substitution);
        mTextTime->setToolTip (dtStr);
    }

    // LindeX moves during the day. Treasury rates are published quarterly.
    // Neither request runs until the local-currency line is turned on.
    if (gSavedSettings.getBOOL("ShowLocalCurrency"))
    {
        const F32 rate_refresh_delay = (mExchangeRate > 0.0) ? (15.f * 60.f) : 30.f;
        if (!mExchangeRateFetchInFlight && mExchangeRateTimer.getElapsedTimeF32() > rate_refresh_delay)
        {
            requestExchangeRate();
        }
        const F32 treasury_delay = mTreasuryReady ? (6.f * 60.f * 60.f) : 60.f;
        if (!mTreasuryFetchInFlight && mTreasuryTimer.getElapsedTimeF32() > treasury_delay)
        {
            requestTreasuryRates();
        }
    }

    if (mBalanceClicked && mBalanceClickTimer.getElapsedTimeF32() > 1.f)
    {
        mBalanceClicked = false;
        sendMoneyBalanceRequest();
    }

    LLRect r;
    const S32 MENU_RIGHT = gMenuBarView->getRightmostMenuEdge();

    // reshape menu bar to its content's width
    if (MENU_RIGHT != gMenuBarView->getRect().getWidth())
    {
        gMenuBarView->reshape(MENU_RIGHT, gMenuBarView->getRect().getHeight());
    }

    mSGBandwidth->setVisible(net_stats_visible);
    mSGPacketLoss->setVisible(net_stats_visible);

    // update the master volume button state
    bool mute_audio = LLAppViewer::instance()->getMasterSystemAudioMute();
    mBtnVolume->setToggleState(mute_audio);

    LLViewerMedia* media_inst = LLViewerMedia::getInstance();

    // Disable media toggle if there's no media, parcel media, and no parcel audio
    // (or if media is disabled)
    bool button_enabled = (gSavedSettings.getBOOL("AudioStreamingMusic")||gSavedSettings.getBOOL("AudioStreamingMedia")) &&
                          (media_inst->hasInWorldMedia() || media_inst->hasParcelMedia() || media_inst->hasParcelAudio());
    mMediaToggle->setEnabled(button_enabled);
    // Note the "sense" of the toggle is opposite whether media is playing or not
    bool any_media_playing = (media_inst->isAnyMediaPlaying() ||
                              media_inst->isParcelMediaPlaying() ||
                              media_inst->isParcelAudioPlaying());
    mMediaToggle->setValue(!any_media_playing);
}

void LLStatusBar::setVisibleForMouselook(bool visible)
{
    mTextTime->setVisible(visible);
    getChild<LLUICtrl>("balance_bg")->setVisible(visible);
    mBoxBalance->setVisible(visible);
    mBtnVolume->setVisible(visible);
    mMediaToggle->setVisible(visible);
    mSGBandwidth->setVisible(visible);
    mSGPacketLoss->setVisible(visible);
    mSearchPanel->setVisible(visible && gSavedSettings.getBOOL("MenuSearch"));
    setBackgroundVisible(visible);
    mIconPresetsCamera->setVisible(visible);
    mIconPresetsGraphic->setVisible(visible);
}

void LLStatusBar::debitBalance(S32 debit)
{
    setBalance(getBalance() - debit);
}

void LLStatusBar::creditBalance(S32 credit)
{
    setBalance(getBalance() + credit);
}

namespace
{

struct CurrencyInfo
{
    const char* iso;
    const char* treasury; // country_currency_desc; null when the currency is USD
    const char* symbol;
    S32 frac_digits;
    S32 symbol_before; // 1 before, 0 after, -1 follow the locale when it matches
    S32 symbol_space;
};

// Treasury publishes no USD row. One USD is the base of that table.
const CurrencyInfo CURRENCIES[] =
{
    {"USD", nullptr, "$", 2, 1, 0},
    {"EUR", "Euro Zone-Euro", "€", 2, -1, -1},
    {"GBP", "United Kingdom-Pound", "£", 2, 1, 0},
    {"JPY", "Japan-Yen", "¥", 0, 1, 0},
    {"CAD", "Canada-Dollar", "CA$", 2, 1, 0},
    {"AUD", "Australia-Dollar", "A$", 2, 1, 0},
    {"NZD", "New Zealand-Dollar", "NZ$", 2, 1, 0},
    {"CHF", "Switzerland-Franc", "Fr", 2, 0, 1},
    {"CNY", "China-Renminbi", "¥", 2, 1, 0},
    {"INR", "India-Rupee", "₹", 2, 1, 0},
    {"KRW", "Korea-Won", "₩", 0, 1, 0},
    {"HKD", "Hong Kong-Dollar", "HK$", 2, 1, 0},
    {"SGD", "Singapore-Dollar", "S$", 2, 1, 0},
    {"SEK", "Sweden-Krona", "kr", 2, 0, 1},
    {"NOK", "Norway-Krone", "kr", 2, 0, 1},
    {"DKK", "Denmark-Krone", "kr", 2, 0, 1},
    {"PLN", "Poland-Zloty", "zł", 2, 0, 1},
    {"CZK", "Czech Republic-Koruna", "Kč", 2, 0, 1},
    {"HUF", "Hungary-Forint", "Ft", 0, 0, 1},
    {"BRL", "Brazil-Real", "R$", 2, 1, 0},
    {"MXN", "Mexico-Peso", "Mex$", 2, 1, 0},
    {"ZAR", "South Africa-Rand", "R", 2, 1, 0},
    {"TRY", "Turkey-New Lira", "₺", 2, 1, 0},
    {"RUB", "Russia-Ruble", "₽", 2, 0, 1},
    {"UAH", "Ukraine-Hryvnia", "₴", 2, 0, 1},
    {"ILS", "Israel-Shekel", "₪", 2, 1, 0},
    {"THB", "Thailand-Baht", "฿", 2, 1, 0},
    {"PHP", "Philippines-Peso", "₱", 2, 1, 0},
    {"VND", "Vietnam-Dong", "₫", 0, 0, 1},
    {"IDR", "Indonesia-Rupiah", "Rp", 0, 1, 0},
    {"MYR", "Malaysia-Ringgit", "RM", 2, 1, 0},
    {"TWD", "Taiwan-Dollar", "NT$", 2, 1, 0},
    {"AED", "United Arab Emirates-Dirham", "د.إ", 2, 1, 0},
    {"SAR", "Saudi Arabia-Riyal", "﷼", 2, 1, 0},
    {"QAR", "Qatar-Riyal", "﷼", 2, 1, 0},
    {"KWD", "Kuwait-Dinar", "KD", 3, 1, 0},
    {"BHD", "Bahrain-Dinar", "BD", 3, 1, 0},
    {"EGP", "Egypt-Pound", "E£", 2, 1, 0},
    {"NGN", "Nigeria-Naira", "₦", 2, 1, 0},
    {"KES", "Kenya-Shilling", "KSh", 2, 1, 0},
    {"ARS", "Argentina-Peso", "AR$", 2, 1, 0},
    {"CLP", "Chile-Peso", "CLP$", 0, 1, 0},
    {"COP", "Colombia-Peso", "COL$", 0, 1, 0},
    {"PEN", "Peru-Sol", "S/", 2, 1, 0},
    {"RON", "Romania-New Leu", "lei", 2, 0, 1},
    {"ISK", "Iceland-Krona", "kr", 0, 0, 1},
    {"RSD", "Serbia-Dinar", "din", 2, 0, 1},
};

struct RegionCurrency
{
    const char* region;
    const char* iso;
};

const RegionCurrency REGIONS[] =
{
    {"US", "USD"}, {"PR", "USD"}, {"GU", "USD"}, {"VI", "USD"}, {"AS", "USD"},
    {"EC", "USD"}, {"SV", "USD"}, {"PA", "USD"}, {"MH", "USD"}, {"FM", "USD"}, {"PW", "USD"},
    {"DE", "EUR"}, {"FR", "EUR"}, {"ES", "EUR"}, {"IT", "EUR"}, {"NL", "EUR"}, {"BE", "EUR"},
    {"AT", "EUR"}, {"IE", "EUR"}, {"PT", "EUR"}, {"FI", "EUR"}, {"GR", "EUR"}, {"LU", "EUR"},
    {"SK", "EUR"}, {"SI", "EUR"}, {"EE", "EUR"}, {"LV", "EUR"}, {"LT", "EUR"}, {"MT", "EUR"},
    {"CY", "EUR"}, {"HR", "EUR"}, {"AD", "EUR"}, {"MC", "EUR"}, {"SM", "EUR"}, {"VA", "EUR"},
    {"GB", "GBP"}, {"UK", "GBP"}, {"GG", "GBP"}, {"JE", "GBP"}, {"IM", "GBP"},
    {"JP", "JPY"}, {"CA", "CAD"}, {"AU", "AUD"}, {"NZ", "NZD"}, {"CH", "CHF"}, {"LI", "CHF"},
    {"CN", "CNY"}, {"HK", "HKD"}, {"SG", "SGD"}, {"IN", "INR"}, {"KR", "KRW"},
    {"SE", "SEK"}, {"NO", "NOK"}, {"DK", "DKK"}, {"PL", "PLN"}, {"CZ", "CZK"}, {"HU", "HUF"},
    {"BR", "BRL"}, {"MX", "MXN"}, {"ZA", "ZAR"}, {"TR", "TRY"}, {"RU", "RUB"}, {"UA", "UAH"},
    {"IL", "ILS"}, {"TH", "THB"}, {"PH", "PHP"}, {"VN", "VND"}, {"ID", "IDR"}, {"MY", "MYR"},
    {"TW", "TWD"}, {"AE", "AED"}, {"SA", "SAR"}, {"QA", "QAR"}, {"KW", "KWD"}, {"BH", "BHD"},
    {"EG", "EGP"}, {"NG", "NGN"}, {"KE", "KES"}, {"AR", "ARS"}, {"CL", "CLP"}, {"CO", "COP"},
    {"PE", "PEN"}, {"RO", "RON"}, {"IS", "ISK"}, {"RS", "RSD"},
};

struct ResolvedCurrency
{
    std::string iso;
    std::string symbol;
    F64 per_usd = 0.0;
    S32 frac_digits = 2;
    bool symbol_before = true;
    bool symbol_space = false;
    bool is_usd = false;
    bool known = false;
};

const CurrencyInfo* find_currency_iso(const std::string& iso)
{
    for (size_t index = 0; index < sizeof(CURRENCIES) / sizeof(CURRENCIES[0]); ++index)
    {
        if (iso == CURRENCIES[index].iso)
        {
            return &CURRENCIES[index];
        }
    }
    return nullptr;
}

const CurrencyInfo* find_currency_treasury(const std::string& treasury_lower)
{
    for (size_t index = 0; index < sizeof(CURRENCIES) / sizeof(CURRENCIES[0]); ++index)
    {
        if (!CURRENCIES[index].treasury)
        {
            continue;
        }
        std::string name = CURRENCIES[index].treasury;
        LLStringUtil::toLower(name);
        if (name == treasury_lower)
        {
            return &CURRENCIES[index];
        }
    }
    return nullptr;
}

const char* currency_for_region(const std::string& region)
{
    for (size_t index = 0; index < sizeof(REGIONS) / sizeof(REGIONS[0]); ++index)
    {
        if (region == REGIONS[index].region)
        {
            return REGIONS[index].iso;
        }
    }
    return nullptr;
}

const char* currency_for_language(const std::string& language)
{
    std::string code = language;
    const std::string::size_type split = code.find_first_of("_-");
    if (split != std::string::npos)
    {
        code.erase(split);
    }
    LLStringUtil::toLower(code);
    if (code == "de" || code == "fr" || code == "es" || code == "it" || code == "nl"
        || code == "pt" || code == "fi" || code == "el" || code == "ga")
    {
        return "EUR";
    }
    if (code == "ja") return "JPY";
    if (code == "zh") return "CNY";
    if (code == "ko") return "KRW";
    if (code == "ru") return "RUB";
    if (code == "pl") return "PLN";
    if (code == "uk") return "UAH";
    if (code == "tr") return "TRY";
    if (code == "da") return "DKK";
    if (code == "sv") return "SEK";
    if (code == "nb" || code == "nn" || code == "no") return "NOK";
    if (code == "cs") return "CZK";
    if (code == "hu") return "HUF";
    if (code == "en") return "USD";
    return nullptr;
}

std::string region_from_locale(const std::string& name)
{
    const std::string::size_type sep = name.find_first_of("_-");
    if (sep == std::string::npos || sep + 2 >= name.size())
    {
        return std::string();
    }
    std::string region = name.substr(sep + 1, 2);
    const bool script = sep + 3 < name.size()
        && std::isalpha(static_cast<unsigned char>(name[sep + 3]))
        && name[sep + 3] != '.';
    if (script)
    {
        const std::string::size_type sep2 = name.find_first_of("_-", sep + 1);
        if (sep2 == std::string::npos || sep2 + 2 >= name.size())
        {
            return std::string();
        }
        region = name.substr(sep2 + 1, 2);
    }
    if (!std::isalpha(static_cast<unsigned char>(region[0]))
        || !std::isalpha(static_cast<unsigned char>(region[1])))
    {
        return std::string();
    }
    LLStringUtil::toUpper(region);
    return region;
}

const char* preferred_treasury_word(const std::string& word)
{
    if (word == "euro") return "Euro Zone-Euro";
    if (word == "pound" || word == "sterling") return "United Kingdom-Pound";
    if (word == "yen") return "Japan-Yen";
    if (word == "rupee") return "India-Rupee";
    if (word == "franc") return "Switzerland-Franc";
    if (word == "won") return "Korea-Won";
    if (word == "yuan" || word == "renminbi" || word == "rmb") return "China-Renminbi";
    if (word == "lira") return "Turkey-New Lira";
    if (word == "ruble" || word == "rouble") return "Russia-Ruble";
    if (word == "dollar" || word == "usd" || word == "us dollar" || word == "u.s. dollar") return nullptr;
    return nullptr;
}

std::string symbol_for_currency_name(const std::string& name)
{
    if (name == "Euro") return "€";
    if (name == "Yen") return "¥";
    if (name == "Pound") return "£";
    if (name == "Rupee") return "₹";
    if (name == "Won") return "₩";
    if (name == "Baht") return "฿";
    if (name == "Dong") return "₫";
    if (name == "Ruble") return "₽";
    if (name == "Shekel") return "₪";
    if (name == "Dollar" || name == "Dolares" || name == "U.S. Dollar") return "$";
    if (name == "Franc") return "Fr";
    if (name == "Krone" || name == "Krona") return "kr";
    if (name == "Zloty") return "zł";
    if (name == "Forint") return "Ft";
    if (name == "Real") return "R$";
    if (name == "Rand") return "R";
    if (name == "Lira" || name == "New Lira") return "₺";
    if (name == "Hryvnia") return "₴";
    if (name == "Renminbi") return "¥";
    return name;
}

std::string format_with_symbol(const std::string& amount, const std::string& symbol, bool before, bool space)
{
    const bool negative = !amount.empty() && amount[0] == '-';
    const std::string body = negative ? amount.substr(1) : amount;
    std::string formatted;
    if (symbol.empty())
    {
        formatted = body;
    }
    else if (before)
    {
        formatted = space ? symbol + " " + body : symbol + body;
    }
    else
    {
        formatted = space ? body + " " + symbol : body + symbol;
    }
    if (negative)
    {
        formatted.insert(0, "-");
    }
    return formatted;
}

ResolvedCurrency currency_from_info(const CurrencyInfo& info)
{
    ResolvedCurrency resolved;
    resolved.iso = info.iso;
    resolved.symbol = info.symbol;
    resolved.frac_digits = info.frac_digits;
    resolved.is_usd = std::string(info.iso) == "USD";
    resolved.known = true;
    resolved.per_usd = resolved.is_usd ? 1.0 : 0.0;
    resolved.symbol_before = info.symbol_before != 0;
    resolved.symbol_space = info.symbol_space == 1;

    if (info.symbol_before < 0 || info.symbol_space < 0)
    {
        const LLNumberFormat format = LLResMgr::getInstance()->getNumberFormat();
        if (format.position_known && format.iso_currency == info.iso)
        {
            if (info.symbol_before < 0)
            {
                resolved.symbol_before = format.symbol_before;
            }
            if (info.symbol_space < 0)
            {
                resolved.symbol_space = format.symbol_spaced;
            }
        }
    }
    return resolved;
}

ResolvedCurrency usd_currency()
{
    return currency_from_info(CURRENCIES[0]);
}

ResolvedCurrency resolve_local_currency(
    const std::map<std::string, std::pair<F64, std::string>>& rates,
    bool rates_ready)
{
    std::string override_text = gSavedSettings.getString("LocalCurrencyOverride");
    LLStringUtil::trim(override_text);

    const CurrencyInfo* info = nullptr;
    if (!override_text.empty())
    {
        std::string upper = override_text;
        LLStringUtil::toUpper(upper);
        std::string lower = override_text;
        LLStringUtil::toLower(lower);
        if (upper == "USD" || lower == "us dollar" || lower == "u.s. dollar" || lower == "united states-dollar")
        {
            info = &CURRENCIES[0];
        }
        else
        {
            info = find_currency_iso(upper);
        }
        if (!info)
        {
            info = find_currency_treasury(lower);
        }
        if (!info)
        {
            if (const char* preferred = preferred_treasury_word(lower))
            {
                std::string preferred_lower = preferred;
                LLStringUtil::toLower(preferred_lower);
                info = find_currency_treasury(preferred_lower);
            }
        }
        if (!info)
        {
            const auto found = rates.find(lower);
            if (found != rates.end() && found->second.first > 0.0)
            {
                ResolvedCurrency direct;
                direct.iso = override_text;
                direct.symbol = symbol_for_currency_name(found->second.second);
                direct.per_usd = found->second.first;
                direct.frac_digits = 2;
                direct.symbol_before = direct.symbol.size() <= 2;
                direct.symbol_space = !direct.symbol_before;
                direct.known = true;
                return direct;
            }
            if (!rates_ready)
            {
                ResolvedCurrency waiting;
                waiting.iso = override_text;
                waiting.known = true;
                return waiting;
            }
            LL_WARNS_ONCE("StatusBar") << "LocalCurrencyOverride '" << override_text
                << "' is not a known currency; using the system locale" << LL_ENDL;
        }
    }

    if (!info)
    {
        const LLNumberFormat format = LLResMgr::getInstance()->getNumberFormat();
        if (format.iso_currency.size() == 3)
        {
            info = find_currency_iso(format.iso_currency);
        }
        if (!info)
        {
            const std::string region = region_from_locale(format.locale_name);
            if (const char* iso = currency_for_region(region))
            {
                info = find_currency_iso(iso);
            }
        }
        if (!info)
        {
            if (const char* iso = currency_for_language(LLUI::getLanguage()))
            {
                info = find_currency_iso(iso);
            }
        }
    }
    if (!info)
    {
        info = &CURRENCIES[0];
    }

    ResolvedCurrency resolved = currency_from_info(*info);
    if (!resolved.is_usd && info->treasury)
    {
        std::string key = info->treasury;
        LLStringUtil::toLower(key);
        const auto found = rates.find(key);
        if (found != rates.end() && found->second.first > 0.0)
        {
            resolved.per_usd = found->second.first;
        }
    }

    static std::string announced;
    if (announced != resolved.iso)
    {
        announced = resolved.iso;
        LL_INFOS("StatusBar") << "Status bar currency is " << resolved.iso
            << " (" << resolved.symbol << ")" << LL_ENDL;
    }
    return resolved;
}

} // namespace

void LLStatusBar::setBalance(S32 balance)
{
    if (balance > getBalance() && getBalance() != 0)
    {
        LLFirstUse::receiveLindens();
    }

    const bool show_local = gSavedSettings.getBOOL("ShowLocalCurrency");
    ResolvedCurrency currency = show_local ? resolve_local_currency(mTreasuryRates, mTreasuryReady) : ResolvedCurrency();
    std::string fallback_note;
    if (show_local && !currency.is_usd && mTreasuryReady && currency.per_usd <= 0.0)
    {
        fallback_note = "No Treasury rate for " + currency.iso + "; showing US dollars.";
        currency = usd_currency();
    }
    const bool can_convert = show_local && mExchangeRate > 0.0 && currency.per_usd > 0.0;

    LLResMgr* resources = LLResMgr::getInstance();
    std::string amount;
    std::string fiat_amount;
    if (mObscureBalance)
    {
        amount = "****";
        if (show_local && !currency.symbol.empty())
        {
            fiat_amount = "****";
        }
    }
    else
    {
        amount = resources->formatNumber(static_cast<F64>(balance), 0);
        if (can_convert)
        {
            const F64 local = (static_cast<F64>(balance) / mExchangeRate) * currency.per_usd;
            fiat_amount = resources->formatNumber(local, currency.frac_digits);
        }
    }

    std::string label = amount + " L$";
    if (!fiat_amount.empty())
    {
        label += " / ";
        label += format_with_symbol(fiat_amount, currency.symbol, currency.symbol_before, currency.symbol_space);
    }
    mBoxBalance->setValue(label);

    std::string tip;
    if (mObscureBalance)
    {
        tip = "Balance hidden.";
    }
    else
    {
        tip = label;
        if (show_local && mExchangeRate > 0.0)
        {
            tip += llformat("\nLindeX: L$ %.2f per USD", mExchangeRate);
            if (can_convert && !currency.is_usd && !mTreasuryRecordDate.empty())
            {
                tip += llformat("\nTreasury: 1 USD = %.4f %s (%s)",
                    currency.per_usd, currency.iso.c_str(), mTreasuryRecordDate.c_str());
            }
        }
        if (!fallback_note.empty())
        {
            tip += "\n" + fallback_note;
        }
    }
    tip += "\nClick to refresh your L$ balance.\nDouble-click to display or hide your L$ balance.\nLocal currency is set in Preferences > General.";
    mBoxBalance->setToolTip(tip);

    updateBalancePanelPosition();

    // If the search panel is shown, move this according to the new balance width. Parcel text will reshape itself in setParcelInfoText
    if (mSearchPanel && mSearchPanel->getVisible())
    {
        updateMenuSearchPosition();
    }

    if (mBalance && (fabs((F32)(mBalance - balance)) > gSavedSettings.getF32("UISndMoneyChangeThreshold")))
    {
        if (mBalance > balance)
            make_ui_sound("UISndMoneyChangeDown");
        else
            make_ui_sound("UISndMoneyChangeUp");
    }

    if( balance != mBalance )
    {
        mBalance = balance;
    }
}


// static
void LLStatusBar::sendMoneyBalanceRequest()
{
    LLMessageSystem* msg = gMessageSystem;
    msg->newMessageFast(_PREHASH_MoneyBalanceRequest);
    msg->nextBlockFast(_PREHASH_AgentData);
    msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
    msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
    msg->nextBlockFast(_PREHASH_MoneyData);
    msg->addUUIDFast(_PREHASH_TransactionID, LLUUID::null );

    if (gDisconnected)
    {
        LL_DEBUGS() << "Trying to send message when disconnected, skipping balance request!" << LL_ENDL;
        return;
    }
    if (!gAgent.getRegion())
    {
        LL_DEBUGS() << "LLAgent::sendReliableMessage No region for agent yet, skipping balance request!" << LL_ENDL;
        return;
    }
    // Double amount of retries due to this request initially happening during busy stage
    // Ideally this should be turned into a capability
    gMessageSystem->sendReliable(gAgent.getRegionHost(), LL_DEFAULT_RELIABLE_RETRIES * 2, true, LL_PING_BASED_TIMEOUT_DUMMY, NULL, NULL);
}


void LLStatusBar::setHealth(S32 health)
{
    //LL_INFOS() << "Setting health to: " << buffer << LL_ENDL;
    if( mHealth > health )
    {
        if (mHealth > (health + gSavedSettings.getF32("UISndHealthReductionThreshold")))
        {
            if (isAgentAvatarValid())
            {
                if (gAgentAvatarp->getSex() == SEX_FEMALE)
                {
                    make_ui_sound("UISndHealthReductionF");
                }
                else
                {
                    make_ui_sound("UISndHealthReductionM");
                }
            }
        }
    }

    mHealth = health;
}

S32 LLStatusBar::getBalance() const
{
    return mBalance;
}

void LLStatusBar::setExchangeRate(F64 lindens_per_usd)
{
    if (lindens_per_usd <= 0.0)
    {
        return;
    }
    mExchangeRate = lindens_per_usd;
    setBalance(mBalance);
}

namespace
{

const char* const LINDEX_HOMEPAGE_URL = "http://secondlife.com/httprequest/homepage.php";

class ExchangeRateHandler : public LLCore::HttpHandler
{
public:
    void onCompleted(LLCore::HttpHandle handle, LLCore::HttpResponse* response) override
    {
        if (!gStatusBar)
        {
            return;
        }
        if (!response)
        {
            gStatusBar->finishExchangeRateRequest(0.0, "empty LindeX response");
            return;
        }

        const LLCore::HttpStatus status = response->getStatus();
        if (!status)
        {
            gStatusBar->finishExchangeRateRequest(0.0, status.toString());
            return;
        }

        std::string body;
        LLCore::BufferArray* raw = response->getBody();
        if (raw && raw->size() > 0)
        {
            body.resize(raw->size());
            raw->read(0, body.data(), body.size());
        }

        std::istringstream lines(body);
        std::string line;
        std::string previous;
        F64 rate = 0.0;
        while (std::getline(lines, line))
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            if (previous == "exchange_rate")
            {
                rate = std::strtod(line.c_str(), nullptr);
                break;
            }
            previous = line;
        }
        if (rate > 0.0)
        {
            gStatusBar->finishExchangeRateRequest(rate, std::string());
        }
        else
        {
            gStatusBar->finishExchangeRateRequest(0.0, "LindeX feed did not contain an exchange rate");
        }
    }
};

} // namespace

void LLStatusBar::finishExchangeRateRequest(F64 lindens_per_usd, const std::string& error)
{
    mExchangeRateFetchInFlight = false;
    if (!error.empty())
    {
        LL_WARNS("StatusBar") << "LindeX rate request failed: " << error << LL_ENDL;
        return;
    }
    setExchangeRate(lindens_per_usd);
}

namespace
{

const char* const TREASURY_RATES_URL =
    "https://api.fiscaldata.treasury.gov/services/api/fiscal_service/v1/accounting/od/rates_of_exchange"
    "?fields=country_currency_desc,exchange_rate,record_date,currency"
    "&sort=-record_date&page%5Bsize%5D=300";

class TreasuryRateHandler : public LLCore::HttpHandler
{
public:
    void onCompleted(LLCore::HttpHandle handle, LLCore::HttpResponse* response) override
    {
        if (!gStatusBar)
        {
            return;
        }
        if (!response)
        {
            gStatusBar->finishTreasuryRequest(false, std::string(), {}, "empty Treasury response");
            return;
        }
        const LLCore::HttpStatus status = response->getStatus();
        if (!status)
        {
            gStatusBar->finishTreasuryRequest(false, std::string(), {}, status.toString());
            return;
        }

        std::string body;
        LLCore::BufferArray* raw = response->getBody();
        if (raw && raw->size() > 0)
        {
            body.resize(raw->size());
            raw->read(0, body.data(), body.size());
        }

        boost::system::error_code parse_error;
        const boost::json::value parsed = boost::json::parse(body, parse_error);
        if (parse_error)
        {
            gStatusBar->finishTreasuryRequest(false, std::string(), {}, "Treasury rate response was not JSON");
            return;
        }

        const LLSD payload = LlsdFromJson(parsed);
        const LLSD rows = payload["data"];
        if (!rows.isArray() || rows.size() == 0)
        {
            gStatusBar->finishTreasuryRequest(false, std::string(), {}, "Treasury rate response had no rows");
            return;
        }

        const std::string newest = rows[0]["record_date"].asString();
        std::map<std::string, std::pair<F64, std::string>> rates;
        for (S32 index = 0; index < rows.size(); ++index)
        {
            const LLSD row = rows[index];
            if (row["record_date"].asString() != newest)
            {
                continue;
            }
            std::string description = row["country_currency_desc"].asString();
            LLStringUtil::trim(description);
            if (description.empty())
            {
                continue;
            }
            std::string key = description;
            LLStringUtil::toLower(key);
            if (rates.find(key) != rates.end())
            {
                continue;
            }

            F64 rate = 0.0;
            const LLSD rate_value = row["exchange_rate"];
            if (rate_value.isReal())
            {
                rate = rate_value.asReal();
            }
            else
            {
                rate = std::strtod(rate_value.asString().c_str(), nullptr);
            }
            if (rate <= 0.0)
            {
                continue;
            }
            rates[key] = std::make_pair(rate, row["currency"].asString());
        }
        if (rates.empty() || newest.empty())
        {
            gStatusBar->finishTreasuryRequest(false, std::string(), {}, "Treasury rate response had no usable rates");
            return;
        }
        gStatusBar->finishTreasuryRequest(true, newest, rates, std::string());
    }
};

} // namespace

void LLStatusBar::finishTreasuryRequest(
    bool ok,
    const std::string& record_date,
    const std::map<std::string, std::pair<F64, std::string>>& rates,
    const std::string& error)
{
    mTreasuryFetchInFlight = false;
    mTreasuryTimer.reset();
    if (!ok)
    {
        LL_WARNS("StatusBar") << "Treasury rate request failed: " << error << LL_ENDL;
        setBalance(mBalance);
        return;
    }

    mTreasuryRates = rates;
    mTreasuryRecordDate = record_date;
    mTreasuryReady = true;
    LL_INFOS("StatusBar") << "Treasury rates " << record_date << " (" << rates.size() << " currencies)" << LL_ENDL;
    setBalance(mBalance);
}

void LLStatusBar::requestTreasuryRates()
{
    if (mTreasuryFetchInFlight || !gSavedSettings.getBOOL("ShowLocalCurrency"))
    {
        return;
    }
    if (!mTreasuryRequest)
    {
        mTreasuryRequest = new LLCore::HttpRequest();
    }

    LLCore::HttpOptions::ptr_t httpOpts = std::make_shared<LLCore::HttpOptions>();
    httpOpts->setFollowRedirects(true);
    httpOpts->setTimeout(30);

    const LLCore::HttpHandle handle = mTreasuryRequest->requestGet(
        LLCore::HttpRequest::DEFAULT_POLICY_ID,
        TREASURY_RATES_URL,
        httpOpts,
        LLCore::HttpHeaders::ptr_t(),
        std::make_shared<TreasuryRateHandler>());
    if (handle == LLCORE_HTTP_HANDLE_INVALID)
    {
        LL_WARNS("StatusBar") << "Treasury rate request was not queued: "
            << mTreasuryRequest->getStatus().toString() << LL_ENDL;
        mTreasuryTimer.reset();
        return;
    }

    mTreasuryFetchInFlight = true;
    mTreasuryTimer.reset();
}

void LLStatusBar::requestExchangeRate()
{
    if (mExchangeRateFetchInFlight)
    {
        return;
    }
    if (!mExchangeRateRequest)
    {
        mExchangeRateRequest = new LLCore::HttpRequest();
    }

    LLCore::HttpOptions::ptr_t httpOpts = std::make_shared<LLCore::HttpOptions>();
    httpOpts->setFollowRedirects(true);
    httpOpts->setTimeout(30);

    const LLCore::HttpHandle handle = mExchangeRateRequest->requestGet(
        LLCore::HttpRequest::DEFAULT_POLICY_ID,
        LINDEX_HOMEPAGE_URL,
        httpOpts,
        LLCore::HttpHeaders::ptr_t(),
        std::make_shared<ExchangeRateHandler>());
    if (handle == LLCORE_HTTP_HANDLE_INVALID)
    {
        LL_WARNS("StatusBar") << "LindeX rate request was not queued: "
            << mExchangeRateRequest->getStatus().toString() << LL_ENDL;
        mExchangeRateTimer.reset();
        return;
    }

    mExchangeRateFetchInFlight = true;
    mExchangeRateTimer.reset();
}


S32 LLStatusBar::getHealth() const
{
    return mHealth;
}

void LLStatusBar::setLandCredit(S32 credit)
{
    mSquareMetersCredit = credit;
}
void LLStatusBar::setLandCommitted(S32 committed)
{
    mSquareMetersCommitted = committed;
}

bool LLStatusBar::isUserTiered() const
{
    return (mSquareMetersCredit > 0);
}

S32 LLStatusBar::getSquareMetersCredit() const
{
    return mSquareMetersCredit;
}

S32 LLStatusBar::getSquareMetersCommitted() const
{
    return mSquareMetersCommitted;
}

S32 LLStatusBar::getSquareMetersLeft() const
{
    return mSquareMetersCredit - mSquareMetersCommitted;
}

void LLStatusBar::onClickBuyCurrency()
{
    // open a currency floater - actual one open depends on
    // value specified in settings.xml
    LLBuyCurrencyHTML::openCurrencyFloater();
    LLFirstUse::receiveLindens(false);
}

void LLStatusBar::onClickShop()
{
    LLFloaterReg::showInstanceOrBringToFront("marketplace");
    if (LLFloaterMarketplace* marketplace = LLFloaterReg::getTypedInstance<LLFloaterMarketplace>("marketplace"))
    {
        marketplace->openMarketplace();
    }
}

void LLStatusBar::onMouseEnterPresetsCamera()
{
    LLView* popup_holder = gViewerWindow->getRootView()->getChildView("popup_holder");
    LLIconCtrl* icon =  getChild<LLIconCtrl>( "presets_icon_camera" );
    LLRect icon_rect = icon->getRect();
    LLRect pulldown_rect = mPanelPresetsCameraPulldown->getRect();
    pulldown_rect.setLeftTopAndSize(icon_rect.mLeft -
         (pulldown_rect.getWidth() - icon_rect.getWidth()),
                   icon_rect.mBottom,
                   pulldown_rect.getWidth(),
                   pulldown_rect.getHeight());

    pulldown_rect.translate(popup_holder->getRect().getWidth() - pulldown_rect.mRight, 0);
    mPanelPresetsCameraPulldown->setShape(pulldown_rect);

    // show the master presets pull-down
    LLUI::getInstance()->clearPopups();
    LLUI::getInstance()->addPopup(mPanelPresetsCameraPulldown);
    mPanelNearByMedia->setVisible(false);
    mPanelVolumePulldown->setVisible(false);
    mPanelPresetsPulldown->setVisible(false);
    mPanelPresetsCameraPulldown->setVisible(true);
}

void LLStatusBar::onMouseEnterPresets()
{
    LLView* popup_holder = gViewerWindow->getRootView()->getChildView("popup_holder");
    LLIconCtrl* icon =  getChild<LLIconCtrl>( "presets_icon_graphic" );
    LLRect icon_rect = icon->getRect();
    LLRect pulldown_rect = mPanelPresetsPulldown->getRect();
    pulldown_rect.setLeftTopAndSize(icon_rect.mLeft -
         (pulldown_rect.getWidth() - icon_rect.getWidth()),
                   icon_rect.mBottom,
                   pulldown_rect.getWidth(),
                   pulldown_rect.getHeight());

    pulldown_rect.translate(popup_holder->getRect().getWidth() - pulldown_rect.mRight, 0);
    mPanelPresetsPulldown->setShape(pulldown_rect);

    // show the master presets pull-down
    LLUI::getInstance()->clearPopups();
    LLUI::getInstance()->addPopup(mPanelPresetsPulldown);
    mPanelNearByMedia->setVisible(false);
    mPanelVolumePulldown->setVisible(false);
    mPanelPresetsPulldown->setVisible(true);
}

void LLStatusBar::onMouseEnterVolume()
{
    LLView* popup_holder = gViewerWindow->getRootView()->getChildView("popup_holder");
    LLButton* volbtn =  getChild<LLButton>( "volume_btn" );
    LLRect vol_btn_rect = volbtn->getRect();
    LLRect volume_pulldown_rect = mPanelVolumePulldown->getRect();
    volume_pulldown_rect.setLeftTopAndSize(vol_btn_rect.mLeft -
         (volume_pulldown_rect.getWidth() - vol_btn_rect.getWidth()),
                   vol_btn_rect.mBottom,
                   volume_pulldown_rect.getWidth(),
                   volume_pulldown_rect.getHeight());

    volume_pulldown_rect.translate(popup_holder->getRect().getWidth() - volume_pulldown_rect.mRight, 0);
    mPanelVolumePulldown->setShape(volume_pulldown_rect);


    // show the master volume pull-down
    LLUI::getInstance()->clearPopups();
    LLUI::getInstance()->addPopup(mPanelVolumePulldown);
    mPanelPresetsCameraPulldown->setVisible(false);
    mPanelPresetsPulldown->setVisible(false);
    mPanelNearByMedia->setVisible(false);
    mPanelVolumePulldown->setVisible(true);
}

void LLStatusBar::onMouseEnterNearbyMedia()
{
    LLView* popup_holder = gViewerWindow->getRootView()->getChildView("popup_holder");
    LLRect nearby_media_rect = mPanelNearByMedia->getRect();
    LLButton* nearby_media_btn =  getChild<LLButton>( "media_toggle_btn" );
    LLRect nearby_media_btn_rect = nearby_media_btn->getRect();
    nearby_media_rect.setLeftTopAndSize(nearby_media_btn_rect.mLeft -
                                        (nearby_media_rect.getWidth() - nearby_media_btn_rect.getWidth())/2,
                                        nearby_media_btn_rect.mBottom,
                                        nearby_media_rect.getWidth(),
                                        nearby_media_rect.getHeight());
    // force onscreen
    nearby_media_rect.translate(popup_holder->getRect().getWidth() - nearby_media_rect.mRight, 0);

    // show the master volume pull-down
    mPanelNearByMedia->setShape(nearby_media_rect);
    LLUI::getInstance()->clearPopups();
    LLUI::getInstance()->addPopup(mPanelNearByMedia);

    mPanelPresetsCameraPulldown->setVisible(false);
    mPanelPresetsPulldown->setVisible(false);
    mPanelVolumePulldown->setVisible(false);
    mPanelNearByMedia->setVisible(true);
}


static void onClickVolume(void* data)
{
    // toggle the master mute setting
    bool mute_audio = LLAppViewer::instance()->getMasterSystemAudioMute();
    LLAppViewer::instance()->setMasterSystemAudioMute(!mute_audio);
}

//static
void LLStatusBar::onClickRefreshBalance(void* data)
{
    LLStatusBar* status_bar = (LLStatusBar*)data;

    if (!status_bar->mBalanceClicked)
    {
        // Schedule a balance request message:
        status_bar->mBalanceClicked = true;
        status_bar->mBalanceClickTimer.reset();
    }
    // The refresh of the display (call to setBalance()) will be done by process_money_balance_reply()
}

void LLStatusBar::onClickToggleBalance()
{
    mObscureBalance = !mObscureBalance;
    gSavedSettings.setBOOL("ObscureBalanceInStatusBar", mObscureBalance);
    setBalance(mBalance);
    mBalanceClicked = false; // supress click
}

//static
void LLStatusBar::onClickMediaToggle(void* data)
{
    LLStatusBar *status_bar = (LLStatusBar*)data;
    // "Selected" means it was showing the "play" icon (so media was playing), and now it shows "pause", so turn off media
    bool pause = status_bar->mMediaToggle->getValue();
    LLViewerMedia::getInstance()->setAllMediaPaused(pause);
}

//static
void LLStatusBar::onClickFps(void* data)
{
    LLFloaterReg::toggleInstance("mpv_performance");
}

bool can_afford_transaction(S32 cost)
{
    return((cost <= 0)||((gStatusBar) && (gStatusBar->getBalance() >=cost)));
}

void LLStatusBar::onVolumeChanged(const LLSD& newvalue)
{
    refresh();
}

void LLStatusBar::onVoiceChanged(const LLSD& newvalue)
{
    if (newvalue.asBoolean())
    {
        // Second instance starts with "VoiceMute_Off" icon, fix it
        mBtnVolume->setImageUnselected(LLUI::getUIImage("Audio_Off"));
    }
    refresh();
}

void LLStatusBar::onObscureBalanceChanged(const LLSD& newvalue)
{
    mObscureBalance = newvalue.asBoolean();
    setBalance(mBalance);
}

void LLStatusBar::onLocalCurrencyChanged(const LLSD&)
{
    if (gSavedSettings.getBOOL("ShowLocalCurrency"))
    {
        if (mExchangeRate <= 0.0)
        {
            requestExchangeRate();
        }
        if (!mTreasuryReady)
        {
            requestTreasuryRates();
        }
    }
    setBalance(mBalance);
}

void LLStatusBar::onUpdateFilterTerm()
{
    LLWString searchValue = utf8str_to_wstring( mFilterEdit->getValue() );
    LLWStringUtil::toLower( searchValue );

    if( !mSearchData || mSearchData->mLastFilter == searchValue )
        return;

    mSearchData->mLastFilter = searchValue;

    mSearchData->mRootMenu->hightlightAndHide( searchValue );
    gMenuBarView->needsArrange();
}

void collectChildren( LLMenuGL *aMenu, ll::statusbar::SearchableItemPtr aParentMenu )
{
    for( U32 i = 0; i < aMenu->getItemCount(); ++i )
    {
        LLMenuItemGL *pMenu = aMenu->getItem( i );

        ll::statusbar::SearchableItemPtr pItem = std::make_shared<ll::statusbar::SearchableItem>();
        pItem->mCtrl = pMenu;
        pItem->mMenu = pMenu;
        pItem->mLabel = utf8str_to_wstring( pMenu->ll::ui::SearchableControl::getSearchText() );
        LLWStringUtil::toLower( pItem->mLabel );
        aParentMenu->mChildren.push_back( pItem );

        LLMenuItemBranchGL *pBranch = dynamic_cast< LLMenuItemBranchGL* >( pMenu );
        if( pBranch )
            collectChildren( pBranch->getBranch(), pItem );
    }

}

void LLStatusBar::collectSearchableItems()
{
    mSearchData = std::make_unique<ll::statusbar::SearchData>();
    ll::statusbar::SearchableItemPtr pItem = std::make_shared<ll::statusbar::SearchableItem>();
    mSearchData->mRootMenu = pItem;
    collectChildren( gMenuBarView, pItem );
}

void LLStatusBar::updateMenuSearchVisibility(const LLSD& data)
{
    bool visible = data.asBoolean();
    mSearchPanel->setVisible(visible);
    if (!visible)
    {
        mFilterEdit->setText(LLStringUtil::null);
        onUpdateFilterTerm();
    }
    else
    {
        updateMenuSearchPosition();
    }
}

void LLStatusBar::updateMenuSearchPosition()
{
    const S32 HPAD = 12;
    LLRect balanceRect = getChildView("balance_bg")->getRect();
    LLRect searchRect = mSearchPanel->getRect();
    S32 w = searchRect.getWidth();
    searchRect.mLeft = balanceRect.mLeft - w - HPAD;
    searchRect.mRight = searchRect.mLeft + w;
    mSearchPanel->setShape( searchRect );
}

void LLStatusBar::updateBalancePanelPosition()
{
    // Resize the L$ balance background to fit a localized amount plus a
    // currency symbol. The text rect can be narrower than the string.
    const S32 HPAD = 24;
    S32 balance_width = mBoxBalance->getTextBoundingRect().getWidth();
    if (const LLFontGL* font = mBoxBalance->getFont())
    {
        balance_width = llmax(balance_width, font->getWidth(mBoxBalance->getValue().asString()) + 12);
    }
    LLRect buy_rect = getChildView("buyL")->getRect();
    LLRect shop_rect = getChildView("goShop")->getRect();
    LLView* balance_bg_view = getChildView("balance_bg");
    LLRect balance_bg_rect = balance_bg_view->getRect();
    balance_bg_rect.mLeft = balance_bg_rect.mRight - (buy_rect.getWidth() + shop_rect.getWidth() + balance_width + HPAD);
    balance_bg_view->setShape(balance_bg_rect);
}

void LLStatusBar::setBalanceVisible(bool visible)
{
    mBoxBalance->setVisible(visible);
}

// Implements secondlife:///app/balance/request to request a L$ balance
// update via UDP message system. JC
class LLBalanceHandler : public LLCommandHandler
{
public:
    // Requires "trusted" browser/URL source
    LLBalanceHandler() : LLCommandHandler("balance", UNTRUSTED_BLOCK) { }
    bool handle(const LLSD& tokens, const LLSD& query_map, const std::string& grid, LLMediaCtrl* web)
    {
        if (tokens.size() == 1
            && tokens[0].asString() == "request")
        {
            LLStatusBar::sendMoneyBalanceRequest();
            return true;
        }
        return false;
    }
};
// register with command dispatch system
LLBalanceHandler gBalanceHandler;
