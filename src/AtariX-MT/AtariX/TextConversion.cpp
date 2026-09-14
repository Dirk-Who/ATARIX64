/*
 * Copyright (C) 1990-2018 Andreas Kromke, andreas.kromke@gmail.com
 *
 * This program is free software; you can redistribute it or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
*
* Zeichensatz-Umsetzung für MagicMacX
*
*/

#include "config.h"
// System-Header
#include <CoreFoundation/CoreFoundation.h>
// Programm-Header
#include "Globals.h"
#include "resource.h"
#include "Debug.h"
#include "TextConversion.h"
#include "maptab.h"
#include <cerrno>
#include <cstring>
#include <string>

namespace {
bool filenameError(char *dst, size_t count, int error)
{
	if (dst && count) dst[0] = 0;
	errno = error;
	return false;
}

bool exactAtariCharacter(unsigned ch)
{
	return ch < 0x10000 && utf16_to_atari[ch] < 256 &&
		atari_to_utf16[utf16_to_atari[ch]] == ch;
}

bool readUtf8(const unsigned char *s, size_t len, size_t &pos, unsigned &ch)
{
	unsigned first = s[pos++];
	if (first < 0x80) { ch = first; return true; }
	unsigned extra, minimum;
	if (first >= 0xc2 && first <= 0xdf) { extra = 1; minimum = 0x80; ch = first & 31; }
	else if (first >= 0xe0 && first <= 0xef) { extra = 2; minimum = 0x800; ch = first & 15; }
	else if (first >= 0xf0 && first <= 0xf4) { extra = 3; minimum = 0x10000; ch = first & 7; }
	else return false;
	if (extra > len - pos) return false;
	while (extra--) {
		unsigned c = s[pos++];
		if ((c & 0xc0) != 0x80) return false;
		ch = (ch << 6) | (c & 63);
	}
	return ch >= minimum && ch <= 0x10ffff && !(ch >= 0xd800 && ch <= 0xdfff);
}

void appendUtf8(std::string &out, unsigned ch)
{
	if (ch < 0x80) out += char(ch);
	else {
		if (ch < 0x800) out += char(0xc0 | (ch >> 6));
		else {
			if (ch < 0x10000) out += char(0xe0 | (ch >> 12));
			else {
				out += char(0xf0 | (ch >> 18));
				out += char(0x80 | ((ch >> 12) & 63));
			}
			out += char(0x80 | ((ch >> 6) & 63));
		}
		out += char(0x80 | (ch & 63));
	}
}
}

// ~{HHHH} (4-6 hex digits) represents a Unicode scalar outside the
// Atari table. A literal "~{" is quoted as "~{007E}{". Ordinary ASCII
// (including DOS ~1 names) and Atari characters retain their encoding.
// Do not normalize: NFC and NFD may be distinct on a mapped filesystem.
bool CTextConversion::HostFilenameToAtari(char *dst, const char *src, size_t count)
{
	if (!dst || !src || !count) return filenameError(dst, count, EINVAL);
	const size_t len = strnlen(src, MAXPATHNAMELEN);
	if (len == MAXPATHNAMELEN) return filenameError(dst, count, ENAMETOOLONG);
	std::string out;
	for (size_t pos = 0; pos < len;) {
		unsigned ch;
		if (!readUtf8((const unsigned char *)src, len, pos, ch))
			return filenameError(dst, count, EILSEQ);
		if (exactAtariCharacter(ch) && !(ch == '~' && src[pos] == '{'))
			out += char(utf16_to_atari[ch]);
		else {
			char escaped[12];
			snprintf(escaped, sizeof(escaped), "~{%04X}", ch);
			out += escaped;
		}
		if (out.size() >= count) return filenameError(dst, count, ENAMETOOLONG);
	}
	memcpy(dst, out.c_str(), out.size() + 1);
	return true;
}

bool CTextConversion::AtariFilenameToHost(char *dst, const char *src, size_t count)
{
	if (!dst || !src || !count) return filenameError(dst, count, EINVAL);
	const size_t len = strnlen(src, MAXPATHNAMELEN);
	if (len == MAXPATHNAMELEN) return filenameError(dst, count, ENAMETOOLONG);
	std::string out;
	for (size_t pos = 0; pos < len;) {
		unsigned ch = atari_to_utf16[(unsigned char)src[pos++]];
		if (ch == '~' && src[pos] == '{') {
			++pos;
			ch = 0;
			unsigned digits = 0;
			while (pos < len && src[pos] != '}') {
				unsigned char c = src[pos++];
				unsigned value = c >= '0' && c <= '9' ? c - '0' :
					c >= 'A' && c <= 'F' ? c - 'A' + 10 :
					c >= 'a' && c <= 'f' ? c - 'a' + 10 : 16;
				if (value == 16 || ++digits > 6) return filenameError(dst, count, EILSEQ);
				ch = (ch << 4) | value;
			}
			if (pos == len || digits < 4 || ch > 0x10ffff ||
				(ch >= 0xd800 && ch <= 0xdfff)) return filenameError(dst, count, EILSEQ);
			++pos;
			// No alternate encodings of NUL, separators, dot, or regular
			// Atari bytes. This also prevents escape-based path traversal.
			if (exactAtariCharacter(ch) && !(ch == '~' && src[pos] == '{'))
				return filenameError(dst, count, EILSEQ);
		}
		appendUtf8(out, ch);
		if (out.size() >= count) return filenameError(dst, count, ENAMETOOLONG);
	}
	memcpy(dst, out.c_str(), out.size() + 1);
	return true;
}

