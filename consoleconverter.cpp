// consoleconverter.cpp : Defines the entry point for the console application.
//
 // whatever you need to do to enable reinclusion

#pragma warning (disable : 4786)
#include "stdafx.h"

#include <windows.h>
#include "common.h"
#include <regex>

#include "parsescramblefile.h"
#include "parsepatternfile.h"
#include "parsedefinefile.h"
#include "pattern.h"

//#include "AfxWin.h"

#include "globals.h"

HANDLE wHnd; /* write (output) handle */
HANDLE rHnd; /* read (input handle */

#define MAXPATTERN 100
#define MAXPATINST 100
#define MAXPATMICRO 100

typedef vector<string> VecOfStr;
typedef vector<VecOfStr> VecOfVecOfStr;

#define VECTOR_WIDTH 128
#define COUNTERS_CNT 61
#define NESTED_LOOP_ALLOWED 2

vector<int> G_COUNTERS(COUNTERS_CNT,0);
vector<int> G_COUNTERS_RLD(COUNTERS_CNT,0);
string G_JMP_LABEL;
int G_VEC_LINE_SIZE;
string directory;
string directoryOut;
string directoryDebug;
bool Custom_Output = false;
bool Custom_DebugOutput = false;
bool Custom_Input = false;
char G_DEFAULT_PIN_STATE = 'X';
string G_INPUT_FILE;
string G_PINSCRAMBLE_NAME;
string G_VECDEF;
string G_RPT_OPT; // used if repeat condition is required
int G_WARN_CNT = 0;
int G_COUNTER_SEL = 0; // index of the counter selected through recent call of COUNT micro instruction
int G_PAT_IDX;
int G_PATINST_IDX;
int G_PATINST_SEL = 0; // index of pattern instruction currently processed in ConvertPatInst
size_t rowSize;

bool G_ADD_KNOWN = true;

bool G_HALT_E = true;

bool G_HALT_W = true;

bool G_ADD_ORIG = true;

bool G_DEBUG_ON = false;

bool G_VERBOSE = false;

bool G_GUI = false;

bool G_USAGE = false;

bool G_CHANGELOG = false;

bool G_WARN_ALU = false;

bool G_ALU_ON = true;

int Nested_Loop = 0;
bool Loop_Violation = false;
string Loop_Pattern_List;

unsigned long G_JAMREG,G_DATREG = 0xc0;
unsigned long G_UDATA;
unsigned long G_DBM;
unsigned long G_XADDR;
unsigned long G_YADDR;
unsigned long G_YMAIN,G_YBASE,G_YFIELD,G_XMAIN,G_XBASE,G_XFIELD,G_NUMX,G_NUMY;
string sourceRegName;
long sourceRegValue;

bool 
G_CS_ACTIVE_H[8] = {false, false, false, false, false, false, false, false};

map<string,string> stdmapDEFINE;

map<string, int[2]> labelParseDEF;


int G_MODE = MODE1;

int G_ADHIZ = DRIVE;

VecOfStr stdvecPATINST;
 
//map<string , int > stdmapPATTERNS;

map<string,int> stdmapVECDEF;

map<int,char> stdmapDEFCHAR;

typedef pair<int,char> def_char_pair;
vector<def_char_pair> vpairDEFCHAR;

map<string,string> stdmapPATINITS;

vector<string> vTriStatePins;

vector<PinScramble> vPINSCRAMBLES; // vector of PIN_SCRAMBLE

vector<Pattern> vvsPATTERNS; // vector of pattern instructions

vector<string> vsEXECPATNAMES;

string currentPattern;
map<string, string> jump2gosub;
map<string, string> jumpPatternMap;
map<string,map <string, string>> pat_JumpLabel;

ofstream debugfile;
ofstream warnlogfile;
ofstream errlogfile;

//string inputFileName = G_INPUT_FILE;

/***********************- POSITIONNING THE CONSOLE WINDOW -*******************************/

//Finding the user's screen resolution
int Width = GetSystemMetrics(SM_CXSCREEN);
int Height = GetSystemMetrics(SM_CYSCREEN);

//Assigning variables for MoveWindows parameters
int WindowWidth = 800;		//--- Used as a parameter to specify the width of the console window (MoveWindows int nWidth)
int WindowHeight = 1000;		//--- Used as a parameter to specify the height of the console window (MoveWindows int nHeight)
int NewWidth = (Width - WindowWidth) / 2;		//--- Used as a parameter to center the console window horizontally (MoveWindows int x)
int NewHeight = ((Height - WindowHeight) / 2);		//--- Used as a parameter to center the console window vertically (MoveWindows int y)

													//Getting the console window handle
HWND hWnd = GetConsoleWindow();

string REVISION = "3.05";

//Declaring the function
#pragma warning( disable : 4273 )

//BOOL WINAPI MoveWindow(_In_ HWND hWnd, _In_ int NewWidth, _In_ int NewHeight, _In_ int WindowWidth, _In_ int WindowHeight, _In_ BOOL bRepaint);

bool HEADER_GRAPHIC = true; // Header graphic box control (on/off)
bool MAX_WINDOW = true; // Resize console window (on/off)

int XPosLast;
int YPosLast;
string RemoveChar(string string_in, string remove_char);
string filterMicroInst(string type, string uInst);
void changeLog() {
	string changeLogMessage =	
		R"(	_______________________________________________________________________________

					APG Tool Change Log
	_______________________________________________________________________________

	08/14/2017
	-Initial Release
	_______________________________________________________________________________
	11/03/17
	-Added a  write once logic for Select 0.
	_______________________________________________________________________________
	12/14/17 Version 2.1
	-Added - Duplicate pattern names Dialog Message with program exit.
	-Fixed - VEC / RPT repeating count.
	-Fixed - RPT Processing puts the operand on the next VEC instruction.
	-Fixed - mar readudata H/L  issue resolved.
	-Fixed - mar read and adhiz issue resolved.
	-Fixed - vecdef continuation character '/' issue resolved.
	-Added - VIHH2 for VEC w/RST_12V microinstruction resolved.
	_______________________________________________________________________________
	1/17/18
	-Fix adhiz strobe bug resolved.
	-Added multi vecdef capability.
	-Added directory selection capability for input and output directories.
	-Fix regression for  jam and data registers loads.
	_______________________________________________________________________________
	6/08/18
	-Added DBM memory using register BUFBUF capability.
	-Added Nested Loop jumps within a loop is Illegal warning dialog.
	-Fixed Start Loop issue comments out resolved.
	-Fixed Pin name in scramble with a Macro definition handling.
	-Added cmpldr instruction to complement datareg data. 
	-Fixed Counter reload is supported properly.
	_______________________________________________________________________________
	7/11/18
	-Added Implement cntupdr and cntdndr micro instruction.
	-Fixed Create PerPatInits file creation.
		Usage : APG_TOOL_VX.X /req
	-Fixed No original code in output file.Puts comments in with '//'.
		Original code removed only comments remain.
		Usage : APG_TOOL_VX.X /no_orig_code
	-Fixed The comments /* and */ block comments issue resolved.
		Comments are removed.
	-Fixed Gosub when target is a label resolved.
		Parsed label micro to mar done to create an optional new pattern.
	-Added Header Graphics with progress bar for Parsing, Converting, Writing,
		build date, and Revision.
	_______________________________________________________________________________
	7/25/18
	-Added options for X/Y ALU ON/OFF and ALU Warnings ON/OFF.
	_______________________________________________________________________________
	8/1/18
	-Added X/Y ALU operations below for multi SourceA/B combinations for 
		checkout. X/Y ALU options processed: INCREMENT, DECREMENT, DOUBLE, COMP, 
		NAND, NOR, AND, OR, XOR, ADD, and SUBTRACT
	_______________________________________________________________________________
	8/6/18
	-Added Pinfunc VTSET instruction which selects the VAR Engine as the source of 
		the time set.
	-Added option to turn off header and progress bar graphics for unsupported terminals
	-Added Change Log viewing option.
	_______________________________________________________________________________
	1/24/19
	-Added shldr and shrdr instruction which shifts the contents of the datreg. 
	)";
	cout << changeLogMessage << endl;
}





