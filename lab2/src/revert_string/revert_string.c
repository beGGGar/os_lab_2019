#include "revert_string.h"
#include <string.h>

void RevertString(char *str)
{
	char buff;
	int first = 0;
	int end = strlen(str) -1;
	while (first < end){
		buff = str[first];
		str[first] = str[end];
		str[end] = buff;
		end--;
		first++;
	}
}

