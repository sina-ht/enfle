#include "enfle-config.h"
/* Define if your MIPS CPU supports a 2-operand MADD16 instruction. */
/* #undef HAVE_MADD16_ASM */
/* Define if your MIPS CPU supports a 2-operand MADD instruction. */
/* #undef HAVE_MADD_ASM */
/* Define to optimize for accuracy over speed. */
/* #undef OPT_ACCURACY */
/* Define to optimize for speed over accuracy. */
#define OPT_SPEED 1
/* Define to enable a fast subband synthesis approximation optimization. */
/* #undef OPT_SSO */
/* Define to influence a strict interpretation of the ISO/IEC standards, even
   if this is in opposition with best accepted practices. */
/* #undef OPT_STRICT */
/* Version number of package */
#define LIBMAD_VERSION "0.15.0b"
#if defined(ARCH_X86)
#define FPM_INTEL
#elif defined(ARCH_PPC)
#define FPM_PPC
#elif defined(ARCH_SPARC)
#define FPM_SPARC
#elif
//#undef FPM_64BIT
//#undef FPM_ARM
//#undef FPM_MIPS
#define FPM_DEFAULT
#endif
