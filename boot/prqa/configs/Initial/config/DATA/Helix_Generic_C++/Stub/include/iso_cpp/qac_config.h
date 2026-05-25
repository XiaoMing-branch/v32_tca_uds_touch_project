/* "qac_config.h"
 * Some flags to control how closely the spec is followed
 * paulfitz, Tue Jun  1 10:19:50 IST 1999
 */

#ifndef __QAC_CONFIG__
#define __QAC_CONFIG__

#define __QAC_GETTIMEOFDAY

//#define __QAC_NO_STANDARD_NAMESPACE
//#define __QAC_NO_REL_OPS_NAMESPACE
//#define __QAC_NO_INDIVIDUAL_USING

#ifndef __QAC_NO_STANDARD_NAMESPACE

#define __QAC_BEGIN_NAMESPACE namespace std {
#define __QAC_END_NAMESPACE }

#else

// Following line makes "std::" look like "::"
#define std
#define __QAC_BEGIN_NAMESPACE
#define __QAC_END_NAMESPACE

#endif

#endif // __QAC_CONFIG__
