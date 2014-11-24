#include "hash.h"



ifstream i_open(const string &str)
{
	ifstream input(str);
	if ( !input )
	{
		string buf = "Not opened \"" + str + '"';
		throw buf;
	}
	return input;
}

ofstream o_open(const string &str)
{
	ofstream input(str);
	if ( !input )
	{
		string buf = "Not opened \"" + str + '"';
		throw buf;
	}
	return input;
}


