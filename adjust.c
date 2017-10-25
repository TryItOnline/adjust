// modified to accept files without trailing newlines
// fixed direction bug
// fixed step bug in op 17/19

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef unsigned char cell;

cell *read_arbitrary_length_string(FILE *fp);
void run(cell **code, int lines, int maxline);

int main(int argc, char *argv[])
{
	cell **code;
	FILE *src;
	int codespace, codelines = 0, maxline = 0;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s filename\n",argv[0]);
		return 1;
	}

	src = fopen(argv[1], "r");
	if (src == NULL)
	{
		fprintf(stderr, "Error: Unable to open file %s: %s\n",argv[1],strerror(errno));
		return 2;
	}

	codespace = 10;
	code = malloc(codespace * sizeof(cell*));
	if (!code)
		return 3;

	for (;;)
	{
		int len;
		if (codelines == codespace)
		{
			codespace *= 2;
			code = realloc(code, codespace * sizeof(cell*));
			if (!code)
				return 4;
		}
		code[codelines] = read_arbitrary_length_string(src);
		if (code[codelines] == NULL)
			break;
		len = strlen(code[codelines]);
		if (len > maxline)
			maxline = len;
		codelines++;
	}
	fclose(src);

	run(code, codelines, maxline);
	free(code);
	return 0;
}

cell *addmem(cell *mem, int cursize, int newsize)
{
	mem = realloc(mem, newsize);
	if (!mem)
	{
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}
	memset(&mem[cursize], 0, newsize - cursize);
	return mem;
}

/* For 2 <= n <= 126, next[n] = n / gpf[n]. */
cell next[127] =
{
	 0,  0,  1,  1,  2,  1,  2,  1,
	 4,  3,  2,  1,  4,  1,  2,  3,
	 8,  1,  6,  1,  4,  3,  2,  1,
	 8,  5,  2,  9,  4,  1,  6,  1,
	16,  3,  2,  5, 12,  1,  2,  3,
	 8,  1,  6,  1,  4,  9,  2,  1,
	16,  7, 10,  3,  4,  1, 18,  5,
	 8,  3,  2,  1, 12,  1,  2,  9,
	32,  5,  6,  1,  4,  3, 10,  1,
	24,  1,  2, 15,  4,  7,  6,  1,
	16, 27,  2,  1, 12,  5,  2,  3,
	 8,  1, 18,  7,  4,  3,  2,  5,
	32,  1, 14,  9, 20,  1,  6,  1,
	 8, 15,  2,  1, 36,  1, 10,  3,
	16,  1,  6,  5,  4,  9,  2,  7,
	24, 11,  2,  3,  4, 25, 18
};

/* For 2 <= n <= 126, gpf[n] is n's greatest prime factor. */
cell gpf[127] =
{
	  0,   0,   2,   3,   2,   5,   3,   7,
	  2,   3,   5,  11,   3,  13,   7,   5,
	  2,  17,   3,  19,   5,   7,  11,  23,
	  3,   5,  13,   3,   7,  29,   5,  31,
	  2,  11,  17,   7,   3,  37,  19,  13,
	  5,  41,   7,  43,  11,   5,  23,  47,
	  3,   7,   5,  17,  13,  53,   3,  11,
	  7,  19,  29,  59,   5,  61,  31,   7,
	  2,  13,  11,  67,  17,  23,   7,  71,
	  3,  73,  37,   5,  19,  11,  13,  79,
	  5,   3,  41,  83,   7,  17,  43,  29,
	 11,  89,   5,  13,  23,  31,  47,  19,
	  3,  97,   7,  11,   5, 101,  17, 103,
	 13,   7,  53, 107,   3, 109,  11,  37,
	  7, 113,  19,  23,  29,  13,  59,  17,
	  5,  11,  61,  41,  31,   5,   7
};

