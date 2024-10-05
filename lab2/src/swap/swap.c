#include "swap.h"

void Swap(char *left, char *right)
{
	char buff;
	buff = *left;
	*left = *right;
	*right = buff;
}