void getFilesList(string filePath,string extension, vector<string> & returnFileName)
{
    WIN32_FIND_DATA fileInfo;
    HANDLE hFind;   
    string  fullPath = filePath + extension;
    hFind = FindFirstFile(fullPath.c_str(), &fileInfo);
    if (hFind != INVALID_HANDLE_VALUE){
        returnFileName.push_back(filePath+fileInfo.cFileName);
        while (FindNextFile(hFind, &fileInfo) != 0){
            returnFileName.push_back(filePath+fileInfo.cFileName);
        }
    }
}

bool GoSub(int pat_idx, int patinst_idx, string called_pat){

	int new_idx = -1;

	vector<string> *listVecLines = vvsPATTERNS.at(pat_idx).m_listPatInst->at(patinst_idx).m_listVecConverted;

	for (  vector<Pattern>::iterator it = vvsPATTERNS.begin(); it != vvsPATTERNS.end(); ++it){
		if(it->name.compare(called_pat) == 0){
			new_idx= it->index;
			break;
		}
	}
	if(new_idx == -1){
		 setTextColor(12);
		LogError("GoSub",": subroutine " + called_pat + " not found." );
		 setTextColor(10);
		if (G_HALT_E) {
			cout << "Hit any key to continue" << endl;
			_getch();
		}
		return false;
		}

	//int c;

	if(vvsPATTERNS.at(pat_idx).m_listPatInst->at(patinst_idx).isXAddrChanged){
		for(size_t c =0; c < vvsPATTERNS.at(new_idx).m_listPatInst->size(); c++ )
			vvsPATTERNS.at(new_idx).m_listPatInst->at(c).isXAddrChanged = true;
	Log("GoSub","isXAddrChanged");
	}

	if(vvsPATTERNS.at(pat_idx).m_listPatInst->at(patinst_idx).isYAddrChanged){
		for(size_t  c =0; c < vvsPATTERNS.at(new_idx).m_listPatInst->size(); c++ )
			vvsPATTERNS.at(new_idx).m_listPatInst->at(c).isYAddrChanged = true;
		Log("GoSub","isYAddrChanged");
	}

	if(vvsPATTERNS.at(pat_idx).m_listPatInst->at(patinst_idx).isInLoop){
		for(size_t c =0; c < vvsPATTERNS.at(new_idx).m_listPatInst->size(); c++ )
			vvsPATTERNS.at(new_idx).m_listPatInst->at(c).isInLoop = true;
	Log("GoSub","isInLoop");
	}
	//_getch();

	vvsPATTERNS.at(new_idx).m_isSubPat=true;

	vvsPATTERNS.at(new_idx).setSubVecDef(vvsPATTERNS.at(pat_idx).getSubVecDef());

	vvsPATTERNS.at(new_idx).Reset();

	vvsPATTERNS.at(new_idx).Convert(directoryOut);
	for(size_t c =0; c < vvsPATTERNS.at(new_idx).m_listVecConv->size(); c++ ){
		listVecLines->push_back(vvsPATTERNS.at(new_idx).m_listVecConv->at(c));
		Log("GoSub", "GOSUB : new_pat->m_listVecConv->at(c):" + vvsPATTERNS.at(new_idx).m_listVecConv->at(c) + "\r\n");
		

	}
	
	listVecLines->push_back("// GOSUB exit from : " + called_pat);


	return true;
}

void addPinScramble(string name){

	PinScramble *ps = new PinScramble;
	ps->name = trim(name);
	Log("addPinScramble", ps->name);
	vPINSCRAMBLES.push_back(*ps);

}

void addScrambleMap(string name){
	ScrambleMap *sm = new ScrambleMap;
	sm->name = ltrim(rtrim(name));
	ToUpper(sm->name);
	vPINSCRAMBLES.back().m_vScrambleMap->push_back(*sm);
}

void addScramble(string val){
	vector<string> p;
	string key,value;
	p = Tokenize(val,',');
	key = ltrim(rtrim(p[0]));
	value =  ltrim(rtrim(p[1]));
	pair<string,string> pr;
	pr.first = key;
	pr.second = value;
	vPINSCRAMBLES.back().m_vScrambleMap->back().m_mapScramble->insert(pr);
	/*int idx;
	if(FindVecDefIndex(key,idx)){
		cout << "pin : " << key << "index :" << idx <<" in vector definition." << endl;
		
	}*/
}

void UpdateDefinitions(){
	if (directoryDebug.size() == 0) {
		directoryDebug = "debug_files\\";
	}
	ofstream outfile;
	outfile.open(directoryDebug+"updated_definitions.txt");
	string str;
	for( map<string,string>::iterator iter = stdmapDEFINE.begin(); iter != stdmapDEFINE.end(); ++iter){
		str = iter->second;
		if(FindDef(str))
			iter->second = str;
		outfile << iter->first << " = " << iter->second << endl;
	}	
	outfile.close();
}

void addDefinition(string pair_str, string fname){

	pair_str =  trim(pair_str);
	
	string name,value;

	if(pair_str.find_first_of(" \t") != -1){ 
		name = pair_str.substr(0, pair_str.find_first_of(" \t"));
		value = pair_str.substr(pair_str.find_first_of(" \t"));
		name = trim(name);//remove_ws(name);
		value = trim(value);

	}
	else{
		name = pair_str;
		value = "";
	}

	
	string tmp = name;
	if(G_MODE == MODE1){
		if(FindDef(tmp) ){
			LogWarning("re-definition of " + name + " in this file - " + fname, " previous value = " + tmp);
		//if(G_HALT_W)
		//	_getch();
		}
	}
	stdmapDEFINE[name] = value;
	//_getch(); 

}

bool checkPatternExist(string pname){

	bool found = false;
	for(size_t c = 0; c < vvsPATTERNS.size(); c++ ){
	
		Log(vvsPATTERNS.at(c).getName(),trim(pname));

		if(vvsPATTERNS.at(c).getName().compare(trim(pname)) == 0){
		found = true; 
		break;
		}
	}
	return found;

}

void addComment(string comment){
	
	if(!vvsPATTERNS.empty()){

		string cm = trim(comment);
		
		if(cm.length() > 0){

			if(cm.find("//")!= -1){
				if (cm.find("//") < cm.find_first_not_of(" /")) {
					comment.pop_back();
					vvsPATTERNS.back().m_listComments->push_back("" + comment);
				}
				else {
					vvsPATTERNS.back().m_listComments->push_back("" + comment);
				}
			}
			else {
				if (comment.find('\n') != -1) { // Added to insert '//' at the begining of block comment line. - TAZ 06/11/2018
					vector<char> v_comment;
#pragma warning( disable : 4309 )
#pragma warning( disable : 4305 )
					char comment_str = '//';
					for (size_t i = 0; i < comment.size(); i++) {
						char comment_temp = comment[i];
						v_comment.push_back(comment_temp);
						if (comment_temp == '\n') {
							v_comment.pop_back();
							v_comment.push_back(comment_str);
							v_comment.push_back(comment_str);
						}
					}
					string comment_temp2;
					for (size_t i = 0; i < v_comment.size(); i++) {
						comment_temp2 = comment_temp2 + v_comment[i];
					}
					comment = comment_temp2;
				 } // End of insert '//' 

				vvsPATTERNS.back().m_listComments->push_back("// " + comment);
			}
		}	
	}
}

void addPattern(string pname) {


	Pattern *pat = new Pattern;
	int index;
	string name, type;

	index = vvsPATTERNS.size();
	name = pname.substr(0, pname.find(","));
	type = pname.substr(pname.find(",")  + 1);
	type = trim(type);

	// Check for empty pattern type field - TAZ 10/27/2017
	// Set type to "mixedsync" if empty.
	int checkType = pname.find(",");
	if (checkType == -1) {
		//cout << checkType << endl;
		type = "mixedsync";
	}

	pat->orig_name = "PATTERN(" + name + ", logic)";
	pat->setName(name);
	pat->setIndex(index);
	pat->type = type;

	Log("index",toString(index));
	Log("name",name);
	//_getch(); 
	

	vvsPATTERNS.push_back(*pat);
	if (index == 132) {
		int stop = 1;
	}
}

