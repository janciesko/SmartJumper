typedef unsigned long long u64Int;
typedef unsigned int u32Int;

//--------------------------------------------
//******************************************
//Switches to set up different engines and distributions
//#define LIN_PT
//#define LIN_PT_POW2
#define LCG_PT
//#define LCG_PT_POW2
//*******************************************
//*******************************************

#define TYPE u64Int

//#define RECORD //un-comment to record access pattern
#define CONNECTIVITY 16

#ifdef LIN_PT_POW2
#define ARRAY_T TYPE
#define SIZE_T TYPE
#define DISTRO PassThrough
#define GEN Linear_pow2
#endif

#ifdef LIN_PT
#define ARRAY_T TYPE
#define SIZE_T TYPE
#define DISTRO PassThrough
#define GEN Linear
#endif

#ifdef LCG_PT_POW2
#define ARRAY_T TYPE
#define SIZE_T TYPE
#define DISTRO PassThrough
#define GEN LCG_pow2
#endif

#ifdef LCG_PT
#define ARRAY_T TYPE
#define SIZE_T TYPE
#define DISTRO PassThrough
#define GEN LCG
#endif

#ifdef LIN_LIB_PT
#define ARRAY_T TYPE
#define SIZE_T TYPE
#define DISTRO PassThrough
#define GEN linear_congruential_engine <unsigned int, 48271, 1, 4294967295>
#endif

#ifdef NORMAL
#define ARRAY_T TYPE
#define SIZE_T TYPE
#define DISTRO NormalDistribution
#define GEN default_random_engine
#endif

#ifdef LINEAR
#define ARRAY_T TYPE
#define SIZE_T TYPE
#define DISTRO LinearDistribution
#define GEN default_random_engine
#endif
