/**
 * @file llresmgr.cpp
 * @brief Localized resource manager
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

// NOTE: this is a MINIMAL implementation.  The interface will remain, but the implementation will
// (when the time is right) become dynamic and probably use external files.

#include "linden_common.h"

#include "llresmgr.h"
#include "llfontgl.h"
#include "llerror.h"
#include "llstring.h"

#include <climits>
#include <vector>

#if LL_DARWIN
#include <CoreFoundation/CoreFoundation.h>
#endif
#if LL_WINDOWS
#include "llwin32headers.h"
#endif


LLResMgr::LLResMgr()
{
    // Set default
    setLocale( LLLOCALE_USA );
}


void LLResMgr::setLocale( LLLOCALE_ID locale_id )
{
    mLocale = locale_id;
}

char LLResMgr::getDecimalPoint() const
{
    char decimal = localeconv()->decimal_point[0];

    return decimal;
}

std::string LLResMgr::getThousandsSeparator() const
{
    return localeconv()->thousands_sep;
}

char LLResMgr::getMonetaryDecimalPoint() const
{
    char decimal = localeconv()->mon_decimal_point[0];

    return decimal;
}

std::string LLResMgr::getMonetaryThousandsSeparator() const
{
    return localeconv()->mon_thousands_sep;
}


namespace
{

struct LocaleChoice
{
    std::string name;
    std::string currency;
};

#if LL_DARWIN
LocaleChoice darwin_locale()
{
    LocaleChoice choice;
    CFLocaleRef locale = CFLocaleCopyCurrent();
    if (!locale)
    {
        return choice;
    }

    char buffer[128];
    CFStringRef identifier = CFLocaleGetIdentifier(locale);
    if (identifier && CFStringGetCString(identifier, buffer, sizeof(buffer), kCFStringEncodingUTF8))
    {
        choice.name = buffer;
    }
    CFStringRef currency = (CFStringRef)CFLocaleGetValue(locale, kCFLocaleCurrencyCode);
    if (currency && CFStringGetCString(currency, buffer, sizeof(buffer), kCFStringEncodingUTF8))
    {
        choice.currency = buffer;
    }
    CFRelease(locale);

    const std::string::size_type variant = choice.name.find('@');
    if (variant != std::string::npos)
    {
        choice.name.erase(variant);
    }
    for (char& character : choice.name)
    {
        if (character == '-')
        {
            character = '_';
        }
    }
    LLStringUtil::toUpper(choice.currency);
    return choice;
}
#endif

#if LL_WINDOWS
std::string windows_currency_code()
{
    wchar_t buffer[16];
    buffer[0] = 0;
    if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SINTLSYMBOL, buffer, 16) <= 0)
    {
        return std::string();
    }
    std::string code;
    for (int index = 0; buffer[index] && index < 8; ++index)
    {
        code.push_back(static_cast<char>(buffer[index]));
    }
    LLStringUtil::trim(code);
    LLStringUtil::toUpper(code);
    return code;
}
#endif

// USER_LOCALE is hard-coded to en_US. On macOS that draws German balances
// with US commas. On Linux the name is en_US.utf8, which often fails, and
// the fallback "C" locale has no thousands separator. Prefer the real
// locale: CoreFoundation on macOS, the user default on Windows, and the
// environment locale ("") everywhere else.
LocaleChoice choose_locale()
{
    static LocaleChoice cached;
    static bool done = false;
    if (done)
    {
        return cached;
    }
    done = true;

    const char* previous = setlocale(LC_ALL, nullptr);
    const std::string saved = previous ? previous : "";

    std::vector<std::string> tries;
#if LL_DARWIN
    const LocaleChoice darwin = darwin_locale();
    cached.currency = darwin.currency;
    if (!darwin.name.empty())
    {
        tries.push_back(darwin.name);
        if (darwin.name.find('.') == std::string::npos)
        {
            tries.push_back(darwin.name + ".UTF-8");
        }
        const std::string::size_type first = darwin.name.find('_');
        const std::string::size_type last = darwin.name.rfind('_');
        if (first != std::string::npos && last != std::string::npos && first != last)
        {
            tries.push_back(darwin.name.substr(0, first) + "_" + darwin.name.substr(last + 1));
        }
    }
#elif LL_WINDOWS
    cached.currency = windows_currency_code();
#endif
    tries.push_back("");
    tries.push_back(LLLocale::USER_LOCALE);

    for (size_t index = 0; index < tries.size(); ++index)
    {
        char* got = setlocale(LC_ALL, tries[index].c_str());
        if (!got)
        {
            continue;
        }
        const std::string resolved = got;
        if (resolved != "C" && resolved != "POSIX")
        {
            cached.name = resolved;
            break;
        }
    }
    if (cached.name.empty())
    {
        cached.name = LLLocale::USER_LOCALE;
    }
    if (!saved.empty())
    {
        setlocale(LC_ALL, saved.c_str());
    }
    return cached;
}

// localeconv grouping is a byte string of group sizes, not the
// semicolon-separated text in locale files. A zero byte or the end of the
// string repeats the previous size. CHAR_MAX stops grouping.
std::string group_digits(const std::string& digits, const std::string& separator, const std::string& grouping)
{
    if (grouping.empty() || static_cast<unsigned char>(grouping[0]) == static_cast<unsigned char>(CHAR_MAX))
    {
        return digits;
    }

    S32 sizes[8];
    S32 count = 0;
    for (size_t index = 0; index < grouping.size() && count < 8; ++index)
    {
        const unsigned char byte = static_cast<unsigned char>(grouping[index]);
        if (byte == 0 || byte == static_cast<unsigned char>(CHAR_MAX))
        {
            break;
        }
        sizes[count++] = static_cast<S32>(byte);
    }
    if (count == 0)
    {
        sizes[0] = 3;
        count = 1;
    }

    std::string formatted;
    S32 pos = static_cast<S32>(digits.size());
    S32 group_index = 0;
    while (pos > 0)
    {
        S32 size = sizes[group_index];
        if (size <= 0)
        {
            break;
        }
        if (size > pos)
        {
            size = pos;
        }
        if (!formatted.empty() && !separator.empty())
        {
            formatted = separator + formatted;
        }
        formatted = digits.substr(static_cast<size_t>(pos - size), static_cast<size_t>(size)) + formatted;
        pos -= size;
        if (group_index + 1 < count)
        {
            ++group_index;
        }
    }
    return formatted;
}

std::string first_nonempty(const char* first, const char* second)
{
    if (first && first[0])
    {
        return first;
    }
    if (second && second[0])
    {
        return second;
    }
    return std::string();
}

} // namespace

LLNumberFormat LLResMgr::getNumberFormat() const
{
    static LLNumberFormat cached;
    static bool ready = false;
    if (ready)
    {
        return cached;
    }

    const LocaleChoice choice = choose_locale();
    cached.locale_name = choice.name;
    {
        // Read localeconv while this locale is in effect, then copy the
        // strings out. The pointers are invalid after the locale changes.
        LLLocale locale(choice.name);
        struct lconv* conv = localeconv();

        cached.thousands_sep = first_nonempty(conv->mon_thousands_sep, conv->thousands_sep);
        cached.decimal_point = first_nonempty(conv->mon_decimal_point, conv->decimal_point);
        if (cached.decimal_point.empty())
        {
            cached.decimal_point = ".";
        }

        const char* grouping = nullptr;
        if (conv->mon_grouping && conv->mon_grouping[0])
        {
            grouping = conv->mon_grouping;
        }
        else if (conv->grouping && conv->grouping[0])
        {
            grouping = conv->grouping;
        }
        if (!grouping || !grouping[0])
        {
            // Linux often leaves mon_grouping empty, and the C locale
            // leaves both empty, so thousands separators never appeared.
            cached.grouping = "\003";
        }
        else if (static_cast<unsigned char>(grouping[0]) == static_cast<unsigned char>(CHAR_MAX))
        {
            cached.grouping.clear();
        }
        else
        {
            cached.grouping = grouping;
        }
        if (cached.thousands_sep.empty() && !cached.grouping.empty())
        {
            // macOS German has sometimes reported an empty monetary
            // separator while the numeric separator (or the decimal
            // character) still describes the locale.
            cached.thousands_sep = (cached.decimal_point == ",") ? "." : ",";
        }

        cached.currency_symbol = conv->currency_symbol ? conv->currency_symbol : "";
        cached.iso_currency = conv->int_curr_symbol ? conv->int_curr_symbol : "";
        LLStringUtil::trim(cached.iso_currency);
        if (cached.iso_currency.size() > 3)
        {
            cached.iso_currency.resize(3);
        }
        LLStringUtil::toUpper(cached.iso_currency);

        const unsigned char precedes = static_cast<unsigned char>(conv->p_cs_precedes);
        cached.position_known = (precedes == 0 || precedes == 1);
        cached.symbol_before = precedes != 0;
        const unsigned char gap = static_cast<unsigned char>(conv->p_sep_by_space);
        cached.symbol_spaced = (gap == 1);
        const unsigned char fraction_digits = static_cast<unsigned char>(conv->frac_digits);
        cached.frac_digits = (fraction_digits <= 9) ? static_cast<S32>(fraction_digits) : -1;
    }
    if (choice.currency.size() == 3)
    {
        cached.iso_currency = choice.currency;
    }

    ready = true;
    LL_INFOS_ONCE("LLResMgr") << "Numeric locale '" << cached.locale_name
        << "' currency " << cached.iso_currency
        << " decimal '" << cached.decimal_point << "'" << LL_ENDL;
    return cached;
}

std::string LLResMgr::formatNumber(F64 value, S32 frac_digits) const
{
    const LLNumberFormat format = getNumberFormat();
    if (frac_digits < 0)
    {
        frac_digits = 0;
    }
    if (frac_digits > 6)
    {
        frac_digits = 6;
    }

    F64 scale = 1.0;
    for (S32 digit = 0; digit < frac_digits; ++digit)
    {
        scale *= 10.0;
    }
    const F64 magnitude = value < 0.0 ? -value : value;
    const S64 scaled = static_cast<S64>(ll_round(magnitude * scale));
    const bool negative = value < 0.0 && scaled != 0;

    S64 whole = scaled;
    unsigned int fraction = 0;
    if (frac_digits > 0)
    {
        const S64 scale_int = static_cast<S64>(scale);
        whole = scaled / scale_int;
        fraction = static_cast<unsigned int>(scaled % scale_int);
    }

    const std::string digits = std::to_string(static_cast<long long>(whole));
    std::string formatted = format.grouping.empty()
        ? digits
        : group_digits(digits, format.thousands_sep, format.grouping);
    if (frac_digits > 0)
    {
        std::string fraction_text = std::to_string(fraction);
        while (static_cast<S32>(fraction_text.size()) < frac_digits)
        {
            fraction_text.insert(0, "0");
        }
        formatted += format.decimal_point;
        formatted += fraction_text;
    }
    if (negative)
    {
        formatted.insert(0, "-");
    }
    return formatted;
}

std::string LLResMgr::getMonetaryString(S32 input) const
{
    return formatNumber(static_cast<F64>(input), 0);
}

void LLResMgr::getIntegerString( std::string& output, S32 input ) const
{
    // handle special case of input value being zero
    if (input == 0)
    {
        output = "0";
        return;
    }

    // *NOTE: this method does not handle negative input integers correctly
    S32 fraction = 0;
    std::string fraction_string;
    S32 remaining_count = input;
    while(remaining_count > 0)
    {
        fraction = (remaining_count) % 1000;

        if (!output.empty())
        {
            if (fraction == remaining_count)
            {
                fraction_string = llformat_to_utf8("%d", fraction) + getThousandsSeparator();
            }
            else
            {
                fraction_string = llformat_to_utf8("%3.3d", fraction) + getThousandsSeparator();
            }
            output = fraction_string + output;
        }
        else
        {
            if (fraction == remaining_count)
            {
                fraction_string = llformat("%d", fraction);
            }
            else
            {
                fraction_string = llformat("%3.3d", fraction);
            }
            output = fraction_string;
        }
        remaining_count /= 1000;
    }
}

#if LL_WINDOWS
const std::string LLLocale::USER_LOCALE("English_United States.1252");// = LLStringUtil::null;
const std::string LLLocale::SYSTEM_LOCALE("English_United States.1252");
#elif LL_DARWIN
const std::string LLLocale::USER_LOCALE("en_US.iso8859-1");// = LLStringUtil::null;
const std::string LLLocale::SYSTEM_LOCALE("en_US.iso8859-1");
#else // LL_LINUX likes this
const std::string LLLocale::USER_LOCALE("en_US.utf8");
const std::string LLLocale::SYSTEM_LOCALE("C");
#endif


LLLocale::LLLocale(const std::string& locale_string)
{
    mPrevLocaleString = setlocale( LC_ALL, NULL );
    char* new_locale_string = setlocale( LC_ALL, locale_string.c_str());
    if ( new_locale_string == NULL)
    {
        LL_WARNS_ONCE("LLLocale") << "Failed to set locale " << locale_string << LL_ENDL;
        setlocale(LC_ALL, SYSTEM_LOCALE.c_str());
    }
    //else
    //{
    //  LL_INFOS() << "Set locale to " << new_locale_string << LL_ENDL;
    //}
}

LLLocale::~LLLocale()
{
    setlocale( LC_ALL, mPrevLocaleString.c_str() );
}