void addSubVecDef(string vecdef){
	vvsPATTERNS.back().setSubVecDef(vecdef);

}

void addPatInstVecDef(string vecdef) {
	//PatInst *inst;
	//PatInst inst->setVecDef(vecdef);
}

void addPatInst(string patinst,string vecdef){
	PatInst *inst =  new PatInst;
	
	istringstream ss(patinst);
	string rd;
	string label;
	int index;
	vector<string> lines;
	inst->setVecDef(vecdef);

	while (getline(ss, rd, '\n')) {
		rd = ltrim(rtrim(remove_comment(rd)));
		#ifdef dbg_microinst
			cout <<"rd : "<< rd << endl;	
		#endif
	    // Place Vec multi site filter here. Filter case example: "% VEC (1:1x011)". - TAZ 07/11/2018
			rd = filterMicroInst("VEC_RPT_Multi_SITE",rd);

		if( rd.size() > 1)
			lines.push_back(rd);
    }
	//inst->setSubVec(rd);
	inst->setOrigLines(patinst);

	index = vvsPATTERNS.back().getListSize(); // index
	
	Log("index",toString(index));

	int idx = lines[0].find(':');  // label
	
	if( idx != -1){ // if has label
		label =  lines[0].substr(0 , lines[0].find_last_not_of(":")+1  );
		label = ltrim(rtrim(label));
		inst->setLabel(label);
		lines.erase(lines.begin());
	}
	Log("label",label);

	for(size_t i = 0; i < lines.size(); i++){
		inst->addMicroInst(lines[i]);
	}

	inst->setIndex(vvsPATTERNS.back().getListSize());
	vvsPATTERNS.back().addPatInst(*inst);
	addComment("%");
	
}
void PutPatternReq(string func,string param){
	int idx;
	idx = param.find(")");

	if(idx != -1)
		param = trim(param.substr(0,idx));
		 
		if(FindDef(param))
			vvsPATTERNS.back().addPatternReq(func + "=" +param);
		else
			vvsPATTERNS.back().addPatternReq(func);
}

void ProcessInitBlock(string init){
	Log("ProcessInitBlock",init);

	istringstream ss(init);
	string rd;
	string label;
//	int index;
    vector<string> lines;
    while (getline(ss, rd, '\n')) {
		rd = trim(remove_comment(rd));

		if( rd.size() > 1){
 
			vector<string> v = Tokenize(rd,'(');
  		    string func = trim(v.at(0));
			string param;
//			int idx;
			if(v.size() > 1 )
				param = trim(v.at(1));
		
			if(func.compare("numx")==0){
				PutPatternReq(func,param);
			}
			else if(func.compare("numy")==0){
				PutPatternReq(func,param);
			}
			else if(func.compare("x	")==0){
				PutPatternReq(func,param);
			}				
			else if(func.compare("xbase")==0){
				PutPatternReq(func,param);
			}
			else if(func.compare("xfield")==0){
				PutPatternReq(func,param);
			}
			else if(func.compare("ymain")==0){
				PutPatternReq(func,param);
			}
			else if(func.compare("ybase")==0){
				PutPatternReq(func,param);
			}
			else if(func.compare("yfield")==0){
				PutPatternReq(func,param);
			}
			else if(func.compare("jamreg")==0){
				PutPatternReq(func,param);
			}
			else if(func.compare("datreg")==0){
				PutPatternReq(func,param);
			}
			else if(func.compare("count")==0){
				vector<string> t = Tokenize(param,',');
  				if(t.size() > 1){
					string val = t.at(1);
					int idx = val.find_last_of(")");
					if(idx != -1)
						val = trim(val.substr(0,idx));
					string cntr = func + t.at(0);
					unsigned long num;
					if(val.find("(") != -1)
						vvsPATTERNS.back().addPatternReq(cntr);
					else if(StrTol(val,num))
						vvsPATTERNS.back().addPatternReq(cntr + "=" + val);
					else if(FindDef(val))
						if(val.find("(") == -1)
							if(StrTol(val,num))
								vvsPATTERNS.back().addPatternReq(cntr + "=" + val);
						else
							vvsPATTERNS.back().addPatternReq(cntr  + " //" + val);
					else
						vvsPATTERNS.back().addPatternReq(cntr);
					
					}

			}	

		}
			
    }
}

void ParsePatNames(string directory){
	
	ifstream infile;
	if (directory.size() == 0) {
		directory = "input_files\\";
	}

//	infile.open( inputFileName + "\\exec_pat_names.txt");
	infile.open(directory+"exec_pat_names.txt");

	string rd;
	int idx = 0;
	while(getline(infile, rd)){
		rd=ltrim(rtrim(rd));
			if(rd.size() > 0){
				vsEXECPATNAMES.push_back(rd);
			}

	}

	infile.close();
	
}

void ParseVecDef(string directory){
	if (directory.size() == 0) {
		directory = "input_files\\";
	}

	
	ifstream infile;

//	infile.open( inputFileName + "\\vec_def.txt");
	infile.open(directory+"vec_def.txt");

	string rd;
	vector<string> strs;
	int idx = 0;
	G_VECDEF += "%%VECDEF\t";
	while(getline(infile, rd)){
		rd=trim(rd);
		if(idx != 0)
			if(idx%8 == 0)
				G_VECDEF += "\n\t\t\t";

			if(rd.length() > 0){
				if(rd.find("=") != -1){
				strs = Tokenize(rd,'=');
				rd = strs.at(0);
				def_char_pair * dcp = new pair<int,char>(idx, trim(strs.at(1)).at(0)); 
				vpairDEFCHAR.push_back(*dcp) ;
				//stdmapDEFCHAR[idx]=trim(strs.at(1)).at(0);
				//G_VECDEF += rd +", ";
				}
				stdmapVECDEF[rd] = idx;
				G_VECDEF += rd +", ";
				idx++;
			}

	}

	G_VECDEF += G_DEFAULT_PIN_STATE; // reverted fix for issue 19
	//G_VECDEF = G_VECDEF.substr(0,G_VECDEF.find_last_of(",")); // fix for issue 19 
	//G_VECTOR_LINE.resize(idx,'X'); email wed, dec 14, 2016
	G_VEC_LINE_SIZE =  idx ; //+ 1;
	infile.close();
	
}

void SetBitDefaults(string &vec_line){
	for(vector< pair<int,char> >::iterator it = vpairDEFCHAR.begin(); it != vpairDEFCHAR.end(); ++it)
		vec_line[it->first]=it->second;
}


