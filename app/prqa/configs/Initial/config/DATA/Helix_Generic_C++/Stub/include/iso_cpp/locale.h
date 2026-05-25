/* locale.h	lastmod 19 Nov 91  SAC
 *			created  8 Jun 89  SAC
 * version:	@(#)locale.h	1.3
 * date:		95/03/02
 */

#ifndef _LOCALE_H
#define _LOCALE_H

struct lconv
{	char	*decimal_point, *thousands_sep, *grouping, *int_curr_symbol, *currency_symbol,
		*mon_decimal_point, *mon_thousands_sep, *mon_grouping, *positive_sign,
		*negative_sign, int_frac_digits, frac_digits, p_cs_precedes, p_sep_by_space,
		n_cs_precedes, n_sep_by_space, p_sign_posn, n_sign_posn,
    int_p_cs_precedes, int_n_cs_precedes, int_p_sep_by_space, int_n_sep_by_space, int_p_sign_posn, int_n_sign_posn;
};

#ifndef NULL
#define NULL		(0)
#endif
#define LC_ALL		0x00
#define LC_COLLATE	0x01
#define LC_CTYPE	0x02
#define LC_MONETARY	0x03
#define LC_NUMERIC	0x04
#define LC_TIME	0x05

extern char *setlocale(int,const char *);
extern struct lconv *localeconv(void);

#endif
