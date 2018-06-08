
#if !defined _COMMON_H_
#define _COMMON_H_

#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include <vector>
#include <map>
#include <algorithm> 
#include <functional> 
#include <ctype.h>
#include <cctype>
 
#include <iostream>  
#include <locale>

#include <sstream>


using namespace std; 

enum patinst_seq{ s_pinfunc,s_chips,s_udata,s_count,s_mar,s_datgen,s_yalu,s_xalu,s_vec,s_rpt,s_vecdef};

enum adhiz_chantype{ DRIVE, RECEIVE};

enum run_mode{MODE1,MODE2};


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void LogError(string lbl, string ln);
void LogWarning(string lbl, string ln);
void Log(string lbl, string ln);
void ToUpper(string &in);
bool FindFromTable(const char *aptr[], string in, int &retval, bool upper_case = true );
bool FindLabelIndex(string jmp_lbl,int &idx);
void InsertEndLoopTag(int patinst_idx, string comment);
void InsertStartLoopTag(int patinst_idx, int counter_val);
bool UnScrambleResource(string res,vector<int> &idxs, string ps_map);
bool GoSub(int pat_idx, int patinst_idx, string called_pat);
void SetBitDefaults(string &vec_line);
string remove_ch( std::string& str, char ch);
template<typename T>
string toString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}




std::string remove_ws( std::string& str );

std::string trim_left( std::string& str );


string ltrim(string str);
string rtrim(string str);

string trim(string str);
string remove_comment(string in);



void Log(string lbl, string ln);

vector<string> Tokenize(string input, char del);

bool FindFromTable(const char *aptr[], string in, int &retval, bool upper_case );

void ToUpper(string &in);

void printl(string n, string s);

void printl(string s, int num);


bool FindDef(string &str);





bool FindVecDefIndex(string pinname, int &idx);

bool FindFromList(vector<string> *list, string str, int idx_found);

bool LoadCounter(string name,long value);

bool is_number(const string& s);

bool StrTol(string num, unsigned long &ret);

class ScrambleMap{
public:
	string name;
	map<string,string> *m_mapScramble  ; 
	ScrambleMap();
	~ScrambleMap();
};

class PinScramble{
public:
	string name;
	vector<ScrambleMap> *m_vScrambleMap;
	PinScramble();
	~PinScramble();

};

#endif