void ParseConfigs(string directory){
	if (directory.size() == 0) {
		directory = "input_files\\";
	}

	ifstream in(directory+"config.txt", ios::in | ios::binary);

//	  if(!in) {
//		  ifstream in("input_files\\config.txt", ios::in | ios::binary);
//		  inputFileName = "input_files";

		  if (!in) {
			   setTextColor(12);
			  LogError("ParseConfigs", "Cannot open input file - config.txt.");
			   setTextColor(10);
			  if (G_HALT_E) {
				  cout << "Hit any key to continue" << endl;
				  _getch();
			  }
			  return;
//		  }
	  }
	 
//	  int idx;
	  string strd;
	  string patname;
	  string regs;
	  
	  do{

		 getline( in, strd);

		 if(strd.length()  < 1 )
			 continue;

vector<string> pair = Tokenize(strd,'=');
		
		string key = trim(pair.at(0));

		if(pair.size() < 2){
			 setTextColor(12);
			LogError("ParseConfigs : ", key + " not set.");
			 setTextColor(10);
			if (G_HALT_E) {
				cout << "Hit any key to continue" << endl;
				_getch();
			}
			continue;
		}

		ToUpper(key);
		if(key.compare("PIN_SCRAMBLE")== 0){
			G_PINSCRAMBLE_NAME = trim(pair.at(1)) ;
		}
		else if(key.compare("DEFAULT_PIN_STATE")== 0){
			string ch = trim(pair.at(1));
			if(ch.length()> 0)
			G_DEFAULT_PIN_STATE = ch.at(0);
		

		}
//		else if ((key.compare("INPUT_FILE") == 0) || (key.compare("INPUT_FILES") == 0)) {
//			string ch = trim(pair.at(1));
//			if (ch.length()> 0)
//				G_INPUT_FILE = ch;
//				inputFileName = ch;
//		}

		else if(key.compare("CS_ACTIVE_HIGH")== 0){
			vector<string> cs = Tokenize( trim(pair.at(1)),',');
			for(int c = 0; c < 8; c++){ // set all to false
					G_CS_ACTIVE_H[c]=false;
			}
			for(size_t c = 0; c < cs.size(); c++){
				string t_cs = cs.at(c);
				ToUpper(t_cs);
				if(t_cs.find("T_CS") != 1){
					int cs_num = atoi( t_cs.substr(4).c_str() );
					if(cs_num > 0 && cs_num <= 8){
						G_CS_ACTIVE_H[cs_num-1] = true;
						Log("ParseConfigs", "Set to true : t_cs" + toString(cs_num));

					}
					else
						 setTextColor(12);
						LogError("ParseConfigs", "Invalid t_cs number : " + toString(cs_num));
						 setTextColor(10);

						if (G_HALT_E) {
							cout << "Hit any key to continue" << endl;
							_getch();
						}

				}
				else{
					 setTextColor(12);
					LogError("ParseConfigs", "Invalid tester function : " + t_cs);
					 setTextColor(10);
					if (G_HALT_E) {
						cout << "Hit any key to continue" << endl;
						_getch();
					}
				}
			}
		}
		else
		{
		 setTextColor(12);
		 LogError("ParseConfigs", "Unknown variable : " + key);
		  setTextColor(10);
		 if (G_HALT_E) {
			 cout << "Hit any key to continue" << endl;
			 _getch();
		 }
		}
		 
	  }
	  while(!in.eof() );

	  in.close();
	
}



void ParsePatInits(string directory){
	if (directory.size() == 0) {
		directory = "input_files\\";
	}
//	ifstream in(inputFileName + "\\per_pat_inits.txt", ios::in | ios::binary);
	ifstream in(directory+"per_pat_inits.txt", ios::in | ios::binary);

	  if(!in) {
		  setTextColor(12);
		LogError("ParsePatInits", "Cannot open input file - per_pat_inits.txt.");
		 setTextColor(10);
		_getch();
		return;
	  }
	 
	  int idx;
	  string strd;
	  string patname;
	  string regs;
	  
	  do{

		 getline( in, strd);

		 if(strd.length()  < 1 )
			 continue;

		 idx = strd.find("[");
		 
		 if(idx == 0){
			 if(patname.length() > 0){
				stdmapPATINITS[patname] = regs;
				Log("stdmapPATINITS[" + patname + "] = ", stdmapPATINITS[patname] );
				if(G_DEBUG_ON)
					_getch();
				
			 }
 
			regs = "";
			int start = strd.find_first_of("[")+1; 
			patname = strd.substr(start,strd.find_first_of("]")-1 ) ;
		 }
		 else{
			 if(regs.length() > 0)
				regs += "|";
			regs += trim(remove_comment(strd));
			//Log("ParsePatInits","regs = " + regs);
		//	_getch();
		 }
		 
	  }
	  while(!in.eof() );

	   stdmapPATINITS[patname] = regs;
	   Log("stdmapPATINITS[" + patname + "] = ", stdmapPATINITS[patname] );

	  in.close();
	
}

void TitleBox(string type) {
	if (!HEADER_GRAPHIC) { return; }
	//SetConsoleOutputCP(437);

	unsigned char d = 0xBB;
	unsigned char e = 0xBC;
	unsigned char f = 0xC8;
	unsigned char g = 0xC9;
	unsigned char h = 0xCD;
	unsigned char i = 0xBA;
	unsigned char j = 0xCC;
	unsigned char k = 0xB9;
	unsigned char l = 0xCB;
	int width = 80;
	int width2 = 18;

	if (type == "TOP") {
		cout << " ";
		cout << g;
		for (int ii = 0; ii <= width; ii++)
			cout << h;
		cout << d << " \n";
	}

	if (type == "DIVIDER") {
		cout << " ";
		cout << j;
		for (int ii = 0; ii <= width; ii++)
			cout << h;
		cout << k << " \n";
	}

	if (type == "SIDE1") {
		cout << " ";
		cout << i;
	}

	if (type == "SIDE2") {
		cout << " ";
		cout << i;
			for (int ii = 0; ii <= width; ii++)
				cout << " ";
			cout << i << " \r";
	}

	if (type == "BOTTOM") {
		cout << " ";
		cout << f;
		for (int ii = 0; ii <= width; ii++)
			cout << h;
		cout << e << " \n";
	}

	if (type == "SIDE1B") {
		cout << " ";
		cout << i;
		for (int ii = 0; ii <= width2; ii++)
			cout << " ";
		cout << i << " \r";
	}

	if (type == "SMALLBOTTOM") {
		cout << " ";
		cout << f;
		for (int ii = 0; ii <= width2; ii++)
			cout << h;
		cout << e << " \n";
	}

	if (type == "DIVIDER2") {
		cout << l;
		for (int ii = 0; ii <= width2; ii++)
			cout << h;
		cout << k << "\n";
	}


}

void setTextColor(int colorNum) {
	if (!HEADER_GRAPHIC) { return; }
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
	return;
}

void setCursorPosition(unsigned int x, int y) {
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
#pragma warning( disable : 4838 )
	COORD pos = { x, y };
	SetConsoleCursorPosition(output, pos);

}

void ParseIncludeFile(string fname){
	directory;
	if (directory.size() == 0) {
		directory = "input_files\\";
	}
	Log("ParseIncludeFile", fname );
	//_getch();
	if(fname.find("\"") != -1){
		fname = fname.substr(fname.find_first_of("\"")+1);
		fname = fname.substr(0,fname.find_first_of("\""));
	}
//	fname = inputFileName + "\\" + fname;
	fname = directory + fname;

	if(fname.find("define") != -1){
		ParseDefineFile(directoryOut, fname);
		UpdateDefinitions();
	}
}
int ParsePatternFiles(string directory){
	if (directory.size() == 0) {
		directory = "input_files\\";
	}
	if (directoryOut.size() == 0) {
		directoryOut = "output_files\\";
	}

vector<string> filesPaths_pre;
string optfileName ="";        
//string inputFolderPath =inputFileName + "\\"; 
string inputFolderPath = directory;
string extension = "*.pat";

//Check for Gosub to Label, Jump to Label. - TAZ 06/30/2018
getFilesList(inputFolderPath,extension,filesPaths_pre);
int temp = filesPaths_pre.size();
unsigned char a = 0xB1, b = 0xDB, c = 0xB9;


if (HEADER_GRAPHIC) {
	 setTextColor(31);
	setCursorPosition(5, 7);
	cout << "PreParse Check" << endl;
	 setTextColor(30);
	setCursorPosition(5, 8);
	for (int i = 0; i <= temp * 1; i++)
		cout << a;

	 setTextColor(31);
	setCursorPosition(5, 10);
	cout << "Parsing Pattern Files(s)" << endl;
	 setTextColor(27);
	setCursorPosition(5, 11);
	for (int i = 0; i <= temp * 1; i++)
		cout << a;



	 setTextColor(30);
	setCursorPosition(5, 8);
}
vector<string>::const_iterator it = filesPaths_pre.begin();
while( it != filesPaths_pre.end())
{
	int ret2 = ParseGosubLabel(*it, filesPaths_pre);
    it++;
	if (HEADER_GRAPHIC) {
		cout << b;
		//cout << b;
	}
}
if (HEADER_GRAPHIC) {
	cout << b;
	//cout << b;
}
vector<string> filesPaths;
getFilesList(inputFolderPath, extension, filesPaths);
vector<string>::const_iterator it2 = filesPaths.begin();
if (HEADER_GRAPHIC) {
	setCursorPosition(5, 11);
	 setTextColor(27);
}
while (it2 != filesPaths.end())
{
	int ret = ParsePatternFile(directoryDebug, *it2);
	if (ret == -1)
		return ret;
	//    frame = imread(*it);//read file names
	//doyourwork here ( frame );
	//  sprintf(buf, "%s/Out/%d.jpg", optfileName.c_str(),it->c_str());
	//imwrite(buf,frame);   
	it2++;
	if (HEADER_GRAPHIC) {
		cout << b;
		//cout << b;
	}
}
if (HEADER_GRAPHIC) {
	cout << b;
	//cout << b;
	 setTextColor(26);
}
return 0;
}

