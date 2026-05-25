
#pragma PRQA_MESSAGES_OFF 3602,3625

#ifndef LOCALE_IS_INCLUDED
#define LOCALE_IS_INCLUDED

struct lconv
{
  char *  decimal_point;
  char *  thousands_sep;
  char *  grouping;
  char *  mon_decimal_point;
  char *  mon_thousands_sep;
  char *  mon_grouping;
  char *  positive_sign;
  char *  negative_sign;
  char *  currency_symbol;
  char    frac_digits;
  char    p_cs_precedes;
  char    n_cs_precedes;
  char    p_sep_by_space;
  char    n_sep_by_space;
  char    p_sign_posn;
  char    n_sign_posn;
  char *  int_curr_symbol;
  char    int_frac_digits;
  char    int_p_cs_precedes;
  char    int_n_cs_precedes;
  char    int_p_sep_by_space;
  char    int_n_sep_by_space;
  char    int_p_sign_posn;
  char    int_n_sign_posn;
};

#ifndef NULL
#define NULL            ((void *) 0)
#endif

#define LC_ALL          0x00
#define LC_COLLATE      0x01
#define LC_CTYPE        0x02
#define LC_MONETARY     0x03
#define LC_NUMERIC      0x04
#define LC_TIME         0x05

char         * setlocale (int category, const char * locale);

struct lconv * localeconv (void);

#endif
