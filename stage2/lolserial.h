#ifndef __LOLSERIAL_H__
#define __LOLSERIAL_H__

#ifdef LOLSERIAL_DEBUG

void lolserial_init();
void lolserial_suspend(void);
void lolserial_resume(void);

#else

#define lolserial_init()
#define lolserial_suspend()
#define lolserial_resume()

#endif /* LOLSERIAL_DEBUG */

#endif /* __LOLSERIAL_H__ */

