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

#define MAXPATTERN 100
#define MAXPATINST 100
#define MAXPATMICRO 100

typedef vector<string> VecOfStr;
typedef vector<VecOfStr> VecOfVecOfStr;

#define VECTOR_WIDTH 128
#define COUNTERS_CNT 61

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

bool G_ADD_KNOWN = true;

bool G_HALT_E = true;

bool G_HALT_W = true;

bool G_ADD_ORIG = true;

bool G_DEBUG_ON = false;

bool G_VERBOSE = false;

bool G_GUI = false;

bool G_USAGE = false;

int Nested_Loop = 0;
bool Loop_Violation = false;
string Loop_Pattern_List;

unsigned long G_JAMREG,G_DATREG = 0xc0;
unsigned long G_UDATA;
unsigned long G_DBM;
unsigned long G_XADDR;
unsigned long G_YADDR;
unsigned long G_YMAIN,G_YBASE,G_YFIELD,G_XMAIN,G_XBASE,G_XFIELD,G_NUMX,G_NUMY;

bool 
G_CS_ACTIVE_H[8] = {false, false, false, false, false, false, false, false};

map<string,string> stdmapDEFINE;

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

ofstream debugfile;
ofstream warnlogfile;
ofstream errlogfile;

//string inputFileName = G_INPUT_FILE;


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
		LogError("GoSub",": subroutine " + called_pat + " not found." );
		_getch();
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
				if( cm.find("//") < cm.find_first_not_of(" /") )
					vvsPATTERNS.back().m_listComments->push_back("// " + comment);
				else
					vvsPATTERNS.back().m_listComments->push_back("// " + comment);
			}
			else
				vvsPATTERNS.back().m_listComments->push_back("// " + comment);
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
			  LogError("ParseConfigs", "Cannot open input file - config.txt.");
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
			LogError("ParseConfigs : ", key + " not set.");
			if(G_HALT_E)
				_getch();
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
						LogError("ParseConfigs", "Invalid t_cs number : " + toString(cs_num));
						if(G_HALT_E)
							_getch();

				}
				else{
					LogError("ParseConfigs", "Invalid tester function : " + t_cs);
					if(G_HALT_E)
						_getch();
				}
			}
		}
		else
		{
		 LogError("ParseConfigs", "Unknown variable : " + key);
		 if(G_HALT_E)
			_getch();;
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
		LogError("ParsePatInits", "Cannot open input file - per_pat_inits.txt.");
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

vector<string> filesPaths;
string optfileName ="";        
//string inputFolderPath =inputFileName + "\\"; 
string inputFolderPath = directory;
string extension = "*.pat";
getFilesList(inputFolderPath,extension,filesPaths);
vector<string>::const_iterator it = filesPaths.begin();
while( it != filesPaths.end())
{
	int ret2 = ParseGosubLabel(*it);
	int ret = ParsePatternFile(directoryDebug,*it);
	if(ret == -1)
		return ret;
//    frame = imread(*it);//read file names
        //doyourwork here ( frame );
  //  sprintf(buf, "%s/Out/%d.jpg", optfileName.c_str(),it->c_str());
    //imwrite(buf,frame);   
    it++;
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
		LogError("UnScrambleResource","pin_scramble " + G_PINSCRAMBLE_NAME + " not found." );
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
	regex keywords_startloop("^[[:s:]]*(STARTLOOP)");
	regex keywords_endloop("^[[:s:]]*(ENDLOOP)");
	smatch match;
	ToUpper(temp);
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
	if (Nested_Loop > 1) {
		Loop_Violation = true;
		Nested_Loop = 0; // reset nested loop counter
		Loop_Pattern_List = Loop_Pattern_List + patName + ", ";
		//cout << "Encountered a nested loop violation " << Nested_Loop << " for pattern: " << pattern.name << "."<< endl;
	}
}


void Initialize(){
	if (directoryDebug.size() == 0) {
		directoryDebug = "debug_files\\";
	}
	if (G_DEBUG_ON)
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

	for(size_t  c = 0; c < vsEXECPATNAMES.size(); c++){
		for (  vector<Pattern>::iterator it = vvsPATTERNS.begin(); it != vvsPATTERNS.end(); ++it,++i){
				//cout << i << ":" << *it << '\n';
			string name =  trim(it->name);

			if( vsEXECPATNAMES.at(c).compare(name) == 0 ){

				WriteVecPattern(directoryOut,*it);
			
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
	
	if(G_MODE ==  MODE1)
	vvsPATTERNS.back().LoadPatInits("_global_");

	for(size_t  c = 0; c < vsEXECPATNAMES.size(); c++){
		for (  vector<Pattern>::iterator it = vvsPATTERNS.begin(); it != vvsPATTERNS.end(); ++it,++i){

			string name =  trim(it->name);

			if( vsEXECPATNAMES.at(c).compare(name) == 0 ){
				
				it->Convert(directoryOut);
				cout <<"Converted " << name << "." << endl;
				//it->printdata(outfile);
				break;
			}
		}                              
	}

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
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    sscanf(time, "%s %d %d", s_month, &day, &year);

    month = (strstr(month_names, s_month)-month_names)/3;

    t.tm_mon = month;
    t.tm_mday = day;
    t.tm_year = year - 1900;
    t.tm_isdst = -1;

    return mktime(&t);
}

int main(int argc, char* argv[])
{
const time_t ONE_DAY = 24 * 60 * 60 ;
const char *buildDate = __DATE__ ;
//############################################################
//Regex Experiment
//############################################################
	//string input;
	//input = "   startLOOP 5";
	//input = "% u_low_4:";
	//input = " mar	        gosub, shift_into_datreg, nolatch";
	//input = "tyarnold1@gmail.com";
	//regex keywords("([tyarnold1]+)@([[:w:]]+)\.com");
	// \w => [[:w:]]  \s => [[:s:]] \d => [[:d:]] 
	//regex keywords("^[[:s:]]*(STARTLOOP)");
	//regex keywords("(mar)+[[:s:]]*(gosub)+[[:s:]]*,+[[:s:]]*([[:w:]]+)[[:s:]]*,*[[:s:]]*([[:w:]]*)");
	//regex keywords("%+[[:s:]]*([[:w:]]+)[[:s:]]*:+");
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
	else if (arg.compare("/usage") == 0)
		G_USAGE = true;
	else if (arg.compare("/gui") == 0)
		G_GUI = true;
	else if (arg.compare("/dirin") == 0) {
		directory = RemoveChar(argv[a + 1], " \r\n");
		if (!dirExists(directory)) {
			MessageBox(NULL, "Input Directory does not exist.\nPlease create input_files directory.", "", MB_ICONWARNING | MB_OK);
			return 0;
		}
		cout << "Input directory: " << argv[a + 1] << "\n";
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
		Custom_Output = true;
		directoryOut = directoryOut + "\\";
		cout << "Output directory: " << argv[a + 1] << "\n";
	}
	else if (arg.compare("/dirdebug") == 0) {
		directoryDebug = RemoveChar(argv[a + 1], " \r\n");
		directoryDebug = directoryDebug;
		//cout << "Directory Exits :" << directoryDebug << ": " << dirExists(directoryDebug) << "\n";
		if (!dirExists(directoryDebug)) {
			Sleep(1000);
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
if (G_USAGE) {
	cout << "=============================================================================" << "\n";
	cout << "\t\t\t\tUsage Examples" << "\n";
	cout << "=============================================================================" << "\n";
	cout << "Example usage 1: Shows version date only and exits" << "\n";
	cout << "APG_TOOL_VX.X " << "/version" << "\n\n";
	cout << "Example usage 2: Pause for error message " << "\n";
	cout << "APG_TOOL_VX.X " << "/errnohalt" << "\n\n";
	cout << "Example usage 3: Pause for warning message" << "\n";
	cout << "APG_TOOL_VX.X " << "/warnnohalt" << "\n\n";
	cout << "Example usage 4: Don't store original pattern to debug output" << "\n";
	cout << "APG_TOOL_VX.X " << "/no_orig_code" << "\n\n";
	cout << "Example usage 5: Turn debug on " << "\n";
	cout << "APG_TOOL_VX.X " << "/debug_on" << "\n\n";
	cout << "Example usage 6: Select input files directory " << "\n";
	cout << "APG_TOOL_VX.X " << "/dirin my_input_files" << "\n\n";
	cout << "Example usage 7: Select and/or create if not exists output files directory " << "\n";
	cout << "APG_TOOL_VX.X " << "/dirout my_output_files" << "\n\n";
	cout << "Example usage 8: Select and/or create if not exists debug files directory " << "\n";
	cout << "APG_TOOL_VX.X " << "/dirdebug my_debug_files" << "\n\n";
	return 0;
}
if(!Custom_Input){
	directory = "input_files";
	if (!dirExists(directory)) {
		MessageBox(NULL, "Default Input Directory does not exist.\nPlease create input_files directory.", "", MB_ICONWARNING | MB_OK);
		return 0;
	}
	cout << "Input directory: " << "input_files" << "\n";
	directory = directory + "\\";
}

if (!Custom_Output) {
	directoryOut = "output_files";
	if (!dirExists(directoryOut)) {
		lpMyString = directoryOut.c_str();
		CreateDirectory(lpMyString, NULL);
	}
	cout << "Output directory: " << "output_files" << "\n";
	directoryOut = directoryOut + "\\";
}

if ((Custom_DebugOutput)) {
	//directoryDebug = "debug_files";
	if (!dirExists(directoryDebug)) {
		lpMyString = directoryDebug.c_str();
		CreateDirectory(lpMyString, NULL);
	}
	cout << "Debug directory: " << directoryDebug << "\n";
	directoryDebug = directoryDebug + "\\";
}

if(check_version)
{
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

Initialize();

if(G_PINSCRAMBLE_NAME.length() < 1 ){
	LogError("Variable ","pin_scramble not specified in config.txt");
	_getch();
	return -1;
}

ParsePatternFiles(directory);

ConvertPatterns();

if(G_MODE ==  MODE1)
	WriteVecPatterns();
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
	temp_str.erase(temp_str.size() - 2);
	message = "Encountered a nested loop violation\nfor pattern(s): "+ temp_str +".";
	lpcMessage = message.c_str();
	int msgboxID = MessageBox(NULL, lpcMessage, "", MB_ICONWARNING | MB_OK);
	cout << "Encountered a nested loop violation " << Nested_Loop << " for pattern(s): " << temp_str << "."<< endl;
}

if(G_WARN_CNT > 0)
cout << G_WARN_CNT << " warning(s). See /"+ directoryDebug+"warning_logs.txt." <<endl;
cout << "Done. Press any key to quit..." <<endl;
if (!G_GUI) {
	_getch();
}
else {
	Sleep(1500);
}

CleanUp();

	return 0;
}

