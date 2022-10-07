/*******************************************************************************
 * Copyright (c) 2016 hklo.tw@gmail.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 ********************************************************************************/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xgetopt.h"

 // Instance of extern global variables
s32 xoptind;
s32 xoptopt;
s32 xopterr = 1;
s32 xoptreset;
c8* xoptarg;

// A local-scope variable.
// Used for tracking parsing point within the same argument element.
static c8* xoptcur;

static s32 xgetopt_impl(s32 argc, c8* const argv[],
	const c8* optstring, const struct xoption* longopts,
	s32* longindex, c8 only);

// Permute an argv array. So that all non-option element will be
// moved to the end of argv with respect to their original presenting order.
static void xgetopt_permute(s32 argc, c8* const argv[],
	const c8* optstring, const struct xoption* longopts,
	c8 only)
{
	// Tweak the optstring:
	//   1. Remove any heading GNU flags ('+','-') at front of it (if any).
	//   2. Insert a single '+' at the front.
	c8* pstr = 0;
	if (optstring == 0)
	{
#ifdef _MSC_VER
		pstr = _strdup("+");
#else
		pstr = strdup("+");
#endif
	}
	else
	{
		s32 len = (s32)strlen(optstring);
		pstr = (c8*)malloc(len + 2);
		memset(pstr, 0, len + 2);
		while (((*optstring) == '+') || ((*optstring) == '-'))
			optstring++;
		pstr[0] = '+';
		strcpy(&pstr[1], optstring);
	}

	// Clone longopts and tweak on flag and val.
	struct xoption* x = 0;
	if (longopts)
	{
		s32 cnt;
		for (cnt = 0; (longopts[cnt].name != 0) || (longopts[cnt].flag != 0) || (longopts[cnt].has_arg != 0) || (longopts[cnt].val != 0); ++cnt);
		x = (struct xoption*)malloc(sizeof(struct xoption) * (cnt + 1));
		memset(x, 0, sizeof(struct xoption) * (cnt + 1));
		for (s32 i = 0; i < cnt; ++i)
		{
			x[i].name = longopts[i].name;
			x[i].has_arg = longopts[i].has_arg;
			x[i].flag = 0;
			x[i].val = 0x5299;
		}
	}

	// A c8* buffer at most can hold all entries in argv.
	c8** nonopts = (c8**)malloc(sizeof(c8*) * argc);
	s32 nonoptind = 0;

	s32 old_xopterr = xopterr;
	xopterr = 0;
	xoptreset = 1;

	c8 stop = 0;
	while (!stop)
	{
		s32 code = xgetopt_impl(argc, argv, pstr, x, 0, only);
		switch (code)
		{
		case -1:
			if (xoptind < argc)
			{
				// Detach current element to nonopts buffer.
				// Rotate all remaining elements forward.
				nonopts[nonoptind++] = argv[xoptind];
				c8** pargs = (c8**)argv;
				for (s32 i = xoptind + 1; i < argc; ++i)
					pargs[i - 1] = pargs[i];
				argc--;

				// Continue iteration on xoptind again.
				xoptcur = 0;
				continue;
			}
			else
			{
				stop = 1;

				// Append all element in nonopts at
				// the end of argv.
				c8** pargs = (c8**)argv;
				for (s32 i = 0; i < nonoptind; ++i)
				{
					pargs[argc++] = nonopts[i];
				}
			}
			break;
		default:
			break;
		}
	}

	if (x)
		free(x);
	free(pstr);
	free(nonopts);

	xopterr = old_xopterr;
}

