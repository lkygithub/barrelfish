#ifndef ZYNQMP_GEM_DEBUG_H_
#define ZYNQMP_GEM_DEBUG_H_

/*****************************************************************
 * Debug printer:
 *****************************************************************/
#if defined(ZYNQMP_GEM_DEBUG_ON) || defined(GLOBAL_DEBUG)
#define ZYNQMP_GEM_DEBUG(x...) printf("ZYNQMP_GEM: " x)
#else
#define ZYNQMP_GEM_DEBUG(x...) ((void)0)
#endif

#endif // ZYNQMP_GEM_DEBUG_H_
