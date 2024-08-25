#include "sdkconfig.h"

// Enable this to get cpu stats
#define COLLECT_CPU_STATS 1


#ifdef __cplusplus
extern "C" {
#endif
extern void run_inference(void *ptr);
#ifdef __cplusplus
}
#endif