// A general getopt parser. Used for both short options and long options.
static s32 xgetopt_impl(s32 argc, c8* const argv[],
	const c8* optstring, const struct xoption* longopts,
	s32* longindex, c8 only)
{
	// Determine the executable name. This is used when outputing
	// stderr messages.
	const c8* progname = strrchr(argv[0], '/');
#ifdef _WIN32
	if (!progname) progname = strrchr(argv[0], '\\');
#endif
	if (progname)
		progname++;
	else
		progname = argv[0];

	if (optstring == 0)
		optstring = "";

	// BSD implementation provides a user controllable xoptreset flag.
	// Set to be a non-zero value to ask getopt to clean up
	// all internal state and thus perform a whole new parsing 
	// iteration.
	if (xoptreset != 0)
	{
		xoptreset = 0;
		xoptcur = 0;
		xoptind = 0;
		xoptarg = 0;
		xoptopt = 0;
	}

	if (xoptind < 1)
		xoptind = 1;

	c8 missingarg = 0; // Whether a missing argument should return as ':' (true) or '?' (false).
	c8 permute = 1;    // Whether to perform content permuting: Permute the contents of the argument vector (argv) 
	// as it scans, so that eventually all the non-option arguments are at the end.
	c8 argofone = 0;   // Whether to handle each nonoption argv-element as if it were the argument of an option 
	// with character code 1.
//bool dashw = (0 != strstr(optstring, "W;")); // TODO: If 'W;' exists in optstring indicates '-W foo' will be treated as '--foo'.

// Parse the head of optstring for flags: "+-:"
	for (const c8* p = optstring; *p != '\0'; ++p)
	{
		if ((*p == '+') || (*p == '-'))
		{
			permute = 0;
			argofone = (*p == '-');
		}
		else if (*p == ':')
			missingarg = 0;
		else
			break;

		optstring = p + 1;
	}

	xoptarg = 0;

	if ((xoptind >= argc) || (0 == argv))
	{
		xoptind = argc;
		if (permute)
			xgetopt_permute(argc, argv, optstring, longopts, only);
		return -1;
	}

	// Check whether xoptind has been changed since the last valid xgetopt_impl call.
	// Reset xoptcur to NULL if does.
	if (xoptcur)
	{
		c8 inrange = 0;
		for (c8* p = argv[xoptind]; *p != '\0'; ++p)
		{
			if (p != xoptcur) continue;
			inrange = 1;
			break;
		}

		if (!inrange)
			xoptcur = 0;
	}

	// Check the content of argv[xoptind], to see if it is:
	//   1. Non-option argument (not start with '-')
	//   2. A solely '-' or '--'
	//   3. A option which starts with '-' or '--'
	while (xoptind < argc)
	{
		// case 1
		if (!xoptcur && (argv[xoptind][0] != '-'))
		{
			if (argofone)
			{
				xoptarg = argv[xoptind++];
				return 1;
			}
			else if (permute)
			{
				xoptind++;
				continue;
			}
			return -1;
		}

		// case 2
		if (!xoptcur &&
			((0 == strcmp("-", argv[xoptind])) ||
				(0 == strcmp("--", argv[xoptind]))))
		{
			xoptind++;
			break;
		}

		// case 3
		const c8* p = (argv[xoptind][1] == '-') ? &argv[xoptind][2] : &argv[xoptind][1];

		// matching long opts
		do
		{
			if (!longopts || xoptcur || ((argv[xoptind][1] != '-') && !only))
				break; // continue to match short opts ...

			const c8* r;
			r = strchr(p, '=');
			s32 plen = (r) ? (s32)(r - p) : (s32)strlen(p);

			const struct xoption* xo = 0;
			s32 hitcnt = 0;
			for (const struct xoption* x = longopts; ; ++x)
			{
				if (!x->name && !x->flag && !x->has_arg && !x->val) // terminating case: xoption == {0,0,0,0}
					break;

				if (!x->name)
					continue;

				// Long option names may be abbreviated if the abbreviation
				// is unique or is an exact match for some defined option.
				if (0 == strncmp(p, x->name, plen))
				{
					xo = x;
					hitcnt++;
					// is this an exact match?
					if (plen == strlen(x->name))
					{
						hitcnt = 1;
						break;
					}
				}
			}

			if (!xo) // unrecognized option
			{
				if (only)
					break; // continue to match short opts..

				if (xopterr)
					fprintf(stderr, "%s: unrecognized option `%s\'\n", progname, argv[xoptind]);
				xoptind++;
				return '?';
			}
			else if (hitcnt > 1) // ambiguous
			{
				if (xopterr)
					fprintf(stderr, "%s: option `%s\' is ambiguous\n", progname, argv[xoptind]);
				xoptind++;
				return '?';
			}

			if (longindex)
				*longindex = (s32)(xo - longopts);

			switch (xo->has_arg)
			{
			case xrequired_argument: // argument required
			case xoptional_argument: // optional argument
			{
				c8 valid = 1;
				s32 curind = xoptind++;

				// Check whether the argument is provided as inline. i.e.: '--opt=argument'
				const c8* q = strchr(p, '=');
				if (q) // Has inline argument.
				{
					xoptarg = (c8*)(q + 1);
				}
				else if (xo->has_arg == xoptional_argument) // No inline argument implies no argument for 'xoptional_argument'
				{
					;
				}
				else if (xoptind < argc) // For xrequired_argument, use the next arg value if no inline argument present.
				{
					xoptarg = argv[xoptind++];
				}
				else // Missing argument.
				{
					valid = 0;
				}

				if (valid)
				{
					if (xo->flag == 0)
						return xo->val;
					*xo->flag = xo->val;
					return 0;
				}

				if (xopterr)
					fprintf(stderr, "%s: option `%s\' requires an argument\n", progname, argv[curind]);
				if (missingarg)
					return ':';
			}
			return '?';

			default: // no argument
				xoptind++;
				if (xo->flag == 0)
					return xo->val;
				*xo->flag = xo->val;
				return 0;
			}
		} while (0);

		// matching short opts ...
		if (xoptcur)
			p = xoptcur;
		xoptarg = 0;
		const c8* d = strchr(optstring, *p);
		if (d && ((0 == strncmp(d + 1, "::", 2)) || (*(d + 1) == ':')))
		{
			xoptind++;
			xoptarg = 0;
			xoptcur = 0;
			const c8 required = (*(d + 2) != ':') ? 1 : 0;
			if (*(p + 1) != '\0')
			{
				xoptarg = (c8*)(p + 1);
			}
			else if ((xoptind < argc) && (required || (argv[xoptind][0] != '-')))
			{
				xoptarg = argv[xoptind];
				xoptind++;
			}
			else if (required)
			{
				if (xopterr)
					fprintf(stderr, "%s: option requires an argument -- \'%c\'\n", progname, *p);
				xoptopt = *p;
				return (missingarg) ? ':' : '?';
			}
			return *p;
		}

		if (*(p + 1) != '\0')
		{
			xoptcur = (c8*)(p + 1);
		}
		else
		{
			xoptind++;
			xoptcur = 0;
		}

		if (!d)
		{
			xoptopt = *p;
			if (!missingarg && xopterr)
				fprintf(stderr, "%s: invalid option -- \'%c\'\n", progname, *p);
			return '?';
		}

		return *p;
	} // end of while (xoptind < argc)

	if (permute)
		xgetopt_permute(argc, argv, optstring, longopts, only);

	return -1;
}

