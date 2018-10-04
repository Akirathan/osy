/**
 * @file print.c
 *
 * Output functions.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <drivers/printer.h>
#include <lib/stdarg.h>

#include <lib/print.h>


/*
 * Number formatting flags for number ().
 */

#define NF_ZEROPAD  1   /* pad with zeros */
#define NF_SIGNED   2   /* unsigned/signed long */
#define NF_PLUS     4   /* display plus sign if signed positive */
#define NF_SPACE    8   /* display space instead of plug sign */
#define NF_LEFT     16  /* justify the number to the left */
#define NF_SPECIAL  32  /* prefix special bases with 0 or 0x */
#define NF_UPCASE   64  /* use upper case digits */


/** Print a character
 *
 * @param c The character to print.
 *
 * @return Number of characters printed.
 *
 */
size_t putc (const char c)
{
	printer_putchar (c);
	return 1;
}


/** Print a zero terminated string
 *
 * @param text The string to print.
 *
 * @return Number of characters printed.
 *
 */
size_t puts (const char *text)
{
	size_t cnt = 0;
	
	while (*text) {
		putc (*text);
		text++;
		cnt++;
	}
	
	return cnt;
}


/** Format a number into a buffer
 *
 * @param buf       Beginning of the buffer.
 * @param end       End of the buffer.
 * @param num       Number for format.
 * @param base      Base to display the number in.
 * @param size      Width of the field.
 * @param precision Precision to display float numbers with.
 * @param format    Number format flags.
 *
 */
static char *number (char *buf, char *end, unsigned long num, int base,
	int size, int precision, int format)
{
	const char *digits;
	static const char digits_locase [] = "0123456789abcdef";
	static const char digits_upcase [] = "0123456789ABCDEF";
	
	int i;
	char pad;
	char sign;
	char tmp[32];
	
	/* sanity check */
	if ((base < 2) || (base > 16)) {
		panic ("%s: unable to format number in base %d\n",
			__FUNCTION__, base);
	}
	
	
	/* select the digit set */
	digits = (format & NF_UPCASE) ? digits_upcase : digits_locase;
	
	/* left justified number can't be zero-padded */
	if (format & NF_LEFT)
		format &= ~NF_ZEROPAD;
	
	/* determine the padding character */
	pad = (format & NF_ZEROPAD) ? '0' : ' ';
	
	/* determine the sign character */
	sign = 0;
	if (format & NF_SIGNED) {
		if ((long) num < 0) {
			sign = '-';
			num = (unsigned long) (- (long) num);
			size--;
			
		} else if (format & NF_PLUS) {
			sign = '+';
			size--;
			
		} else if (format & NF_SPACE) {
			sign = ' ';
			size--;
		}
	}
	
	/* adjust size for special base prefixes */
	if (format & NF_SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	
	/* print the number into the buffer */
	i = 0;
	do {
		tmp[i++] = digits [num % base];
		num /= base;
	} while (num != 0);
	
	/* update the precision if necessary */
	if (i > precision)
		precision = i;
	
	/* pad head with spaces if right aligned */
	size -= precision;
	if (!(format & (NF_ZEROPAD + NF_LEFT))) {
		while (size-- > 0) {
			if (buf <= end)
				*buf = ' ';
			++buf;
		}
	}
	
	/* output the sign character (if any) */
	if (sign) {
		if (buf <= end)
			*buf = sign;
		++buf;
	}
	
	/* output prefixes for special bases */
	if (format & NF_SPECIAL) {
		if (base == 8) {
			if (buf <= end)
				*buf = '0';
			++buf;
			
		} else if (base == 16) {
			if (buf <= end)
				*buf = '0';
			++buf;
			
			if (buf <= end)
				*buf = 'x';
			++buf;
		}
	}
	
	/* pad the head if not left aligned */
	if (!(format & NF_LEFT)) {
		while (size-- > 0) {
			if (buf <= end)
				*buf = pad;
			++buf;
		}
	}
	
	while (i < precision--) {
		if (buf <= end)
			*buf = '0';
		++buf;
	}
	
	/* output the number */
	while (i-- > 0) {
		if (buf <= end)
			*buf = tmp [i];
		++buf;
	}
	
	/* pad the tail if left aligned */
	while (size-- > 0) {
		if (buf <= end)
			*buf = ' ';
		++buf;
	}
	
	/* write the trailing null */
	if (buf <= end)
		*buf = 0;
	
	return buf;
}


/** Display formatted message
 *
 * Display a formatted message on the console and return
 * the number of characters displayed.
 *
 * @param format Format string.
 * @param args   List of arguments.
 *
 */
static size_t vprintk (const char *format, va_list args)
{
	char chr;           /* character argument */
	unsigned long num;  /* number argument */
	const char *str;    /* string argument */
	const void *ptr;    /* pointer argument */
	
	char *beg;
	char *end;
	char buf[32];
	
	int base;            /* base to display number in */
	int flags;           /* number formatting options */
	int width;           /* width of the number field */
	size_t printed = 0;  /* number of printed characters */
	
	beg = buf;
	end = beg + sizeof_array (buf) - 1;
	
	for (/* nothing */; *format; ++format) {
		/* normal characters */
		if (*format != '%') {
			printed += putc (*format);
			continue;
		}
		
		/* process formatting flags */
		format++;
		base = 10;
		flags = 0;
		width = -1;
		
		switch (*format) {
		case '%':
			printed += putc ('%');
			continue;
		
		case 'c':
			chr = (char) va_arg (args, int);
			printed += putc (chr);
			continue;
		
		case 's':
			str = va_arg (args, char *);
			printed += puts (str != NULL ? str : "<NULL>");
			continue;
		
		case 'p':
			ptr = va_arg (args, void *);
			str = number (beg, end, (unsigned long) ptr, 16,
				2 + 2 * sizeof (void *), -1,
				NF_ZEROPAD | NF_SPECIAL);
			printed += puts (beg);
			continue;
		
		case 'd':
		case 'i':
			flags = NF_SIGNED;
			break;
		
		case 'o':
			base = 8;
			flags = NF_SPECIAL;
			break;
		
		case 'u':
			flags = 0;
			break;
		
		case 'x':
			base = 16;
			width = 2 + 2 * sizeof (unsigned int);
			flags = NF_ZEROPAD | NF_SPECIAL;
			break;
		
		default:
			printed += putc ('%');
			printed += putc (*format);
			continue;
		}
		
		/* print the numbers */
		num = va_arg (args, unsigned int);
		if (flags & NF_SIGNED)
			num = (int) num;
		
		str = number (beg, end, num, base, width, -1, flags);
		printed += puts (beg);
	}
	
	return printed;
}


/** printk
 *
 * Display a formatted message on the console and return
 * the number of characters displayed.
 *
 * @param format Format string.
 * @param ...    List of arguments.
 *
 */
size_t printk (const char * format, ...)
{
	va_list args;
	
	va_start (args, format);
	size_t cnt = vprintk (format, args);
	va_end (args);
	
	return cnt;
}


/** Kernel panic
 *
 * Display an error message, dump the contents of the CPU registers
 * and halt the simulator.
 *
 * @param format Format string for vprintk.
 * @param ...    List of arguments for vprintk.
 *
 */
void panic (const char *format, ...)
{
	va_list args;
	
	va_start (args, format);
	vprintk (format, args);
	va_end (args);
	
	msim_reg_dump ();
	msim_halt ();
}
