
template<class SS, class ...Args>
int vsprintf(char *buf)
{
    const char* fmt = SS{};

    int len;
    unsigned long num;
    int i, base;
    char *str;
    const char *s;

    int flags;              /* flags to number() */

    int field_width;        /* width of output field */
    int precision;          /* min. # of digits for integers; max
                               number of chars for from string */
    int qualifier;          /* 'h', 'l', or 'L' for integer fields */

    for (str = buf; *fmt; ++fmt) {
            if (*fmt != '%') {
                    *str++ = *fmt;
                    continue;
            }

            /* process flags */
            flags = 0;
          repeat:
            ++fmt;          /* this also skips first '%' */
            switch (*fmt) {
            case '-':
                    flags |= LEFT;
                    goto repeat;
            case '+':
                    flags |= PLUS;
                    goto repeat;
            case ' ':
                    flags |= SPACE;
                    goto repeat;
            case '#':
                    flags |= SPECIAL;
                    goto repeat;
            case '':
                    flags |= ZEROPAD;
                    goto repeat;
            }

            /* get field width */
            field_width = -1;
            if (isdigit(*fmt))
                    field_width = skip_atoi(&fmt);
            else if (*fmt == '*') {
                    ++fmt;
                    /* it's the next argument */
                    field_width = va_arg(args, int);
                    if (field_width < 0) {
                            field_width = -field_width;
                            flags |= LEFT;
                    }
            }

            /* get the precision */
            precision = -1;
            if (*fmt == '.') {
                    ++fmt;
                    if (isdigit(*fmt))
                            precision = skip_atoi(&fmt);
                    else if (*fmt == '*') {
                            ++fmt;
                            /* it's the next argument */
                            precision = va_arg(args, int);
                    }
                    if (precision < 0)
                            precision = 0;
            }

            /* get the conversion qualifier */
            qualifier = -1;
            if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
                    qualifier = *fmt;
                    ++fmt;
            }

            /* default base */
            base = 10;

            switch (*fmt) {
            case 'c':
                    if (!(flags & LEFT))
                            while (--field_width > 0)
                                    *str++ = ' ';
                    *str++ = (unsigned char)va_arg(args, int);
                    while (--field_width > 0)
                            *str++ = ' ';
                    continue;

            case 's':
                    s = va_arg(args, char *);
                    len = strnlen(s, precision);

                    if (!(flags & LEFT))
                            while (len < field_width--)
                                    *str++ = ' ';
                    for (i = 0; i < len; ++i)
                            *str++ = *s++;
                    while (len < field_width--)
                            *str++ = ' ';
                    continue;

            case 'p':
                    if (field_width == -1) {
                            field_width = 2 * sizeof(void *);
                            flags |= ZEROPAD;
                    }
                    str = number(str,
                                 (unsigned long)va_arg(args, void *), 16,
                                 field_width, precision, flags);
                    continue;

            case 'n':
                    if (qualifier == 'l') {
                            long *ip = va_arg(args, long *);
                            *ip = (str - buf);
                    } else {
                            int *ip = va_arg(args, int *);
                            *ip = (str - buf);
                    }
                    continue;

            case '%':
                    *str++ = '%';
                    continue;

                    /* integer number formats - set up the flags and "break" */
            case 'o':
                    base = 8;
                    break;

            case 'x':
                    flags |= SMALL;
            case 'X':
                    base = 16;
                    break;

            case 'd':
            case 'i':
                    flags |= SIGN;
            case 'u':
                    break;

            default:
                    *str++ = '%';
                    if (*fmt)
                            *str++ = *fmt;
                    else
                            --fmt;
                    continue;
            }
            if (qualifier == 'l')
                    num = va_arg(args, unsigned long);
            else if (qualifier == 'h') {
                    num = (unsigned short)va_arg(args, int);
                    if (flags & SIGN)
                            num = (short)num;
            } else if (flags & SIGN)
                    num = va_arg(args, int);
            else
                    num = va_arg(args, unsigned int);
            str = number(str, num, base, field_width, precision, flags);
    }
    *str = '\0';
    return str - buf;
}