void ParseDefines(string directory){
	if (directory.size() == 0) {
		directory = "input_files\\";
	}

vector<string> filesPaths;
string optfileName ="";        
//string inputFolderPath =inputFileName + "\\defines\\"; 
string inputFolderPath = directory+"defines\\";

string extension = "*.h*";
getFilesList(inputFolderPath,extension,filesPaths);
vector<string>::const_iterator it = filesPaths.begin();
while( it != filesPaths.end())
{
	cout << it->c_str() << endl;
//    frame = imread(*it);//read file names
        //doyourwork here ( frame );
  //  sprintf(buf, "%s/Out/%d.jpg", optfileName.c_str(),it->c_str());
    //imwrite(buf,frame);   
    it++;
}
}


bool UnScrambleResource(string res,vector<int> &idxs, string ps_map){
	
	int ps_idx = -1;
	int map_idx = -1;
	bool ret = false;

	Log("UnScrambleResource","resource = " + res + "\r\n");

	for(size_t c = 0; c < vPINSCRAMBLES.size() ; c++){
	
		Log("UnScrambleResource", vPINSCRAMBLES.at(c).name + " == " + G_PINSCRAMBLE_NAME + "\r\n"); 
			
		if(vPINSCRAMBLES.at(c).name.compare(G_PINSCRAMBLE_NAME) == 0){

			ps_idx = c;
			
		}
	}
	if( ps_idx == -1){
		 setTextColor(12);
		LogError("UnScrambleResource","pin_scramble " + G_PINSCRAMBLE_NAME + " not found." );
		 setTextColor(10);
		_getch();
		return false;
	
	}

	PinScramble  ps = vPINSCRAMBLES.at(ps_idx);

	for(size_t c = 0; c < ps.m_vScrambleMap->size()  ; c++){
	
		
		if(ps.m_vScrambleMap->at(c).name.compare(ps_map)  == 0){
			Log("UnScrambleResource", ps.m_vScrambleMap->at(c).name + " is equal to " + ps_map + "\r\n");
			map_idx = c;
			break;
		}
	}

	if( map_idx == -1)
		return false;

	ScrambleMap smap = ps.m_vScrambleMap->at(map_idx);

	int idx;
	for(map<string,string>::iterator it = smap.m_mapScramble->begin() ; it != smap.m_mapScramble->end(); ++it)
	{
		
		string str_up = it->second;
		// Insert macro check here. - TAZ 05/30/2018
		if (FindDef(str_up)) {
			if (G_DEBUG_ON)
			debugfile << "macro for pin value! : " << it->second << " = " << str_up << endl;
		}
		ToUpper(str_up);
		if (G_DEBUG_ON)
		debugfile <<  res << " = " << it->second << " = " << it->first << endl;

		if(res.compare(str_up) == 0){
			if( FindVecDefIndex(it->first,idx) ){
				ret = true;
				idxs.push_back(idx);
				if (G_DEBUG_ON)
				debugfile <<"found in vecdef! : "<<  res << " = " << it->second << " = " << it->first << endl;
				
			}
		}

	}

	return ret;
  }

void NestLoopCheck(string temp, string patName) {
	// ******** Added for Nested Loop Violation. - TAZ 06/07/2018 *************
	//regex keywords_startloop("^\\s*(STARTLOOP)");
	//regex keywords_endloop("^\\s*(ENDLOOP)");
	regex keywords_startloop("^\\s*(STARTLOOP)", std::regex_constants::icase);
	regex keywords_endloop("^\\s*(ENDLOOP)",std::regex_constants::icase);
	smatch match;
	//ToUpper(temp);
	bool found_startloop = regex_search(temp, match, keywords_startloop);
	bool found_endloop = regex_search(temp, match, keywords_endloop);

	if (found_startloop)
	{
		Nested_Loop = Nested_Loop + 1;
	}
	if (found_endloop)
	{
		Nested_Loop = Nested_Loop - 1;
	}
	if (Nested_Loop > NESTED_LOOP_ALLOWED) {
		Loop_Violation = true;
		Nested_Loop = 0; // reset nested loop counter
		Loop_Pattern_List = Loop_Pattern_List +"\n  " +patName;
		//cout << "Encountered a nested loop violation " << Nested_Loop << " for pattern: " << pattern.name << "."<< endl;
	}
}


void Initialize(){
	if (directoryDebug.size() == 0) {
		directoryDebug = "debug_files\\";
	}
	//if (G_DEBUG_ON)
	debugfile.open(directoryDebug+"debug.txt");
	errlogfile.open (directoryDebug+"error_logs.txt");
	warnlogfile.open (directoryDebug+"warning_logs.txt");

//	if (G_INPUT_FILE.size() < 1) {
//		G_INPUT_FILE = "input_files";
//	}


	
/*	for(int c = 0; c < G_COUNTERS.size(); c++){
		G_COUNTERS[c] = c + 1;
		G_COUNTERS_RLD[c] = G_COUNTERS[c];
	}
*/	
	ParseConfigs(directory);
	
	ParsePatInits(directory);

	ParsePatNames(directory);
	
	ParseVecDef(directory);
	
	ParseScrambleFile(directory,"pin_scramble.cpp");
}

void WriteVecPattern(string dirout, Pattern &pattern){
	if (dirout.size() == 0) {
		dirout = "output_files\\";
	}

	ofstream outfile;
	string fname = dirout;
	fname += pattern.name + ".vec";
	outfile.open(fname.c_str());
	Nested_Loop = 0;
	for (  vector<string>::iterator it= pattern.m_listVecConv->begin(); it != pattern.m_listVecConv->end(); ++it){
		Log("WriteVecPatterns", *it);

			NestLoopCheck(*it, pattern.name);

			outfile << *it << endl;
	}

		outfile.close();
}

void WriteVecPatterns(){
	
	//for (  vector<Pattern>::iterator it = vvsPATTERNS.begin()+1; it != /*vvsPATTERNS.begin() */ vvsPATTERNS.end(); ++it,++i){
		//cout << i << ":" << *it << '\n';
	//	WriteVecPattern(*it); 
	//}

	int i=0;
	int xpos = 0, ypos = 0;
	unsigned char a = 0xB1, b = 0xDB;
	int pbarCol = 0;
	int pbarRow = 17;
	int barIndex;
	rowSize = rowSize - 2;
	if (HEADER_GRAPHIC) {
		 setTextColor(20);
		setCursorPosition(5, 17);
		for (size_t i = 0; i < vsEXECPATNAMES.size(); i++) {
			barIndex = i;
			if ((barIndex == 75) || (barIndex == 150) || (barIndex == 225) || (barIndex == 300)  || (barIndex == 375)  || (barIndex == 450)) {
				pbarCol = pbarCol - 75;
				pbarRow = pbarRow + 1;
			}

			setCursorPosition(5 + i + pbarCol, pbarRow + rowSize);
			cout << a;
		}
		setCursorPosition(XPosLast, YPosLast);
	}

	pbarCol = 0;
	pbarRow = 17;
	barIndex;
	for(size_t  c = 0; c < vsEXECPATNAMES.size(); c++){
		barIndex = c;
		for (  vector<Pattern>::iterator it = vvsPATTERNS.begin(); it != vvsPATTERNS.end(); ++it,++i){
				//cout << i << ":" << *it << '\n';
			string name =  trim(it->name);

			if( vsEXECPATNAMES.at(c).compare(name) == 0 ){
				//cout << "  Writing " << name << endl;
				WriteVecPattern(directoryOut,*it);
				if (HEADER_GRAPHIC) {
					xpos, ypos = currentCurPos();
					//cout << xpos <<  " : " << ypos << endl;
					if ((barIndex == 75) || (barIndex == 150) || (barIndex == 225) || (barIndex == 300)  || (barIndex == 375)  || (barIndex == 450)) {
						pbarCol = pbarCol - 75;
						pbarRow = pbarRow + 1;
					}
					setCursorPosition(5 + c + pbarCol, pbarRow + rowSize);
					cout << b;
					setCursorPosition(xpos, ypos);
					//cout << xpos << " : " << ypos << endl;
				}
				break;
			}
		}                              
	}

}