cell bitsset[256] =
{
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

#define       LEFT     0
#define    UP_LEFT     1
#define         UP     2
#define   UP_RIGHT     3
#define      RIGHT     4
#define DOWN_RIGHT     5
#define       DOWN     6
#define  DOWN_LEFT     7

#define RIGHT135(d) ((d) = right135((d)))
#define  RIGHT90(d) ((d) =  right90((d)))
#define  RIGHT45(d) ((d) =  right45((d)))
#define   LEFT45(d) ((d) =   left45((d)))
#define   LEFT90(d) ((d) =   left90((d)))
static inline int right135(int dir) { return (dir+3)%8; }
static inline int  right90(int dir) { return (dir+2)%8; }
static inline int  right45(int dir) { return (dir+1)%8; }
static inline int   left45(int dir) { return (dir+7)%8; }
static inline int   left90(int dir) { return (dir+6)%8; }
static inline void movefwd(int *x, int *y, int dir, int steps)
{
	if (dir == LEFT || dir == UP_LEFT || dir == DOWN_LEFT)
		*x -= steps;
	else if (dir == RIGHT || dir == UP_RIGHT || dir == DOWN_RIGHT)
		*x += steps;
	if (dir == UP || dir == UP_LEFT || dir == UP_RIGHT)
		*y -= steps;
	else if (dir == DOWN || dir == DOWN_LEFT || dir == DOWN_RIGHT)
		*y += steps;
}
#ifdef UNSAFE
static inline void checkmove(int x, int y, int w, int h) {}
#else
static inline void checkmove(int x, int y, int w, int h)
{
	if (x < 0 || y < 0 || x >= w || y >= h)
	{
		fprintf(stderr, "Out of bounds\n");
		exit(-1);
	}
}
#endif
static inline void push(cell val, cell **pstack, int *items, int *space)
{
	if (*items == *space)
	{
		*pstack = addmem(*pstack, *space, 2**space);
		*space *= 2;
	}
	(*pstack)[(*items)++] = val;
}
static inline int pop(cell *stack, int *items)
{
	if (*items == 0)
		return -1;
	return stack[--(*items)];
}
static inline int peek(cell *stack, int items)
{
	if (items == 0)
		return -1;
	return stack[items - 1];
}

void run(cell **code, int lines, int maxline)
{
	cell acc, *stack1 = NULL, *stack2 = NULL;
	int stack1size = 10, stack2size = 10;
	int stack1pos = 0, stack2pos = 0;
	int dir = UP_RIGHT, codex, codey;
	int i, *lengths;

	stack1 = addmem(stack1, 0, stack1size);
	stack2 = addmem(stack2, 0, stack2size);
	acc = 0;
	codex = 0;
	codey = lines - 1;

	lengths = malloc(sizeof(int) * lines);
	for (i = 0; i < lines; i++)
		lengths[i] = strlen(code[i]);

	for (;;)
	{
		cell c;
		if (codex >= lengths[codey])
			c = '!'; /* lines are padded with ! */
		else
		{
			c = code[codey][codex];
#ifndef UNSAFE
			if (c < 32 || c > 126)
			{
				fprintf(stderr, "Invalid character\n");
				exit(-1);
			}
#endif
		}

		/* run the commands */
		for (; c > 1; c = next[(size_t) c]) switch(gpf[(size_t) c])
		{
			case 2:
			{
				cell tmp;
				tmp = (acc & 7) << 5;
				acc >>= 3;
				acc |= tmp;
				break;
			}
			case 3:
			{
				int s1, s2;
				s1 = peek(stack1, stack1pos);
				s2 = peek(stack2, stack2pos);
				if (s2 < s1)
				{
					push(acc, &stack2,
						&stack2pos, &stack2size);
					LEFT45(dir);
					movefwd(&codex, &codey, dir, 1);
					break;
				}
				push(acc, &stack1, &stack1pos, &stack1size);
				if (s2 == s1)
				{
					RIGHT45(dir);
					movefwd(&codex, &codey, dir, 1);
					break;
				}
				if (acc)
					LEFT90(dir);
				else
					RIGHT135(dir);
				movefwd(&codex, &codey, dir, 1);
				break;
			}
			case 5:
			{
				if (acc & 1)
					acc--;
				else
					acc++;
				break;
			}
			case 7:
			{
				movefwd(&codex, &codey, dir,
					bitsset[(size_t) acc]);
				break;
			}
			case 11:
			{
				int s1, s2;
				s1 = peek(stack1, stack1pos);
				s2 = peek(stack2, stack2pos);
				if (s1 > s2)
					acc = pop(stack1, &stack1pos);
				else if (s2 == -1)
					acc = 0;
				else
					acc = pop(stack2, &stack2pos);
				break;
			}
			case 13:
			{
				int s2;
				s2 = pop(stack2, &stack2pos);
				if (s2 != -1)
					putchar(s2);
				break;
			}
			case 17:
			case 19:
			{
				int ch;
				if (gpf[(size_t) c] == 17)
					ch = getchar();
				else
					ch = pop(stack2, &stack2pos);
				if (ch < 0 || ch > 255) /* empty stack/EOF */
				{
					int steps = 0;
					if (acc & (1<<2))
						RIGHT90(dir);
					steps += (acc & (1<<3)) > 0;
					steps += (acc & (1<<4)) > 0;
					steps += (acc & (1<<7)) > 0;
					movefwd(&codex, &codey, dir, steps);
					break;
				}
				push(ch, &stack1, &stack1pos, &stack1size);
				break;
			}
			case 23:
			{
				acc <<= 5;
				break;
			}
			case 29:
			{
				int s1, s2, steps = 2;
				if (acc != 0)
					break;
				LEFT45(dir);
				s1 = peek(stack1, stack1pos);
				s2 = peek(stack2, stack2pos);
				if (s2 < s1)
					steps++;
				movefwd(&codex, &codey, dir, steps);
				RIGHT45(dir);
				break;
			}
			case 31:
			{
				movefwd(&codex, &codey, dir, 1);
				break;
			}
			case 37:
			{
				int s1, s2;
				s1 = peek(stack1, stack1pos);
				s2 = peek(stack2, stack2pos);
				if (s1 == s2)
					break;
				if (s1 < s2)
					push(s1, &stack2, &stack2pos,
						&stack2size);
				else
					push(s2, &stack1, &stack1pos,
						&stack1size);
				break;
			}
			case 41:
			{
				int s1, s2;
				s1 = peek(stack1, stack1pos);
				s2 = peek(stack2, stack2pos);
				if (s1 == s2)
					break;
				if (s1 > s2)
					pop(stack1, &stack1pos);
				else
					pop(stack2, &stack2pos);
				break;
			}
			case 43:
			{
				acc >>= 1;
				if (!acc)
				{
					movefwd(&codex, &codey, dir, 1);
					LEFT90(dir);
				}
				break;
			}
			case 47:
			{
				int s1, s2;
				s1 = peek(stack1, stack1pos);
				s2 = peek(stack2, stack2pos);
				if (s1 == s2)
					break;
				if (s1 < s2)
					acc = pop(stack1, &stack1pos);
				else
					acc = pop(stack2, &stack2pos);
				break;
			}
			case 53:
			{
				int s1, s2;
				s1 = peek(stack1, stack1pos);
				s2 = peek(stack2, stack2pos);
				if (s1 < s2)
				{
					cell tmp;
					tmp = acc & 0xf0;
					if (acc & 1)
						tmp |= 8;
					if (acc & 2)
						tmp |= 4;
					if (acc & 4)
						tmp |= 2;
					if (acc & 8)
						tmp |= 1;
					acc = tmp;
				}
				else
				{
					cell tmp;
					tmp = acc & 0x0f;
					if (acc & 16)
						tmp |= 128;
					if (acc & 32)
						tmp |= 64;
					if (acc & 64)
						tmp |= 32;
					if (acc & 128)
						tmp |= 16;
					acc = tmp;
				}
				break;
			}
			case 59:
			{
				int n;
				n = bitsset[(size_t) acc] % 8;
				while(n--)
					RIGHT45(dir);
				break;
			}
			case 61:
			{
				cell *tmps;
				int tmp;
				tmps = stack1;
				stack1 = stack2;
				stack2 = tmps;
				tmp = stack1size;
				stack1size = stack2size;
				stack2size = tmp;
				tmp = stack1pos;
				stack1pos = stack2pos;
				stack2pos = tmp;
				break;
			}
			case 67:
				goto quit;
			default:
				acc = gpf[(size_t) c];
		}

		movefwd(&codex, &codey, dir, 1);
		checkmove(codex, codey, maxline, lines);
	}

quit:
	free(lengths);
	free(stack1);
	free(stack2);
}

cell *read_arbitrary_length_string(FILE *fp)
{
	char *p = NULL, *q;
	int size = 50;

	if ((p = malloc(size)) == NULL)
		return NULL;
	if (fgets(p, size, fp) == NULL)
		return NULL;

	while ((q = strchr(p, '\n')) == NULL)
	{
		size *= 2;
		if ((p = realloc(p, size)) == NULL)
			return NULL;
		if (fgets(&p[size/2-1], size/2+1, fp) == NULL)
			return p;
	}

	*q = '\0';
	return p;
}
