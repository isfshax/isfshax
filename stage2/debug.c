#ifdef LOLSERIAL_DEBUG

extern void lolserial_lprint(char *str, int len);

void DEBUG(char *fmt, ...)
{
    char str[256];
    va_list va;
    va_start(va, fmt);
    vsnprintf(str, sizeof(str), fmt, va);
    lolserial_lprint(str, sizeof(str));
    va_end(va);
}

#endif /* LOLSERIAL_DEBUG */
