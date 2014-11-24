#pragma once

#include "hash.h"

class Mustache
{
	static void destroy(HashDatabase *&hash)
	{
		if ( hash == NULL ) return;
		auto &box = hash->box;
		for (auto it = box.begin(); it != box.end(); it++)
		{
			destroy( it->second );
		}
		delete hash;
		hash = NULL;
	}
	static void destroy(HashText *&hash)
	{
		if ( hash == NULL ) return;
		auto &box = hash->box;
		for (auto it = box.begin(); it != box.end(); it++)
		{
			destroy( it->second );
		}
		delete hash;
		hash = NULL;
	}
	ParserDatabase parser_database;
	ParserText parser_text;
	Traslator traslator;

	HashText *text;
	HashDatabase *database;
public:
	Mustache() : parser_database()
	{
		database = new HashDatabase;
		text = new HashText;
	}
	~Mustache()
	{
		destroy(database);
		destroy(text);
	}
	
	void read_database(const string &file)
	{
		ifstream &input = i_open(file);
		parser_database.read(input, database);
		input.close();
	}
	void read_text(const string &file)
	{
		ifstream &input = i_open(file);
		parser_text.read(input, text);
		input.close();
	}
	void write(const string &file, const string &str)
	{
		ofstream &output = o_open(file);
		output << str;
		output.close();
	}

	void go(const string &str_database, const string &str_text, const string &str_out)
	{
		try
		{
			read_database(str_database);
			read_text(str_text);
			string str = traslator.translate(database, text);
			write(str_out, str);
		}
		
		catch (string err)
		{
			cerr << err << endl;
		}
		
		catch (Exception &err)
		{
			cerr << err.what() << endl;
		}
	}
};