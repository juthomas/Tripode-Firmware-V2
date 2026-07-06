#include "tripodes.h"

int convertCharToBase35(char c)
{
	if (c >= 'A' && c <= 'Z')
		return (c - 'A' + 10);
	if (c >= 'a' && c <= 'z')
		return (c - 'a' + 10);
	if (c >= '0' && c <= '9')
		return (c - '0');
	return 0;
}

char convertBase35ToChar(int nb)
{
	if (nb < 0)
		return '0';
	if (nb > 35)
		return 'z';
	if (nb < 10)
		return (char)('0' + nb);
	return (char)('a' + nb - 10);
}
