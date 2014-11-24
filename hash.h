#pragma once
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>




using namespace std;


struct HashDatabase
{
	struct Boards
	{
		bool empty()
		{
			return left.empty() && right.empty();
		}
		string left;
		string right;
	};

	HashDatabase* get_next(const string &buf)
	{
		auto it = box.find(buf);
		if (it == box.end()) return NULL;
		return it->second;
	}

	bool this_exists(const string &buf)
	{
		auto it = box.find(buf);
		if (it == box.end()) return false;
		return true;
	}

	string get_data(const string &buf)
	{
		auto it = data.find(buf);
		if (it == data.end()) return "";
		return it->second;
	}

	bool empty()
	{
		return boards.empty() && box.empty() && data.empty();
	}

	Boards boards;
	map<string, string> data;
	map<string, HashDatabase*> box;
};

struct HashText
{
	struct Cell
	{
		Cell()
		{
			first.clear();
			second = NULL;
		}
		string first;
		HashText *second;
	};
	HashText () : box()
	{
		data_is_array = false;
	}
	bool data_is_array;
	vector<int> data;
	vector<int> negative;
	vector<int> positive;
	vector<Cell> box;
};



struct Exception
{
	virtual string what(){return "";}
};

struct WrongDatabaseException : public Exception
{
	string what() {return "Wrong database";}
};

struct WrongTextException : public Exception
{
	string what() {return "Wrong text";}
};


class ParserDatabase
{
	struct Brackets
	{
		char left;
		char right;
	};
	
	char skip_space (istream &input)
	{
		char ch;
		do
		{
			ch = input.get();
		}  while ( isspace(ch) );
		return ch;
	}

	Brackets find_brackets(char ch)
	{
		auto it = brackets.begin();
		for (; it != brackets.end(); it++)
		{
			if ( it->left == ch ) break;
		}
		return *it;
	}

	void create_box(istream &input, string &name, HashDatabase *&hash)
	{
		Brackets it = find_brackets('{');

		char ch = skip_space(input);
		
		stringstream input_inside;

		if ( ch = it.left )
		{
			string buf;
			get_inside (input, buf, it);
			input_inside << buf;
		}

		read_inside(input_inside, hash->box[name]);
	}

	void create_boards(string &str_input, HashDatabase *&hash)
	{
		Brackets it = find_brackets('"');

		stringstream input(str_input);
		
		char ch = skip_space(input);
		if ( it.left == ch )	get_inside(input, hash->boards.left, it);
		else throw WrongDatabaseException();

		ch = skip_space(input);
		if ( it.left == ch )	get_inside(input, hash->boards.right, it);
		else throw WrongDatabaseException();
	}

	void create_date(string &str_input, HashDatabase *&hash)
	{
		Brackets it_1 = find_brackets('"');
		
		Brackets it_2 = find_brackets('(');

		stringstream input(str_input);

		string key;
		string value;

		for(;;)
		{
			char ch = skip_space(input);

			if ( input.eof() ) return;

			if ( it_1.left == ch )	get_inside(input, key, it_1);
			else throw WrongDatabaseException();

			ch = skip_space(input);
			if ( it_2.left == ch )	get_inside(input, value, it_2);
			else throw WrongDatabaseException();

			hash->data[key] = value;
		}
	}


	
	void get_inside(istream &input, string &str_inside, Brackets brakets)
	{
		char ch;
		int index = 0;
		str_inside = "";
		for (;;)
		{
			ch = input.get();
			if ( ch == brakets.right ) index--;
			else if ( ch == brakets.left ) index++;
			if ( index < 0 )
			{				
				return;
			}
			if ( input.eof() ) throw WrongDatabaseException();
			str_inside += ch;
		}
	}

	void read_inside (istream &input_inside, HashDatabase *&hash_inside)
	{
		hash_inside = new HashDatabase;
			
		read_box(input_inside, hash_inside);
			
		if ( hash_inside->empty() )
		{
			delete hash_inside;
			hash_inside = NULL;
		}
	}

	void read_box (istream &input, HashDatabase *&hash)
	{
		string str_inside;
		char ch;
		for(;;)
		{
			str_inside.clear();

			ch = skip_space(input);

			if ( input.eof() ) return;

			bool is_ok = false;
			for (auto it = brackets.begin(); it != brackets.end(); it++)
			{
				if ( ch == it->left )
				{
					is_ok = true;
					get_inside (input, str_inside, *it);

					if ( it->left == '"' )
					{
						create_box(input, str_inside, hash);
					}
					else if ( it->left == '<' )
					{
						create_boards(str_inside, hash);
					}
					else if ( it->left == '[' )
					{
						create_date(str_inside, hash);
					}
					else throw WrongDatabaseException();
					break;
				}
			}
			if ( !is_ok ) throw WrongDatabaseException();
		}
	}

	vector<Brackets> brackets;
public:
	ParserDatabase ()
	{
		brackets.resize(5);
		brackets[0].left	= '"';
		brackets[0].right	= '"';
		brackets[1].left	= '(';
		brackets[1].right	= ')';
		brackets[2].left	= '<';
		brackets[2].right	= '>';
		brackets[3].left	= '[';
		brackets[3].right	= ']';
		brackets[4].left	= '{';
		brackets[4].right	= '}';
	}
	void read(istream &input, HashDatabase *&hash)
	{
		read_box (input, hash);
	}
};