int FindFromPatterns(string pat_name){

	int i = 0;
	for (  vector<Pattern>::iterator it = vvsPATTERNS.begin(); it != vvsPATTERNS.end(); ++it,++i){
		string name =  trim(it->name);
		if( pat_name.compare(name) == 0 ){
			return i-1; 
		}
	}              

	return -1;
}

void ConvertPatterns(){
	
	int i=0;
	int xpos=0, ypos=0;
	unsigned char a = 0xB1 ,b = 0xDB;

	if (HEADER_GRAPHIC) {
		int barIndex = 0;
		int pbarRow = 14;
		int pbarCol = 0;

		 setTextColor(10);
		setCursorPosition(5, pbarRow);
		for (size_t i = 0; i < vsEXECPATNAMES.size(); i++) {
			barIndex = i;
			if ((barIndex == 75) || (barIndex == 150) || (barIndex == 225) || (barIndex == 300)  || (barIndex == 375)  || (barIndex == 450)) {
				pbarCol = pbarCol - 75;
				pbarRow = pbarRow + 1;
			}

			setCursorPosition(5 + i + pbarCol, pbarRow);
			cout << a;
			//setCursorPosition(5, pbarRow);
		}
		setCursorPosition(0, (23+ rowSize*2)-4);
	}

	if(G_MODE ==  MODE1)
	vvsPATTERNS.back().LoadPatInits("_global_");
	int barIndex = 0;
	int pbarRow = 14;
	int pbarCol = 0;
	for(size_t  c = 0; c < vsEXECPATNAMES.size(); c++){
		barIndex = c;

		for (  vector<Pattern>::iterator it = vvsPATTERNS.begin(); it != vvsPATTERNS.end(); ++it,++i){
			string name =  trim(it->name);

			if( vsEXECPATNAMES.at(c).compare(name) == 0 ){

				it->Convert(directoryOut);
				cout <<"  Converted " << name  << endl;
				//it->printdata(outfile);
				if (HEADER_GRAPHIC) {
					xpos, ypos = currentCurPos();
					if ((barIndex == 75) || (barIndex == 150) || (barIndex == 225) || (barIndex == 300)  || (barIndex == 375)  || (barIndex == 450)) {
						pbarCol = pbarCol - 75;
						pbarRow = pbarRow + 1;
					}

					//cout << xpos <<  " : " << ypos << endl;
					setCursorPosition(5 + c  +pbarCol, pbarRow);
					cout << b;
					setCursorPosition(xpos, ypos);
					//cout << xpos << " : " << ypos << endl;
				}
				break;
			}
		} 
	}

	XPosLast = xpos;
	YPosLast = ypos;
}

void CleanUp(){

	G_COUNTERS.clear();
	G_COUNTERS_RLD.clear();

}


void WritePatternReqs(){

ofstream ofile;

//ofile.open(inputFileName + "\\per_pat_inits.txt");
ofile.open("input_files\\per_pat_inits.txt");


	int i=0;
	
	ofile <<"[_global_]" << endl;
	for(size_t  c = 0; c < vsEXECPATNAMES.size(); c++){
		for (  vector<Pattern>::iterator it = vvsPATTERNS.begin(); it != vvsPATTERNS.end(); ++it,++i){
			string name =  trim(it->name);

			if( vsEXECPATNAMES.at(c).compare(name) == 0 ){
				ofile <<"[" << name << "]" << endl;	
				for(size_t  c = 0; c < it->m_listReqReg->size(); c++){
					ofile << it->m_listReqReg->at(c) << endl;
		
				}
				
				break;
			}
		}                              
	}


	

ofile.close();

}

bool dirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

void CreateFolder(const char * path)
{
	if (!CreateDirectory(path, NULL))
	{
		return;
	}
}

string RemoveChar(string string_in, string remove_char) {
	for (size_t c = 0; c < remove_char.size(); c++) {
		string_in.erase(std::remove(string_in.begin(), string_in.end(), remove_char.at(c)), string_in.end());
	}
	return string_in;
}

time_t cvt_time(char const *time) { 
    char s_month[5];
    int month, day, year;
    struct tm t = {0};
    static const char month_names[] = "JanuaryFebuaryMarchAprilMayJuneJulyAugustSeptemberOctoberNovemberDecember";

#pragma warning( disable : 4996 )
    sscanf(time, "%s %d %d", s_month, &day, &year);

    month = (strstr(month_names, s_month)-month_names)/3;

    t.tm_mon = month;
    t.tm_mday = day;
    t.tm_year = year - 1900;
    t.tm_isdst = -1;

    return mktime(&t);
}

string filterMicroInst(string type, string uInst) {
	bool foundvecMultiSite;
	smatch match15;
	string strTest = " VEC  (1:XXXX XXXX  0XX0X  XXX)";
	string Space = " ";
	strTest = RemoveChar(strTest, Space);
	//regex vecMultiSite_keywords("(^\\s*%*\\s*VEC\\s*)(\\(\\s*\\d+\\s*:)(\\s*\\w*\\s*\\w*\\s*)(\\)\\s*,*)(\\s*\\w*.*)", std::regex_constants::icase);
	regex vec_rpt_MultiSite_keywords("(^\\s*%*\\s*(VEC\\s*|RPT\\s*\\d+)\\s*)(\\(\\s*\\d+\\s*:)(.*)(\\)\\s*,*)(\\s*\\w*.*)", std::regex_constants::icase);
	foundvecMultiSite = regex_search(uInst, match15, vec_rpt_MultiSite_keywords);
	string uInstFiltered;

	if ((type == "VEC_RPT_Multi_SITE") && (foundvecMultiSite)) {
			if (match15[6].str() != "") {
				uInstFiltered = match15[1].str() + Space + match15[4].str() + Space + "," + match15[6].str();
			}
			else {
				uInstFiltered = match15[1].str() + Space + match15[4].str();
			}
		return uInstFiltered;
	}
	return uInst;
}

