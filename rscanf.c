
#include "rscanf.h"
#include <string.h>

#define MAX_DELIM_LEN	50
#define MAX_ARGS		50
#define STR_LEN			50

static int act_fmt;
static void *args[MAX_ARGS];
static int arg_actrep[MAX_ARGS];
static char ch;
static void *reppointer[MAX_ARGS][2];

static int getRepCount(char **fmt, va_list va, char *repdelim, int *alloc)
{
	int num, i;
	char *end;

	if(**fmt != '(')
		return -1;

	(*fmt)++;

	num = strtol(*fmt, &end, 0);

	if(end == *fmt)
		return -1;

	if(num == 0)
		*alloc = 1;
	else if(num < 0)
		return -1;
	else
		*alloc = 0;

	*fmt = end;
	if(**fmt != ',')
	{
		if(**fmt == '*')
		{
			for(i = 0; i < MAX_ARGS; i++)
			{
				if((char*)(reppointer[i][1]) == *fmt)
				{
					reppointer[i][0]++;
					num = *((int*)reppointer[i][0]);
					break;
				}

				if(reppointer[i][0] == NULL)
				{
					reppointer[i][0] = *(va_arg(va, int**));
					num = *((int*)reppointer[i][0]);
					reppointer[i][1] = *fmt;
					break;
				}
			}
			if(i == MAX_ARGS)
				return -1;

			(*fmt)++;
			if(**fmt != ',')
				return -1;
		}
		else
		{
			return -1;
		}
	}
	else if(*alloc == 1)
	{
		num = *(va_arg(va, int*));
	}

	(*fmt)++;

	for(; **fmt != ')'; (*fmt)++)
	{
		*repdelim = **fmt;
		repdelim++;
	}
	*repdelim = '\0';

	(*fmt) += 1;
	if(**fmt != '{')
		return -1;
	(*fmt)++;

	return num;
}

static int repeat(FILE *file, char **fmt, va_list va, int repcount, char *repdelim, int alloclen, int isfirst)
{
	char subrepdelim[MAX_DELIM_LEN];
	int subrepcount, suballoc;
	int i;
	int act_fmt_bak;
	char str[STR_LEN];
	char *fmt_start, *repdelim_bak;
	size_t size;

	act_fmt_bak = act_fmt;
	fmt_start = *fmt;
	repdelim_bak = repdelim;

	while(repcount > 0)
	{
		while(**fmt != '}')
		{
			if(**fmt == '\0')
			{
				for(i = 0; i < MAX_ARGS; i++)
					if(args[i] != NULL)
						free(args[i]);
				return 1;
			}

			if(**fmt == '(')
			{
				subrepcount = getRepCount(fmt, va, subrepdelim, &suballoc);
				if(subrepcount < 0)
					return -5;
				repeat(file, fmt, va, subrepcount, subrepdelim, subrepcount * alloclen, isfirst);
				continue;
			}

			if(**fmt == '%')
			{
				(*fmt)++;
				switch(**fmt)
				{
					case '%':

						act_fmt--;
						break;

					case 'd':

						if(isfirst)
						{
							if(alloclen > 0)
							{
								args[act_fmt] = malloc(alloclen * sizeof(int));
								*(va_arg(va, void**)) = args[act_fmt];
							}
							else
							{
								args[act_fmt] = va_arg(va, void*);
							}
						}

						if(ch == '\0')
							ch = fgetc(file);

						for(i = 0; i < STR_LEN; i++)
						{
							if(ch < '0' || ch > '9')
								break;
							str[i] = ch;
							ch = fgetc(file);
						}
						if(i >= STR_LEN || i == 0)
							return 3;
						str[i] = '\0';

						((int*)args[act_fmt])[arg_actrep[act_fmt]] = atoi(str);
						break;
					
					case 'c':
					
						
						break;
					
					case 's':
					
						if(isfirst)
						{
							if(alloclen > 0)
							{
								args[act_fmt] = malloc(alloclen * sizeof(char**));
								*(va_arg(va, char**)) = args[act_fmt];
								size = STR_LEN;
								for(i = 0; i < alloclen; i++)
									((char**)args[act_fmt])[i] = malloc(size * sizeof(char*));
							}
							else
							{
								args[act_fmt] = va_arg(va, char**);
							}
						}
						
						if(ch == '\0')
							ch = fgetc(file);
						
						i = 0;
						while(ch != *repdelim && ch != *(*fmt + 1))
						{	
							if(ch < '0' || ch > '9')
								break;
							((char**)args[act_fmt])[arg_actrep[act_fmt]][i] = ch;
							ch = fgetc(file);
							i++;
							if(i % STR_LEN == 0)
							{
								size += STR_LEN;
								realloc(((char**)args[act_fmt])[arg_actrep[act_fmt]], size * sizeof(char*));
							}
						}
						if(i % STR_LEN == 0)
							realloc(((char**)args[act_fmt])[arg_actrep[act_fmt]], (size + 1) * sizeof(char*));
						((char**)args[act_fmt])[arg_actrep[act_fmt]][i] = '\0';
						break;
				}
				arg_actrep[act_fmt]++;
				act_fmt++;
			}
			else
			{
				if(ch == '\0')
					 ch = fgetc(file);
				if(ch != **fmt)
					return 3;
				ch = '\0';
			}
			(*fmt)++;
		}
		repcount--;
		if(repcount <= 0)
			break;
		isfirst = 0;
		act_fmt = act_fmt_bak;

		if(ch == '\0')
			 ch = fgetc(file);
		repdelim = repdelim_bak;
		while(*repdelim != '\0')
		{
			if(*repdelim != ch)
				return 4;
			ch = fgetc(file);
			repdelim++;
		}

		(*fmt) = fmt_start;
	}

	(*fmt)++;

	return 0;
}