class ParserText
{
	bool get_line(istream &input, string &str, string find)
	{
		char ch;
		string buf;
		str.clear();
		while ( !input.eof() )
		{
			getline(input, buf, find[0]);
			if ( !input.eof() )
			{
				str += buf;
				bool is_ok = true;
				for (int i = 1; i < find.size(); i++)
				{
					ch = input.get();
					if ( ch != find[i] )
					{
						str += find[0];
						int it = input.tellg();
						input.seekg(it-i);
						is_ok = false;
						break;
					}
				}
				if ( is_ok ) return true;
			}
			else
			{
				str += buf;
			}
		}
		return false;
	}
	char get_condition(istream &input)
	{
		char ch = input.get();
		if ( (ch == '#') || (ch == '^') || (ch == '?') )
		{
			return ch;
		}
		else
		{
			int it = input.tellg();
			input.seekg(it-1);
			return '\0';
		}
	}
	void read_box(istream &input, HashText *&hash)
	{
		HashText::Cell buf;
		while ( !input.eof() )
		{
			if ( get_line(input, buf.first, "{{") )
			{
				if ( !buf.first.empty() ) hash->box.push_back(buf);
				char ch = get_condition(input);
				if ( !get_line(input, buf.first, "}}") ) throw WrongTextException();

				hash->box.push_back(buf);
				HashText::Cell &it = hash->box[hash->box.size() - 1];
				if ( (ch == '#') || (ch == '^') || (ch == '?') )
				{
					it.second = new HashText;

					if ( ch == '?' ) ch = '#';
					else if ( ch == '#' ) it.second->data_is_array = true;

					if ( ch == '^' ) hash->negative.push_back(hash->box.size() - 1);
					else hash->positive.push_back(hash->box.size() - 1);
					

					string find = "{{/" + buf.first + "}}";
					if ( !get_line(input, buf.first, find) ) throw WrongTextException();
				
					stringstream input_inside(buf.first);
					read_box (input_inside, it.second);

					if ( it.second->box.empty() )
					{
						delete it.second;
						it.second = NULL;
					}
				}
				else
				{
					hash->data.push_back(hash->box.size() - 1);
				}
			}
			else
			{
				if ( !buf.first.empty() ) hash->box.push_back(buf);
			}
		}
	}
public:
	void read(istream &input, HashText *&hash)
	{
		read_box(input, hash);
	}
};

class Traslator
{
	bool check_condition(stringstream &input, HashDatabase *&it_database)
	{
		string buf;
		for(;;)
		{
			getline(input, buf, '.') ;
			if( input.eof() )
			{
				if ( buf.empty() ) return true;
				else if ( it_database->this_exists(buf) )
				{
					it_database = it_database->get_next(buf);
					return true;
				}
				else return false;
			}
			it_database = it_database->get_next(buf);
			if ( it_database == NULL ) return false;
		}
	}
	string get_ready_text(HashText *text_inside)
	{
		string buf = "";
		for (auto it = text_inside->box.begin(); it != text_inside->box.end(); it++)
		{
			buf += it->first;
		}
		return buf;
	}
	

	void go_condition(HashDatabase *database_inside, HashText *text_inside, vector<int> &condition, bool is_positive)
	{
		for (int i = 0; i < condition.size(); i++)
		{
			int pos = condition[i];
			HashText::Cell &it = text_inside->box[pos];

			stringstream input( it.first );
			HashDatabase *it_database = database;

			it.first.clear();
			if ( is_positive == check_condition(input, it_database) )
			{
				if ( it_database != NULL ) it.first += it_database->boards.left;
				it.first += go(it_database, it.second);
				if ( it_database != NULL ) it.first += it_database->boards.right;
			}
		}
	}

	string go_data (HashDatabase *database_inside, HashText *text_inside)
	{
		if ( database_inside == NULL)
		{
			for (int i = 0; i < text_inside->data.size(); i++)
			{
				int pos = text_inside->data[i];
				HashText::Cell &it = text_inside->box[pos];
				it.first.clear();
			}
			return get_ready_text(text_inside);
		}
		else
		{
			if ( text_inside->data_is_array )
			{
				vector<string> buf;
				text_inside->data_is_array = false;
				for (int i = 0; i < text_inside->data.size(); i++)
				{
					int pos = text_inside->data[i];
					HashText::Cell &it = text_inside->box[pos];
					buf.push_back(it.first);
				}
				string str;
				for (auto it = database_inside->box.begin(); it != database_inside->box.end(); it++)
				{
					if ( it != database_inside->box.begin()) str += '\n';
					if ( it->second != NULL) str += it->second->boards.left;
					str += go_data(it->second, text_inside);
					if ( it->second != NULL) str += it->second->boards.right;

					for (int i = 0; i < text_inside->data.size(); i++)
					{
						int pos = text_inside->data[i];
						HashText::Cell &it = text_inside->box[pos];
						it.first = buf[i];
					}
				}
				return str;
			}
			else
			{
				for (int i = 0; i < text_inside->data.size(); i++)
				{
					int pos = text_inside->data[i];
					HashText::Cell &it = text_inside->box[pos];
					it.first = database_inside->get_data( it.first );
				}
				return get_ready_text(text_inside);
			}
		}
		
	}


	string go(HashDatabase *database_inside, HashText *text_inside)
	{
		go_condition(database_inside, text_inside, text_inside->positive, true);
		go_condition(database_inside, text_inside, text_inside->negative, false);
		
		return go_data(database_inside, text_inside);
	}
	HashDatabase *database;
	HashText *text;
public:
	string translate(HashDatabase *_database, HashText *_text)
	{
		database = _database;
		text = _text;
		return go(database, text);
	}
};

ifstream i_open(const string &str);
ofstream o_open(const string &str);