void buildHeader(string section, int rSize) {
	if (!HEADER_GRAPHIC) { return; }
	if (MAX_WINDOW) { MoveWindow(hWnd, NewWidth, NewHeight, WindowWidth, WindowHeight, TRUE); }
	setCursorPosition(0, 0);
	LONG style = GetWindowLong(hWnd, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX);
	SetWindowLong(hWnd, GWL_STYLE, style);
	rowSize = rSize;
	const time_t ONE_DAY = 24 * 60 * 60;
	const char *buildDate = __DATE__;
	const char *buildTime = __TIME__;
	if (section == "TOP") {
		 setTextColor(31);
		TitleBox("TOP");
		TitleBox("SIDE2");
		TitleBox("SIDE1");
		cout << "\n";

		TitleBox("DIVIDER");
		TitleBox("SIDE2");
		TitleBox("SIDE1");
		cout << "\n";

		TitleBox("SIDE2");
		TitleBox("SIDE1");
		cout << "\n";

		TitleBox("BOTTOM");
		setCursorPosition(2, 1);
		cout << "\t\t\tAPG Tool Build Date: " << buildDate << " at " << buildTime << endl;

		setCursorPosition(0, 6);
		//PreParse Check 
		 setTextColor(30);
		//system("color 0e");
		unsigned char a = 0xB1, b = 0xDB, c = 0xB9;
		TitleBox("TOP");
		TitleBox("SIDE2");
		cout << "\r";
		TitleBox("SIDE1");
		cout << "\n";

		TitleBox("SIDE2");
		TitleBox("SIDE1");
		cout << "\n";

		TitleBox("DIVIDER");
		 setTextColor(27);
		//Parse Patterns 
		TitleBox("SIDE2");
		cout << "\r";
		TitleBox("SIDE1");
		cout << "\n";

		TitleBox("SIDE2");
		cout << "\r";
		TitleBox("SIDE1");
		cout << "\n";

		TitleBox("BOTTOM");
	}
	//Converting Patterns 
	if (section == "BOTTOM") {
		int numPats = vsEXECPATNAMES.size();
		int remainder = numPats % 75;
		rowSize = 1 + (numPats / 75);
		if (remainder > 0) { rowSize = rowSize + 1; }

		setCursorPosition(0, 12);
		TitleBox("DIVIDER");

		 setTextColor(26);

		for (size_t rSize = 0; rSize < rowSize; rSize++) {
			TitleBox("SIDE2");
			cout << "\r";
			TitleBox("SIDE1");
			cout << "\n";
		}

		TitleBox("DIVIDER");
		 setTextColor(20);

		for (size_t rSize = 0; rSize < rowSize; rSize++) {
			TitleBox("SIDE2");
			cout << "\r";
			TitleBox("SIDE1");
			cout << "\n";
		}

		TitleBox("BOTTOM");
		int curXPosLast=0;
		int curYPosLast=0;
		 curXPosLast,  curYPosLast = currentCurPos();

		//cout << "\n";
		setCursorPosition(63, curYPosLast -1);
		TitleBox("DIVIDER2");
		setCursorPosition(62, curYPosLast);
		TitleBox("SIDE1B");
		setCursorPosition(62, curYPosLast +1);
		TitleBox("SMALLBOTTOM");
		setCursorPosition(64, curYPosLast);
		 setTextColor(31);
		cout << " Program Rev: " << REVISION << endl;
		cout << "\n\n";
		 setTextColor(10);

		 setTextColor(31);
		setCursorPosition(5, 13);
		cout << "Converting Pattern(s)" << endl;

		 setTextColor(31);
		setCursorPosition(5, 16 + (rowSize-2));
		cout << "Writing Pattern(s)" << endl;
		 setTextColor(27);
	}
}

void ShowConsoleCursor(bool showFlag)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO     cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = showFlag; // set the cursor visibility
	SetConsoleCursorInfo(out, &cursorInfo);
}

