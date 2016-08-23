#include "sql_api.h"

sql_api::sql_api(const string &host,const string &user,const string &password,const string &db)
	:_host(host)
	,_user(user)
	,_password(password)
	,_db(db)
{
	_port = 3306;
	_res = NULL;
	_conn = mysql_init(NULL);
}

int sql_api::my_connect_mysql()
{
	//链接数据库
	int ret = -1;
	if(!mysql_real_connect(_conn,_host.c_str(),_user.c_str(),_password.c_str(),_db.c_str(),_port,NULL,0))
	{
		cerr<<"connect failed"<<endl;
	}
	else
	{
		cerr<<"connect success"<<endl;
		ret = 0;
	}

	return ret;
}

int sql_api::my_insert(string &cols, string &data)
{
	string sql = "INSERT INTO student_info";
	sql += cols;
	sql += " values ";
	sql += data;

	int ret = -1;
	cout<<sql<<endl;
	//插入数据
	if(mysql_query(_conn,sql.c_str()) == 0)
	{
		cout<<"insert success"<<endl;
		ret = 0;
	}
	else
	{
		cout<<"insert failed"<<endl;
	}
	return ret;
}

int sql_api::my_select()
{
	string sql = "SELECT * FROM student_info";
	if(mysql_query(_conn,sql.c_str()) == 0)
	{
		cout<<"select success"<<endl;
	}
	else
	{
		cerr<<"select failed"<<endl;
		return -1;
	}
	//获取数据的结果，放到MYSQL_RES 指针
	_res = mysql_store_result(_conn);
	if(_res)
	{
		int lines = mysql_num_rows(_res);
		int cols = mysql_num_fields(_res);
		
		cout<<"lien :"<<lines<<endl<<"cols :"<<cols<<endl;

		MYSQL_FIELD * _fn = NULL;
		for(; _fn = mysql_fetch_field(_res);)
		{
			cout<<_fn->name<<"\t";
		}

		int i = 0;

		cout<<endl;

		for(; i< lines; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(_res);
			int j = 0;
			for(; j < cols ; j++)
			{
				cout<<row[j]<<"\t";
			}
			cout<<endl;
		}
		cout<<endl;
	}
	return 0;
}

sql_api::~sql_api()
{}


