#pragma once

#include <iostream>
#include <string>
#include "mysql.h"

using namespace std;

class sql_api
{

	public:
		sql_api(const string &host="127.0.0.1",const string &user="root",const string &password="",const string &db="student");
		int my_connect_mysql();
		int my_insert(string &clos, string &data);
		int my_select();
		~sql_api();

	private:
		MYSQL *_conn;
		MYSQL_RES *_res;
		string _host;
		string _user;
		string _password;
		string _db;
		int _port;

};