s32 xgetopt(s32 argc, c8* const argv[], const c8* optstring)
{
	return xgetopt_impl(argc, argv, optstring, 0, 0, 0);
}

s32 xgetopt_long(s32 argc, c8* const argv[], const c8* optstring,
	const struct xoption* longopts, s32* longindex)
{
	return xgetopt_impl(argc, argv, optstring, longopts, longindex, 0);
}

s32 xgetopt_long_only(s32 argc, c8* const argv[], const c8* optstring,
	const struct xoption* longopts, s32* longindex)
{
	return xgetopt_impl(argc, argv, optstring, longopts, longindex, 1);
}

s32 xgetsubopt(c8** optionp, c8* const* tokens, c8** valuep)
{
	c8* p = *optionp;
	c8* n = 0;
	c8* a = 0;
	s32 ret = -1;
	s32 nlen = 0;
	c8 stop = 0;

	for (n = p; !stop; ++n)
	{
		switch (*n)
		{
		case '=':
			if (!a) a = n + 1;
			break;
		case ',':
		case '\0':
			*optionp = (*n == ',') ? n + 1 : n;
			*n = '\0';
			stop = 1;
			break;
		default:
			if (!a) nlen++;
			break;
		}
	}

	c8 found = 0;
	for (s32 i = 0; (nlen > 0) && (tokens[i] != NULL); ++i)
	{
		if (0 == strncmp(tokens[i], p, nlen))
		{
			found = 1;
			ret = i;
			break;
		}
	}

	*valuep = (found) ? a : p;
	return ret;
}

