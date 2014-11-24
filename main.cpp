
#include <ctime>
#include "mustache.h"



int main ()
{
	
	Mustache mustache;
	mustache.go("database.txt", "input.txt", "output.txt");
	return 0;
}