int rvfscanf(FILE *file, char *fmt, va_list va)
{
	int repcount, alloc, i;
	char repdelim[MAX_DELIM_LEN], str[STR_LEN];
	char ch;

	act_fmt = 0;
	ch = '\0';
	memset(args, 0, MAX_ARGS * sizeof(void*));
	memset(arg_actrep, 0, MAX_ARGS * sizeof(int));
	memset(reppointer, 0, 2 * MAX_ARGS * sizeof(void*));

	while(*fmt != '\0')
	{
		if(*fmt == '(')
		{
			repcount = getRepCount(&fmt, va, repdelim, &alloc);
			if(repcount == -1)
				return 1;
			if(repeat(file, &fmt, va, repcount, repdelim, alloc * repcount, 1))
				return 2;
		}
		else if(*fmt == '%')
		{
			fmt++;
			switch(*fmt)
			{
				case '%':

					if(ch == '\0')
						 ch = fgetc(file);
					if(ch != *fmt)
						return 3;
					ch = '\0';
					break;

				case 'd':

					for(i = 0; i < STR_LEN; i++)
					{
						ch = fgetc(file);
						if(ch < '0' || ch > '9')
							break;
						str[i] = ch;
					}
					if(i >= STR_LEN || i == 0)
						return 3;
					str[i] = '\0';

					*(va_arg(va, int*)) = atoi(str);
					break;
			}
		}
		else
		{
			if(ch == '\0')
				 ch = fgetc(file);
			if(ch != *fmt)
				return 3;
			ch = '\0';
		}
		fmt++;
	}

	return 0;
}

int rfscnaf(FILE *file, char *fmt, ...)
{
	va_list va;
	int x;

	va_start(va, fmt);
	x = rvfscanf(file, fmt, va);
	va_end(va);

	return x;
}

int rscanf(char *fmt, ...)
{
	va_list va;
	int x;

	va_start(va, fmt);
	x = rvfscanf(stdin, fmt, va);
	va_end(va);

	return x;
}
