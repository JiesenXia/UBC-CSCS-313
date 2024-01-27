#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Here is your personalized structure! */
struct mystruct {
	int32_t f1;
	uint8_t f2;
	uint64_t f3;
};

#define	FMT1	"%"PRId32"\n"
#define	FMT2	"%"PRIx8"\n"
#define	FMT3	"0x %"PRIx64"\n"