void consoleGraphic(){
	/* Window size coordinates, be sure to start index at zero! */
	SMALL_RECT windowSize = { 0, 0, 69, 34 };

	/* A COORD struct for specificying the console's screen buffer dimensions */
	COORD bufferSize = { 70, 35 };

	/* Setting up different variables for passing to WriteConsoleOutput */
	COORD characterBufferSize = { 2, 2 };
	COORD characterPosition = { 1, 0 };
	SMALL_RECT consoleWriteArea = { 0, 1, 0, 1 };

	/* A CHAR_INFO structure containing data about a single character */
	CHAR_INFO characterQ;
	characterQ.Char.AsciiChar = 205; /* Setting the Char.Ascii data member of characterQ to the value of 'Q' */

									 /* Setting up the color values for our Q character: blue + green + intensity */
	characterQ.Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | 0x0040 |
		FOREGROUND_INTENSITY;


	/* initialize handles */
	wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
	rHnd = GetStdHandle(STD_INPUT_HANDLE);

	/* Set the console's title */
	SetConsoleTitle("Our shiny new title!");

	/* Set the window size */
	SetConsoleWindowInfo(wHnd, TRUE, &windowSize);

	/* Set the screen's buffer size */
	SetConsoleScreenBufferSize(wHnd, bufferSize);

	/* Write our character buffer (a single character currently) to the console buffer */
	WriteConsoleOutputA(wHnd, &characterQ, characterBufferSize, characterPosition, &consoleWriteArea);

	getchar();


}
int main(int argc, char* argv[])
{
const time_t ONE_DAY = 24 * 60 * 60 ;
const char *buildDate = __DATE__ ;

int start = clock();


//double diff;
//############################################################
//Regex Experiment
//############################################################
	//regex keywords("([tyarnold1]+)@(\\w+)\.com");
	// \w => \\w  \s => \\s \d => \\d 
	//regex keywords("^\\s*(STARTLOOP)");
	//regex keywords("(mar)+\\s*(gosub)+\\s*,+\\s*(\\w+)\\s*,*\\s*(\\w*)");
	//regex keywords("%+\\s*(\\w+)\\s*:+");
	//smatch match;
	//ToUpper(input);
	//bool found = regex_search(input, match, keywords);
	//for (size_t i = 0; i < match.size(); ++i) {
	//	std::cout << i << ": '" << match[i].str() << "'\n";
	//}
	//cout << input << endl;
	/*if (found)
		{
		cout << "Found!" << endl;
		cout << regex_search(input, keywords) << endl;
		}
	 	else
		{
		cout << "Not found" << endl;
		cout << regex_search(input, keywords) << endl;
		}*/
//############################################################

bool check_version = false;
LPCSTR lpcMessage;
LPCSTR lpMyString;
string message;
Custom_Input = false;
Custom_Output = false;
Custom_DebugOutput = false;
for (int a = 0; a < argc; a++) {
	string arg(argv[a]);
	if (arg.compare("/req") == 0)
		G_MODE = MODE2;
	else if (arg.compare("/version") == 0)
		check_version = true;
	else if (arg.compare("/errnohalt") == 0)
		G_HALT_E = false;
	else if (arg.compare("/warnnohalt") == 0)
		G_HALT_W = false;
	else if (arg.compare("/no_orig_code") == 0)
		G_ADD_ORIG = false;
	else if (arg.compare("/debug_on") == 0)
		G_DEBUG_ON = true;
	else if (arg.compare("/verbose") == 0)
		G_VERBOSE = true;
	else if (arg.compare("/usage") == 0) {
		G_USAGE = true;
		HEADER_GRAPHIC = false;
	}
	else if (arg.compare("/gui") == 0)
		G_GUI = true;
	else if (arg.compare("/alu_warn") == 0)
		G_WARN_ALU = true;
	else if (arg.compare("/alu_on") == 0)
		G_ALU_ON = true;
	else if (arg.compare("/chg_log") == 0) {
		G_CHANGELOG = true;
		HEADER_GRAPHIC = false;
	}
	else if (arg.compare("/grph_off") == 0)
		HEADER_GRAPHIC = false;
	else if (arg.compare("/dirin") == 0) {
		directory = RemoveChar(argv[a + 1], " \r\n");
		if (!dirExists(directory)) {
			MessageBox(NULL, "Input Directory does not exist.\nPlease create input_files directory.", "", MB_ICONWARNING | MB_OK);
			return 0;
		}
		if (HEADER_GRAPHIC) {
			setCursorPosition(2, 3);
		}
		cout << "   Input Directory: " << argv[a + 1] << "\n";
		directory = directory + "\\";
		Custom_Input = true;
	}
	else if (arg.compare("/dirout") == 0) {
		directoryOut = RemoveChar(argv[a + 1], " \r\n");
		directoryOut = directoryOut;
		//cout << "Directory Exits :" << directoryOut << dirExists(directoryOut) << "\n";
		if (!dirExists(directoryOut)) {
			message = "Create Directory: " + directoryOut + "?\n[Yes] Creates Directory\n[No] Exits program.";
			lpcMessage = message.c_str();
			lpMyString = directoryOut.c_str();

			int msgboxID = MessageBox(NULL, lpcMessage, "", MB_ICONWARNING | MB_YESNO);
			switch (msgboxID)
			{
			case IDYES:
				CreateDirectory(lpMyString, NULL);
				Custom_Output = true;
				break;
			case IDNO:
				MessageBox(NULL, "Exiting program", "", MB_ICONWARNING | MB_OK);
				return 0;
				break;
			}
		}
		if (HEADER_GRAPHIC) {
			setCursorPosition(2, 4);
		}
		Custom_Output = true;
		directoryOut = directoryOut + "\\";
		cout << "   Output Directory: " << argv[a + 1] << "\n";
	}
	else if (arg.compare("/dirdebug") == 0) {
		directoryDebug = RemoveChar(argv[a + 1], " \r\n");
		directoryDebug = directoryDebug;
		//cout << "Directory Exits :" << directoryDebug << ": " << dirExists(directoryDebug) << "\n";
		if (!dirExists(directoryDebug)) {
			lpMyString = directoryDebug.c_str();
			message = "Create Directory: " + directoryDebug + "?\n[Yes] Creates Directory\n[No] Debug directory not written to.";
			lpcMessage = message.c_str();

			int msgboxID = MessageBox(NULL, lpcMessage, "", MB_ICONWARNING | MB_YESNO);
			switch (msgboxID)
			{
			case IDYES:
				CreateDirectory(lpMyString, NULL);
				Custom_DebugOutput = true;
				break;
			case IDNO:
				Custom_DebugOutput = false;
				break;
			}
		}
		//Custom_DebugOutput = true;
		directoryDebug = directoryDebug + "\\";
		//cout << "Debug directory: " << argv[a + 1] << "\n";
	}
}
if (HEADER_GRAPHIC) {
	buildHeader("TOP", 1);

	LPCSTR lpcMessage2;
	string tempREVISION = "  APG Tool " + REVISION;
	lpcMessage2 = tempREVISION.c_str();
	SetConsoleTitle(lpcMessage2);
	ShowConsoleCursor(false);
}
if (G_USAGE) {
		if (HEADER_GRAPHIC) {
		setCursorPosition(0, 22);
	}

	cout << "  =============================================================================" << "\n";
	cout << "\t\t\t\tUsage Examples" << "\n";
	cout << "  =============================================================================" << "\n";
	cout << "  Example usage 1: Shows version date only and exits" << "\n";
	cout << "  APG_TOOL_VX.X " << "/version" << "\n\n";
	cout << "  Example usage 2: Pause for error message " << "\n";
	cout << "  APG_TOOL_VX.X " << "/errnohalt" << "\n\n";
	cout << "  Example usage 3: Pause for warning message" << "\n";
	cout << "  APG_TOOL_VX.X " << "/warnnohalt" << "\n\n";
	cout << "  Example usage 4: Don't store original pattern to debug output" << "\n";
	cout << "  APG_TOOL_VX.X " << "/no_orig_code" << "\n\n";
	cout << "  Example usage 5: Turn debug on " << "\n";
	cout << "  APG_TOOL_VX.X " << "/debug_on" << "\n\n";
	cout << "  Example usage 6: Select input files directory " << "\n";
	cout << "  APG_TOOL_VX.X " << "/dirin my_input_files" << "\n\n";
	cout << "  Example usage 7: Select and/or create output files directory " << "\n";
	cout << "  APG_TOOL_VX.X " << "/dirout my_output_files" << "\n\n";
	cout << "  Example usage 8: Select and/or create debug files directory.\n";
	cout << "  Create default dirctory debug_files to collect debug data. " << "\n";
	cout << "  APG_TOOL_VX.X " << "/dirdebug my_debug_files" << "\n\n";
	cout << "  Example usage 9: Create per_pat_inits.txt file " << "\n";
	cout << "  APG_TOOL_VX.X " << "/req" << "\n\n";
	cout << "  Example usage 10: Warning for X/Y ALU unsupported instructions. " << "\n";
	cout << "  APG_TOOL_VX.X " << "/alu_warn" << "\n\n";
	cout << "  Example usage 12: Turn Header Graphics Off. " << "\n";
	cout << "  APG_TOOL_VX.X " << "/grph_off" << "\n\n";
	cout << "  Example usage 13: Show Change Log. " << "\n";
	cout << "  APG_TOOL_VX.X " << "/chg_log" << "\n\n";
	_getch();
	return 0;
}
if (G_CHANGELOG) {
	if (HEADER_GRAPHIC) {
		setCursorPosition(0, 22);
	}
	changeLog();
	_getch();
	return 0;
}
if(!Custom_Input){
	directory = "input_files";
	if (!dirExists(directory)) {
		MessageBox(NULL, "Default Input Directory does not exist.\nPlease create input_files directory.", "", MB_ICONWARNING | MB_OK);
		return 0;
	}
	if (HEADER_GRAPHIC) {
		setCursorPosition(2, 3);
	 setTextColor(26);
	}
	cout << "   Input Directory:  ";

	if (HEADER_GRAPHIC) {
		 setTextColor(31);
	}
	cout << directory;

	directory = directory + "\\";
}

if (!Custom_Output) {
	directoryOut = "output_files";
	if (!dirExists(directoryOut)) {
		lpMyString = directoryOut.c_str();
		CreateDirectory(lpMyString, NULL);
	}
	if (HEADER_GRAPHIC) {
		setCursorPosition(2, 4);
	 setTextColor(26);
	}
	cout << "   Output Directory: ";
	if (HEADER_GRAPHIC) {

		 setTextColor(31);
	}
	cout << directoryOut;
	directoryOut = directoryOut + "\\";
}


if ((Custom_DebugOutput)) {
	//directoryDebug = "debug_files";
	if (!dirExists(directoryDebug)) {
		lpMyString = directoryDebug.c_str();
		CreateDirectory(lpMyString, NULL);
	}
	if (HEADER_GRAPHIC) {
		setCursorPosition(45, 4);
	}
	string tempDir = directoryDebug;
	tempDir.erase(tempDir.end() - 1);
	 setTextColor(26);
	cout << "Debug Directory: ";
	 setTextColor(31);
	cout << tempDir << "\n";
	directoryDebug = directoryDebug + "\\";
}

if(check_version)
{
	if (HEADER_GRAPHIC) {
		setCursorPosition(1, 18);
	}
	cout << buildDate << endl;
	if (!G_GUI) {
		_getch();
	}
	return 0;
}

time_t ref_t =  cvt_time( buildDate );
time_t cur_t;
time(&cur_t);
cur_t = cur_t - ref_t;

time_t num_of_days =  cur_t / ONE_DAY;

/*
if(num_of_days > 5){
	cout << "Evaluation period has expired, please contact the program vendor." << endl;
	_getch();
	return 0 ;
}
*/
if (HEADER_GRAPHIC) {
	setCursorPosition(1, 19);
}
Initialize();

if(G_PINSCRAMBLE_NAME.length() < 1 ){
	 setTextColor(12);
	LogError("Variable ","pin_scramble not specified in config.txt");
	 setTextColor(10);
	if (G_HALT_E)
	_getch();
	return -1;
}

ParsePatternFiles(directory);
if (HEADER_GRAPHIC) {
	setCursorPosition(0, 23);
}
else {
	cout << "\n";
}
buildHeader("BOTTOM", 4);
ConvertPatterns();

if (G_MODE == MODE1) {
	WriteVecPatterns();
}
else{
	WritePatternReqs();
}

//outfile.close();
if (G_DEBUG_ON)
debugfile.close();
errlogfile.close();
warnlogfile.close();
if (directoryDebug.size() == 0) {
	directoryDebug = "debug_files\\";
}
if (Loop_Violation) { // Added for Nested Loop Violation. - TAZ 06/07/2018 
	string temp_str = Loop_Pattern_List;
	temp_str.erase(temp_str.size());
	message = "Encountered a nested loop violation for pattern(s): "+ temp_str +".";
	lpcMessage = message.c_str();
	int msgboxID = MessageBox(NULL, lpcMessage, "", MB_ICONWARNING | MB_OK);
	 setTextColor(12);
	cout << "  Encountered a nested loop violation for pattern(s): " << temp_str << "."<< endl;
	 setTextColor(10);
}

if (G_WARN_CNT > 0) {
	 setTextColor(2);
	cout << "  " << G_WARN_CNT << " warning(s). See /" + directoryDebug + "warning_logs.txt." << endl;
}
 setTextColor(10);
cout << "  Done. Press any key to quit..." <<endl;
ShowConsoleCursor(true);

//diff = (clock() - start) / (double)(CLOCKS_PER_SEC);
//cout << "Program Execution Time:" << diff  << endl;
// current date/time based on current system
time_t result = time(NULL);

// convert now to string form
char str[26];
ctime_s(str, sizeof str, &result);
//cout << "The local date and time is: " << str << endl;
if (!G_GUI) {
	_getch();
}
else {
	Sleep(1500);
}

CleanUp();

	return 0;
}

