
#include <stdafx.h>

#include "common.h"

#include "globals.h"




string remove_ch( std::string& str, char ch)
{
	std::string str_no_ws ;
	if(str.length() > 0){
		//std::string ws(" \t\f\v\n\r");
		for ( basic_string<char>::iterator str_Iter = str.begin( ); str_Iter != str.end( ); ++str_Iter ) 
			if(*str_Iter != ch )
					str_no_ws += *str_Iter ;
	}
 return str_no_ws ;
}

string remove_ws( std::string& str )
{
	std::string str_no_ws ;
	if(str.length() > 0){
		std::string ws(" \t\f\v\n\r");
		for ( basic_string<char>::iterator str_Iter = str.begin( ); str_Iter != str.end( ); str_Iter++ ) 
			if( !isspace(*str_Iter))
				if(ws.find(*str_Iter)== -1)
					str_no_ws += *str_Iter ;
	}
	return str_no_ws ;
}

string trim_left( std::string& str )
{
	std::string str_no_ws ;
	if(str.length() > 0){
		std::string ws(" \t\f\v\n\r");
		for ( basic_string<char>::iterator str_Iter = str.begin( ); str_Iter != str.end( ); str_Iter++ ) 
			if( !isspace(*str_Iter)/*ws.find(*str_Iter)!= -1*/ ) 
				str_no_ws += *str_Iter ;
	}
	return str_no_ws ;
}


string ltrim(string str) {
    size_t startpos = str.find_first_not_of(" \t\f\v\r\n");
if( string::npos != startpos )
{
    str = str.substr( startpos );
}

    return str;
}
string rtrim(string str){
size_t endpos = str.find_last_not_of(" \t\f\v\r\n");
if( string::npos != endpos )
{
    str = str.substr( 0, endpos+1 );
}
return str;
}

string trim(string str){
	return ltrim(rtrim(str));
}

string remove_comment(string in){
	if(in.length() > 0 ){
		int idx = in.find("/");
		if(idx >= 0)
			in.erase(idx);
	}
	return in;
}


void Log(string lbl, string ln){
	debugfile << lbl << " : " << ln << endl;
	if(G_VERBOSE)
		cout << lbl << " : " << ln << endl;
}

void LogError(string lbl, string ln){
	errlogfile <<"Error: "<< lbl << " , " << ln << endl;
	cout << "Error: "<< lbl << " , " << ln << endl;
}

void LogWarning(string lbl, string ln){
	G_WARN_CNT++;
	warnlogfile <<"Warning: "<< lbl << " , " << ln << endl;
	if(G_VERBOSE)
	cout << "Warning: "<< lbl << " , " << ln << endl;
}



vector<string> Tokenize(string input,char del){
	vector<string> lines;
	string rd;
	istringstream str(input);
    while (getline(str, rd, del)) {
		#ifdef dbg_tokenize
			cout <<"rd : "<< rd << endl;	
		#endif
			rd = trim(rd);
		lines.push_back(rd);
    }
	return lines;
}


bool FindFromTable(const char *aptr[], string in, int &retval, bool upper_case ){
	#ifdef _debug_on_
		Log("FindFromTable", " -> " + in );
	#endif
	int idx = 0;
	string stop;
	string cmp;
	if(upper_case)
		ToUpper(in);
	while(stop.find("_end") == -1 && idx < 123){ // Updated while loop count from 100->123 for reload counter fix. - TAZ 05/06/2018
		stop.assign(aptr[idx]);
		#ifdef _debug_on_
			cout << aptr[idx] <<endl;
		#endif
		if( in.compare( aptr[idx] ) == 0 ){
			retval = idx;
			return true;
		}
	idx++;
	}
	return false;
}

void ToUpper(string &in){
	transform(in.begin(), in.end(),in.begin(), ::toupper);

}

void printl(string n, string s){
	cout<< n << " : " << s << endl;
}

void printl(string s, int num){
	cout<< s << ":" << num << endl;
}


bool FindDef(string &str){
	map<string,string>::iterator iter = stdmapDEFINE.find( str );
	if( iter != stdmapDEFINE.end() ){ // if found
		str = stdmapDEFINE[str];
		return true;
	}
	else
		return false;
}





bool FindVecDefIndex(string pinname, int &idx){

	
	map<string,int>::iterator iter = stdmapVECDEF.find( pinname );
	if( iter != stdmapVECDEF.end() ){ // if found
		idx = stdmapVECDEF[pinname];
		debugfile << "FindVecDefIndex : "<< pinname << " = "<< idx <<endl;
		return true;
	}
	else
		return false;
	
}

bool FindFromList(vector<string> *list, string str, int idx_found){
	for(size_t c = 0; c < list->size(); c++){
		if(list->at(c).find(str) != -1){
		idx_found = c;
		return true;
		}
	}
return false;
}

bool LoadCounter(string name,long value){
	return true;
}

bool is_number(const string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool StrTol(string num, unsigned long &ret){
	char * pEnd;
	unsigned long val;

	if(num.find("0x")!=-1){
		val =  strtoul(num.c_str(),&pEnd,16);
		//cout <<num << " : " << val <<endl;
	}
	else{
		if(is_number(num))
			val = atoi(num.c_str());
		else{
			return false;
		}
	}

	ret = val;
	return true;
	
}





ScrambleMap::ScrambleMap(){
		m_mapScramble = new map<string,string>;
	}
ScrambleMap::~ScrambleMap(){
	}

PinScramble::PinScramble(){
	m_vScrambleMap = new vector<ScrambleMap>;}
PinScramble::~PinScramble(){}