// statische Attribute:



/**********************************************************************
*
* statisch: Initialisieren
*
**********************************************************************/

int CTextConversion::Init(void)
{
	return 0;
}


bool CTextConversion::Atari2HostUtf8Copy(char *dst, const char *src, size_t count)
{
	if (!dst || !src || count == 0)
		return false;
	while (*src)
	{
		const unsigned short ch = atari_to_utf16[(unsigned char)*src];
		const size_t bytes = ch < 0x80 ? 1 : ch < 0x800 ? 2 : 3;
		if (count <= bytes)
		{
			*dst = '\0';
			return false;
		}
		if (bytes == 1)
			*dst++ = ch;
		else if (bytes == 2)
		{
			*dst++ = ((ch >> 6) & 0x1f) | 0xc0;
			*dst++ = (ch & 0x3f) | 0x80;
		}
		else
		{
			*dst++ = ((ch >> 12) & 0x0f) | 0xe0;
			*dst++ = ((ch >> 6) & 0x3f) | 0x80;
			*dst++ = (ch & 0x3f) | 0x80;
		}
		++src;
		count -= bytes;
	}
	*dst = '\0';
	return true;
}


bool CTextConversion::Host2AtariUtf8Copy(char *dst, const char *src, size_t count)
{
	if (!dst || !src || count == 0)
		return false;
	*dst = '\0';
#ifdef __APPLE__
	/* MacOSX uses decomposed strings, normalize them first */
	CFMutableStringRef theString = CFStringCreateMutable(NULL, 0);
	CFStringAppendCString(theString, src, kCFStringEncodingUTF8);
	CFStringNormalize(theString, kCFStringNormalizationFormC);
	UniChar ch;
	unsigned short c;
	CFIndex idx;
	CFIndex len = CFStringGetLength(theString);
	
	idx = 0;
	while (count > 1 && idx < len)
	{
		ch = CFStringGetCharacterAtIndex(theString, idx);
		c = utf16_to_atari[ch];
		if (c >= 0x100)
		{
			const size_t needed = ch < 0x80 ? 1 : ch < 0x800 ? 2 : 3;
			if (count <= needed)
				break;
			charset_conv_error(ch);
			/* not convertible. return utf8-sequence to avoid producing duplicate filenames */
			if (ch < 0x80)
			{
				*dst++ = ch;
				count--;
			} else if (ch < 0x800)
			{
				*dst++ = ((ch >> 6) & 0x3f) | 0xc0;
				*dst++ = (ch & 0x3f) | 0x80;
				count -= 2;
			} else 
			{
				*dst++ = ((ch >> 12) & 0x0f) | 0xe0;
				*dst++ = ((ch >> 6) & 0x3f) | 0x80;
				*dst++ = (ch & 0x3f) | 0x80;
				count -= 3;
			}
		} else
		{
			*dst++ = c;
			count -= 1;
		}
		idx++;
	}
	if (count > 0)
	{
		*dst = 0;
	}
	CFRelease(theString);
	return idx == len;
#else
	unsigned short ch;
	unsigned short c;
	size_t bytes;
	
	while (count > 1 && *src)
	{
		c = (unsigned char) *src;
		ch = c;
		if (ch < 0x80)
		{
			bytes = 1;
		} else if ((ch & 0xe0) == 0xc0 || count < 3)
		{
			ch = ((ch & 0x1f) << 6) | (src[1] & 0x3f);
			bytes = 2;
		} else
		{
			ch = ((((ch & 0x0f) << 6) | (src[1] & 0x3f)) << 6) | (src[2] & 0x3f);
			bytes = 3;
		}
		c = utf16_to_atari[ch];
		if (c >= 0x100)
		{
			if (count <= bytes)
				break;
			charset_conv_error(ch);
			/* not convertible. return utf8-sequence to avoid producing duplicate filenames */
			*dst++ = *src++;
			if (bytes >= 2)
				*dst++ = *src++;
			if (bytes >= 3)
				*dst++ = *src++;
			count -= bytes;
		} else
		{
			*dst++ = c;
			src += bytes;
			count -= 1;
		}
	}
	if (count > 0)
	{
		*dst = 0;
	}
	return *src == '\0';
#endif
}

void CTextConversion::charset_conv_error(unsigned short ch)
{
	fprintf(stderr, "cannot convert $%04x to atari codeset\n", ch);
}
