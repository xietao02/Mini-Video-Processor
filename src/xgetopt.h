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


#pragma once

typedef int s32;
typedef char c8;

#ifdef __cplusplus
extern "C" {
#endif

extern c8  *xoptarg;
extern s32  xoptind;
extern s32  xopterr;
extern s32  xoptopt;
extern s32  xoptreset;


/*****
 *  This is a cross-platform implementation of legacy getopt related functions.
 *  Here provides 4 functions: xgetopt(), xgetopt_long(), xgetopt_long_only(),
 *  and xgetsubopt() which mimic almost the same functionality of both GNU
 *  and BSD's version of getopt(), getopt_long(), getopt_long_only, and 
 *  getsubopt().
 *
 *  Since there were some differences between GNU and BSD's getopt impl, xgetopt
 *  have to choose a side on those differences. Here list the behaviours:
 *
 *    Argv permuting:
 *       GNU: By default would permute the argv so that all non-option argv eventually
 *            will be moved to the end of argv. User can disable permuting by specifying
 *            a '+' char in the optstring.
 *       BSD: No permuting.
 *   xgetopt: Follow GNU. Both '+' and '-' flags in the front of optstring can be recognized
 *            and should work properly.
 *
 *    value of optopt:
 *       GNU: Set optopt only when either meet an undefined option, or detect on a missing
 *            argument.
 *       BSD: Always set it to be the option code which is currently examining at.
 *   xgetopt: Follow GNU.
 *
 *    value of optarg:
 *       GNU: Reset to 0 at the start of every getopt() call.
 *       BSD: No reseting. It remains the same value until next write by getopt().
 *   xgetopt: Follow GNU.
 *
 *    Reset state for parsing a new argc/argv set:
 *       GNU: Simply reset optind to be 1. However, it is impossible to reset parsing
 *            state when an internal short option parsing is taking place in argv[1].
 *       BSD: Use another global boolean 'optreset' for this purpose.
 *   xgetopt: Both behaviours are supported.
 *
 *    -W special case:
 *       GNU: If "W;" is present in optstring, the "-W foo" in commandline will be 
 *            treated as the long option "--foo".
 *       BSD: No mentioned in the man page but seems to support such usage.
 *   xgetopt: Do NOT support -W special case.
 */

s32 xgetopt(
	s32 argc, 
	c8 * const argv[],
	const c8 *optstr);

struct xoption
{
  const c8 *name;
  s32       has_arg;
  s32      *flag;
  s32       val;
};

#define xno_argument (0)
#define xrequired_argument (1)
#define xoptional_argument (2)

s32 xgetopt_long(
	s32 argc, 
	c8 * const argv[],
	const c8 *optstring,
	const struct xoption *longopts,
	s32 *longindex);

s32 xgetopt_long_only(
	s32 argc,
	c8 * const argv[],
	const c8 *optstring,
	const struct xoption *longopts,
	s32 *longindex);


s32 xgetsubopt(
	c8 **optionp,
	c8 * const *tokens,
	c8 **valuep);

#ifdef __cplusplus
}
#endif
