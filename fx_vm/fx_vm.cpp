// fx_vm.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <iostream>
#include <list>
#include "stdio.h"
#include "Pentagon.h"
#include "fx_vm.h"
#include "deelx.h"
#include "executor.h"
#include "syntaxparser.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

std::vector<std::vector<WordParsed>> clauses;
int CollectClauses(const std::vector<WordParsed>& words_parsed)
{
	clauses.push_back(words_parsed);
	return 0;
}

std::string last_err_str;
Executor exe;
int fxvm_load_file(const char* file_path, IEIBEmulator* emulator, UINT dev_id)
{
	int retcode = 0;
	char buff[1024] = { 0 };
	const int buff_len = sizeof(buff);
	std::vector<std::string> txt;
	std::vector<WordParsed> words;
	FILE* stream;

	LexicalParser lp;
	SyntaxParser  sp;

	lp.Connect2SoftDev(emulator, dev_id);
	exe.Connect2SoftDev(emulator, dev_id);

	clauses.clear();
	exe.Clear();

	stream = fopen(file_path, "r");
	if ( NULL == stream )  // 打开文件失败. 
	{
		retcode = errno;
		last_err_str = strerror(retcode);
		goto _out;
	}


	while ( fgets(buff, buff_len, stream) )
	{
		to_uppercase(buff);
		txt.push_back(buff);
	}
	fclose(stream);

	retcode = lp.ParseText(txt, words, last_err_str);  // 词法分析. 
	if ( 0 != retcode )
	{
		goto _out;
	}
	retcode = sp.CheckSyntax(words, CollectClauses, last_err_str);  // 语法分析. 
	if ( 0 != retcode )
	{
//		printf(err_info.c_str());
		goto _out;
	}

_out:
	return retcode;
}

const char* fxvm_get_lasterr()
{
	return last_err_str.c_str();
}

int fxvm_run_step()
{
	static int step_no;
	int step_count = clauses.size();
	if ( step_no == step_count )
	{
		return 0;
	}
	exe.Run(clauses[step_no]);
	step_no = (step_no + 1) % (step_count + 1);
	
	return step_no;
}

void _fxvm_test(const char* str, const char* pattern)
{
	printf("%s : %s  ", str, pattern);

	CRegexpT <char> regexp(pattern);
	// test
	MatchResult result = regexp.MatchExact(str);

	// matched or not
	if( result.IsMatched() )
		printf("MATCHED.\n");
	else
		printf("UN-MATCHED.\n");
}

int fxvm_read_softunits(const char* type, SoftUnitData* unit_data, int& len)
{
	std::vector<_SoftUnitData> data;
	exe.ReadSoftUnitData(type, data);
	if( unit_data == NULL || len < data.size() )
	{
		len = 0-data.size();
		return -1;
	}
	for ( int i = 0; i < data.size(); i++ )
	{
		memcpy(unit_data, &data[i], sizeof(_SoftUnitData));
	}

	return 0;
}

void _fxvm_parse(const char* filepath)
{
	//char buff[512];
	//int buff_len = sizeof(buff);

	//FILE* stream = fopen(filepath, "r");
	//std::vector<std::string> txt;
	//while ( fgets(buff, buff_len, stream) )
	//{
	//	to_uppercase(buff);
	//	txt.push_back(buff);
	//}
	//fclose(stream);

	//LexicalParser lp;
	//SyntaxParser  sp;

	//int retcode;
	//std::vector<WordParsed> words;
	//std::string err_info;
	//retcode = lp.ParseText(txt, words, err_info);
	//if ( 0 != retcode )
	//{
	//	printf(err_info.c_str());
	//}
	//retcode = sp.CheckSyntax(words, Run, err_info);
	//if ( 0 != retcode )
	//{
	//	printf(err_info.c_str());
	//}
}

