// FileZilla Server - a Windows ftp server

// Copyright (C) 2004 - Tim Kosse <tim.kosse@gmx.de>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"
#include "iputils.h"
#include <boost/regex.hpp>

bool IsValidAddressFilter(CStdString& filter, bool allowWildcards /*=true*/)
{
	// Check for regular expression syntax (to match against the hostname).
	if (filter.Left(1) == _T("/") && filter.Right(1) == _T("/"))
	{
		if (filter.GetLength() < 3)
			return false;

#ifdef _UNICODE
		const CStdStringA localFilter = ConvToLocal(filter.Mid(1, filter.GetLength() - 2));
#else
		const CStdString localFilter = filter;
#endif
		try
		{
			boost::regex e(localFilter);
		}
		catch (std::runtime_error)
		{
			return false;
		}
		return true;
	}

	if (filter.Find(_T("..")) != -1)
		return false;

	// Check for IP/subnet syntax.
	TCHAR chr = '/';
	int pos = filter.Find(chr);
	if (pos == -1)
	{
		// Check for IP range syntax.
		chr = '-';
		pos = filter.Find(chr);
	}
	if (pos != -1)
	{
		// Only one slash or dash is allowed.
		CStdString right = filter.Mid(pos + 1);
		if (right.Find('/') != -1 || right.Find('-') != -1)
			return false;

		// When using IP/subnet or IP range syntax, no wildcards are allowed.
		if (!IsValidAddressFilter(right, false))
			return false;
		CStdString left = filter.Left(pos);
		if (!IsValidAddressFilter(left, false))
			return false;

		filter = left + chr + right;
	}
	else
	{
		int dotCount = 0;
		for (const TCHAR *cur = filter; *cur; cur++)
		{
			// Check for valid characters.
			if ((*cur < '0' || *cur > '9') && *cur != '.' && (!allowWildcards || (*cur != '*' && *cur != '?')))
				return false;
			else
			{
				// Count the number of contained dots.
				if (*cur == '.')
					dotCount++;
			}
		}
		if (dotCount != 3 || filter.Right(1) == _T(".") || filter.Left(1) == _T("."))
			return false;
	}

	// Merge redundant wildcards.
	while (filter.Replace(_T("?*"), _T("*")));
	while (filter.Replace(_T("*?"), _T("*")));
	while (filter.Replace(_T("**"), _T("*")));

	return true;
}

bool MatchesFilter(const CStdString& filter, unsigned int ip, LPCTSTR pIp)
{
	// ip and pIp are the same IP, one as number, the other one as string.

	// A single asterix matches all IPs.
	if (filter == _T("*"))
		return true;

	// Check for regular expression filter.
	if (filter.Left(1) == _T("/") && filter.Right(1) == _T("/"))
		return MatchesRegExp(filter.Mid(1, filter.GetLength() - 2), ntohl(ip));

	// Check for IP range syntax.
	int pos2 = filter.Find('-');
	if (pos2 != -1)
	{
#ifdef _UNICODE
		CStdStringA offset = ConvToLocal(filter.Left(pos2));
		CStdStringA range = ConvToLocal(filter.Mid(pos2 + 1));
#else
		CStdString offset = filter.Left(pos2);
		CStdString range = filter.Mid(pos2 + 1);
#endif
		if (offset == "" || range == "")
			return false;
		unsigned int min = htonl(inet_addr(offset));
		unsigned int max = htonl(inet_addr(range));
		return (ip >= min) && (ip <= max);
	}

	// Check for IP/subnet syntax.
	pos2 = filter.Find('/');
	if (pos2 != -1)
	{
#ifdef _UNICODE
		CStdStringA ipStr = ConvToLocal(filter.Left(pos2));
		CStdStringA subnet = ConvToLocal(filter.Mid(pos2 + 1));
#else
		CStdString ipStr = filter.Left(pos2);
		CStdString subnet = filter.Mid(pos2 + 1);
#endif
		if (ipStr == "" || subnet == "")
			return false;

		unsigned int ip2 = htonl(inet_addr(ipStr));
		unsigned int mask = htonl(inet_addr(subnet));
		return (ip & mask) == (ip2 & mask);
	}

	// Validate the IP address.
	LPCTSTR c = filter;
	LPCTSTR p = pIp;

	// Look if remote and local IP match.
	while (*c && *p)
	{
		if (*c == '*')
		{
			if (*p == '.')
				break;

			// Advance to the next dot.
			while (*p && *p != '.')
				p++;
			c++;
			continue;
		}
		else if (*c != '?') // If it's '?', we don't have to compare.
			if (*c != *p)
				break;
		p++;
		c++;
	}
	return (!*c) && (!*p);
}

bool MatchesRegExp(const CStdString& filter, unsigned int addr)
{
	// If the IP could not be resolved to a hostname, return false.
	HOSTENT *host = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
	if (!host)
		return false;
#ifdef _UNICODE
	boost::regex e(ConvToLocal(filter));
#else
	boost::regex e(filter);
#endif
	return regex_match(host->h_name, e);
}

bool IsUnroutableIP(unsigned int ip)
{
	if (MatchesFilter(_T("127.0.0.1/255.0.0.0"), ip, 0))
		return true;
	
	if (MatchesFilter(_T("10.0.0.0/255.0.0.0"), ip, 0))
		return true;
	
	if (MatchesFilter(_T("172.16.0.0/255.240.0.0"), ip, 0))
		return true;
	
	if (MatchesFilter(_T("192.168.0.0/255.255.0.0"), ip, 0))
		return true;
	
	if (MatchesFilter(_T("169.254.0.0/255.255.0.0"), ip, 0))
		return true;

	return false;
}

bool ParseIPFilter(CStdString in, std::list<CStdString>* output /*=0*/)
{
	bool valid = true;

	in.Replace(_T("\n"), _T(" "));
	in.Replace(_T("\r"), _T(" "));
	in.Replace(_T("\t"), _T(" "));
	while (in.Replace(_T("  "), _T(" ")));
	in.TrimLeft(_T(" "));
	in.TrimRight(_T(" "));
	in += _T(" ");

	int pos;
	while ((pos = in.Find(_T(" "))) != -1)
	{
		CStdString ip = in.Left(pos);
		if (ip == _T(""))
			break;
		in = in.Mid(pos + 1);

		if (ip == _T("*") || IsValidAddressFilter(ip))
		{
			if (output)
				output->push_back(ip);
		}
		else
			valid = false;
	}

	return valid;
}
