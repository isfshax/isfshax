#ifdef LOLSERIAL_DEBUG

#include "lolserial.h"
#include "gpio.h"
#include "utils.h"
#include "latte.h"
#include <sys/iosupport.h>
#include <string.h>

#define LOLSERIAL_WAIT_TICKS    200

#if 1
#define LOLSERIAL_PIN GP_SENSORBAR
#else
#define LOLSERIAL_PIN GP_DEBUG0
#endif

static int enable = 1;
static char suspend_buf[4096];
static int suspend_len = 0;

/* lolserial string output, now not in assembly... */
static void lolserial_lprint(const char *str, int len)
{
    /* setup output pin */
    clear32(LT_GPIO_OWNER, LOLSERIAL_PIN);
    set32(LT_GPIO_ENABLE, LOLSERIAL_PIN);
    set32(LT_GPIO_DIR, LOLSERIAL_PIN);
    set32(LT_GPIO_OUT, LOLSERIAL_PIN);

    /* loop until null terminator or string end */
    for (const char *end = str + len; *str && (str != end); str++) {
        for (u32 bits = 0x200 | (*str << 1); bits; bits >>= 1) {
            /* set bit value */
            mask32(LT_GPIO_OUT, LOLSERIAL_PIN, (bits & 1) ? LOLSERIAL_PIN : 0);

            /* wait ticks for bit */
            u32 now = read32(LT_TIMER), then = now + LOLSERIAL_WAIT_TICKS;
            for (; then < now; now = read32(LT_TIMER)); /* wait overflow */
            for (; now < then; now = read32(LT_TIMER)); /* wait */
        }
    }
}

/* lolserial is slow, for timing critical tasks it might change
 * the outcome; as a workaround, output can be suspended and
 * printed out at a later point in time */
void lolserial_suspend(void)
{
    memset(suspend_buf, 0, sizeof(suspend_buf));
    suspend_len = 0;
    enable = 0;
}

void lolserial_resume(void)
{
    enable = 1;
    lolserial_lprint(suspend_buf, suspend_len);
}

/* devoptab output function */
static ssize_t lolserial_write(struct _reent *r, void *fd, const char *ptr, size_t len)
{
    if (enable) {
        lolserial_lprint(ptr, len);
    } else {
        for(size_t i = 0; (i < len) && (suspend_len < sizeof(suspend_buf)); i++) 
            suspend_buf[suspend_len++] = ptr[i];
    }

    return len;
}

/* register lolserial devoptab */
void lolserial_init() {
    static devoptab_t lolserial_dotab = {
        .name = "lolserial",
        .write_r = &lolserial_write,
    };
    devoptab_list[STD_OUT] = &lolserial_dotab;
    devoptab_list[STD_ERR] = &lolserial_dotab;
}

#endif /* LOLSERIAL_DEBUG */
