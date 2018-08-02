#include "stdafx.h"
#include "parsepatternfile.h"
#include <regex>
#include <windows.h>
#include "globals.h"
#include <chrono>

extern ofstream debugfile;

extern vector < Pattern > vvsPATTERNS; // vector of pattern instructions

extern vector < string > vsEXECPATNAMES;

extern map<string, string> labelParseDEF;
extern map<string, string> jump2gosub;
extern map<string, map <string, string>> pat_JumpLabel;
extern map<string, string> jumpPatternMap;

extern void addPattern(string pname);
extern bool checkPatternExist(string pname);
extern void addSubVecDef(string vecdef);
extern void addPatInstVecDef(string vecdef);
extern void addPatInst(string patinst, string vecdef);
void setCursorPosition(unsigned int x, int y);
extern void ParseIncludeFile(string fname);
extern void ProcessInitBlock(string init);
extern void addComment(string comment);
extern int G_MODE;
string lastVecDef;
string initVecDef = "";
vector <string >jumpList;
vector <string >jumpLineList;
extern string directory;

int ParseGosubLabel(string fname_in, vector<string>fileList) {

	string gs;
	LPCSTR lpcMessage;
	string message;

	vector <string >gosubLabelFixed;
	map<string, string> labelLocation;
	vector <string >patList;
	vector <string >gosubLabel;
	vector <string >labelList;

	regex marDoneReturn_keywords("^[[:s:]]*%*[[:s:]]*(mar|var)+[[:s:]]*(done|return)+", std::regex_constants::icase);
	regex pinFunc_keywords("^[[:s:]]*(pinfunc)", std::regex_constants::icase);
	regex label_keywords("^[[:s:]]*%+[[:s:]]*([[:w:]]+)[[:s:]]*:+");
	regex gosub_keywords("^[[:s:]]*%*[[:s:]]*mar[[:s:]]+gosub[[:s:]]*,[[:s:]]*([[:w:]]+)[[:s:]]*", std::regex_constants::icase);
	regex jump_keywords("^[[:s:]]*%*[[:s:]]*mar[[:s:]]+jump[[:s:]]*,[[:s:]]*([[:w:]]+)[[:s:]]*", std::regex_constants::icase);
	regex pattern_keywords("^[[:s:]]*PATTERN.[[:s:]]*([[:w:]]+)", std::regex_constants::icase);
	//regex gosub_keywords("(mar)+[[:s:]]*(gosub)+[[:s:]]*,+[[:s:]]*([[:w:]]+)[[:s:]]*,*[[:s:]]*([[:w:]]*)");

	ifstream in2(fname_in.c_str(), ios::in | ios::binary);
	int i = 0;
	int lineCount = 0;
	int marLineCount = 0;
	bool foundGosub;
	bool marDone;
	bool foundLabel;
	bool foundPinfunc;
	bool foundJump;
	bool foundPattern;
	bool marDoneFound = false;
	int mar_2_pinfunc_Count = 0;
	string lastMarDone = "";
	string stopLine;
	string startLine;
	string str;
	string str2;
	string fileExist;
	int ret;
	do {
		getline(in2, gs);
		smatch match;
		smatch match2;
		smatch match3;
		smatch match4;
		smatch match5;
		smatch match6;
		//cout << "Line : " << gs << endl;

		foundGosub = regex_search(gs, match2, gosub_keywords);
		marDone = regex_search(gs, match3, marDoneReturn_keywords);
		foundLabel = regex_search(gs, match, label_keywords);
		foundJump = regex_search(gs, match5, jump_keywords);
		foundPattern = regex_search(gs, match6, pattern_keywords);
		foundPinfunc = regex_search(gs, match4, pinFunc_keywords);

		bool countEnd = false;

		//if ((foundPattern) && (LabelFirst)) {

		mar_2_pinfunc_Count++;

		if ((foundPinfunc) && (marDoneFound) && (mar_2_pinfunc_Count <= 4)) {
			stopLine = to_string(i);
			mar_2_pinfunc_Count = 0;
			marDoneFound = false;
			for (map<string, string>::iterator iter = labelLocation.begin(); iter != labelLocation.end(); ++iter) {
				str = iter->second;

				if (str.find("." + lastMarDone) != -1) {
					//std::cout << "mar done to change: " << lastMarDone <<   " from " << str << "to: " << stopLine <<" ############'\n";
					size_t index = 0;
					index = str.find(".");
					str.replace(index + 1, stopLine.length(), stopLine);
					labelLocation[iter->first] = str;
				}
			}
		}


		if (marDone) {
			marDoneFound = true;
			mar_2_pinfunc_Count = 0;
			stopLine = to_string(i);

			for (map<string, string>::iterator iter = labelLocation.begin(); iter != labelLocation.end(); ++iter) {
				str = iter->second;
				if (str.find(".") == -1) {
					labelLocation[iter->first] = str + "." + stopLine;
					lastMarDone = stopLine;
				}
			}
		}

		if (foundPattern) {

			patList.push_back(match6[1].str());
			currentPattern = match6[1].str();

			//std::cout << "########### Label found: " << match[1].str() <<   " starting on line : " <<  startLine <<" ############'\n";
		}

		if (foundJump) {
			jumpList.push_back(match5[1].str());
			jumpLineList.push_back(gs);
			jumpPatternMap[match5[1].str()] = currentPattern;
			//std::cout << "########### Label found: " << match[1].str() <<   " starting on line : " <<  startLine <<" ############'\n";
		}

		if (foundLabel) {
			startLine = to_string(i);
			//std::cout << "########### Label found: " << match[1].str() <<   " starting on line : " <<  startLine <<" ############'\n";
			labelLocation[match[1].str()] = startLine;
			//std::cout << "################### Label Found: " << match[1].str() << "#####################'\n";
			labelList.push_back(match[1].str());
			string labelName = match[1].str();
			startLine = lineCount;
		}
		if (foundGosub) {
			//cout << "Found Gosub." << endl;
			string myGroup = match2[1].str();
			//std::cout << "Gosub Found: " << match2[1].str() << " in line " << gs << "'\n";
			gosubLabel.push_back(match2[1].str());
			//std::cout << "at line : " << lineCount << "'\n";
		}
		i++;
		lineCount++;
	} while (!in2.eof());
	in2.close();

	regex Pattern_keywords("^[[:s:]]*input_files\\\\([[:w:]]+).pat", std::regex_constants::icase);
	regex JumpGosub_keywords("^[[:s:]]*%*mar[[:s:]]*jump[[:s:]]*,[[:s:]]*([[:w:]]+)", std::regex_constants::icase);

	smatch match7;
	smatch match8;
	//bool patName;
	//bool jumpGosub;
	string keydata;
	string valuedata;
	map <string, string> tempMap;

	for (vector<string>::iterator jp = jumpList.begin(); jp != jumpList.end(); ++jp) {
		for (vector<string>::iterator ptl = patList.begin(); ptl != patList.end(); ++ptl) {
			str = *ptl;

			if (*ptl == *jp) {
				// std::cout << "Found a jump to a gosub:" << str << " in file " << fname_in << endl; // Debug
				message = "A Jump to a Subroutine was found!!\n\n[Yes] Change the ""jump"" micro instruction to ""gosub"" for " + str + " in pattern: " + fname_in + "?\n\n[No] Continue on with error and manually modify the pattern.";
				lpcMessage = message.c_str();
				int msgboxID = MessageBox(NULL, lpcMessage, "Error Encountered", MB_ICONERROR | MB_YESNO);
				switch (msgboxID)
				{
				case IDYES:
					goto jumpProcess;
					break;
				case IDNO:
					MessageBox(NULL, "Continuing with Jump to a Subroutine Error", "Error Encountered", MB_ICONWARNING | MB_OK);
					continue;
					break;
				}
			jumpProcess:
				jump2gosub[str] = jumpPatternMap[str];
				for (map<string, string>::iterator lb1 = jump2gosub.begin(); lb1 != jump2gosub.end(); lb1++) {
					keydata = lb1->first;
					valuedata = lb1->second;
					if (str.find(keydata) != -1) {
						tempMap[keydata] = valuedata;
						pat_JumpLabel[lb1->second] = tempMap;
					}
				}
			}
		}
	}

	for (vector<string>::iterator lb = labelList.begin(); lb != labelList.end(); ++lb) {
		for (vector<string>::iterator gsl = gosubLabel.begin(); gsl != gosubLabel.end(); ++gsl) {
			str = *gsl;
			str2 = "input_files\\" + str + ".pat";
			if (*gsl == *lb) {
				// Check if new subroutine .pat file already exits.
				if ((std::find(fileList.begin(), fileList.end(), str2) != fileList.end()) ||
					(std::find(gosubLabelFixed.begin(), gosubLabelFixed.end(), str) != gosubLabelFixed.end()))
				{
					break;
				}
				string newGosubPat = str;
				vector<string> strSE = Tokenize(labelLocation[str], '.');
				int tts = stoi(strSE[0]);

				int tte = stoi(strSE[1]);

				message = "A Gosub to a Label was found!!\n\n[Yes] Auto Create Subrouine Pattern " + newGosubPat + ".pat.\n\n[No] Continue on with error and manually create pattern.";
				lpcMessage = message.c_str();
				int msgboxID = MessageBox(NULL, lpcMessage, "Error Encountered", MB_ICONERROR | MB_YESNO);
				switch (msgboxID)
				{
				case IDYES:
					ret = creatSubPattern(fname_in, newGosubPat, tts, tte);
					gosubLabelFixed.push_back(newGosubPat);

					break;
				case IDNO:
					MessageBox(NULL, "Continuing with Gosub to Label Error", "Error Encountered", MB_ICONWARNING | MB_OK);
					break;
				}
				//std::cout << "Found a Gosub to a Label:" << str << " at Start.Stop location: " << labelLocation[str] << endl;
				//std::cout << "Current pattern file:" << fname_in << " at Start.Stop location: " << labelLocation[str] << " New pattern:" << newGosubPat << endl;
				//std::cout << "Current pattern file:" << fname_in << " at Start location: " << tts << " at Stop location: " << tte << " New pattern:" << newGosubPat << endl;
			}
		}
	}
	return 0;
}

int creatSubPattern(string patName, string newPatName, int startLine, int stopLine) {
	string dirout = directory;
	string dirin = directory;
	ofstream outfile;
	ofstream infile;
	string fname = dirout;
	string fnameNew = dirin;
	fname += patName + ".pat";
	fnameNew += newPatName;
	vector<string> inputFile;
	string gs;
	int lineNum = 0;

	ifstream in3(patName.c_str(), ios::in | ios::binary);

	if (!in3) {
		setCursorPosition(5, 22);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
		LogError("ParsePatternFile", "Cannot open input file - patterns.pat.");
		return -1;
	}
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time(&rawtime);
#pragma warning (disable : 4996)
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, " %A %B %d at %r.", timeinfo);
	//strftime(buffer, 80, " %c.", timeinfo);

	inputFile.push_back("// *****************************************************************************");
	inputFile.push_back("// Filename:	" + newPatName + ".pat");
	inputFile.push_back("// Pattern Auto Generated by APG Tool on" + string(buffer));
	inputFile.push_back("// *****************************************************************************");
	inputFile.push_back("");
	inputFile.push_back("#include \"defines_part.h\"");
	inputFile.push_back("#include \"defines_family.h\"");
	inputFile.push_back("#include \"defines_common.h\"");
	inputFile.push_back("");
	inputFile.push_back("PATTERN( " + newPatName + " )");
	inputFile.push_back("// *****************************************************************************");
	inputFile.push_back("// *****************************************************************************");

	do {
		getline(in3, gs);
		if ((lineNum >= startLine) && (lineNum <= stopLine)) {
			gs.erase(std::remove(gs.begin(), gs.end(), '\r'), gs.end());
			inputFile.push_back(gs);
		}
		lineNum++;
	} while (!in3.eof());

	outfile.open(dirin + newPatName + ".pat");
	for (vector<string>::iterator iter = inputFile.begin(); iter != inputFile.end(); ++iter) {
		outfile << *iter << endl;
	}
	outfile.close();


	//infile.open(fname.c_str());

	//infile.close();

	//outfile.open(fname.c_str());

	return 0;
}

int ParsePatternFile(string dirdebug, string fname_in) {
	if (dirdebug.size() == 0) {
		dirdebug = "debug_files\\";
	}
	enum Mode {
		none,
		keyword,
		incl,
		pat_name,
		init,
		vecdef,
		instr,
		micro,
		param,
		comment,
		ln_comment,
		cl_comm1
	};

	ofstream outfile;
	int start = fname_in.find_first_of("\\") + 1;
	string fname_out = "parse_" + fname_in.substr(start);
	fname_out = fname_out.substr(0, fname_out.find_first_of("."));
	fname_out += ".txt";
	fname_out = dirdebug + fname_out;
	Log("ParsePatternFile1", "outfile = " + fname_out);
	outfile.open(fname_out.c_str());

	int mode = none;
	int prev_mode = none;

	string buf, buf2, currentLine;
	char pk, ch;

	ifstream in(fname_in.c_str(), ios::in | ios::binary);

	if (!in) {
		setCursorPosition(5, 22);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
		LogError("ParsePatternFile", "Cannot open input file - patterns.pat.");
		return -1;
	}

#define _debug_parsing_

	Log("ParsePatternFile2", "ParsePatFile() : before do...");
	do {

		pk = in.peek();
		//cout << pk << endl;
		if ('\n' == pk) {
			currentLine = "";
		}
		currentLine = currentLine + pk;
#ifdef _debug_parsing_
		Log("ParsePatternFile3", "pos: " + toString(in.tellg()) + " mode : " + toString(mode) + " peek : " + toString(pk)); //

#endif

		if (mode == none) {

			if (isspace(pk)) {
				in.get(ch);
			}
			else if (pk == '#') {
				in.get(); //discard #
				if (in.peek() == 'i')
					in.get(ch);
				buf += ch;
				if (in.peek() == 'n')
					in.get(ch);
				buf += ch;
				if (in.peek() == 'c')
					in.get(ch);
				buf += ch;
				if (in.peek() == 'l')
					in.get(ch);
				buf += ch;
				if (in.peek() == 'u')
					in.get(ch);
				buf += ch;
				if (in.peek() == 'd')
					in.get(ch);
				buf += ch;
				if (in.peek() == 'e')
					in.get(ch);
				buf += ch;
				if (buf.compare("include") == 0) {
					buf = "";
					mode = incl;
					Log("ParsePatternFile4", "incl");
				}
				else {
					setCursorPosition(5, 22);
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
					LogError("ParsePatternFile :" + fname_in, pk + " error in parsing: expecting an \"include\" keyword");
					_getch();
					return -1;
				}
			}
			else if (pk == 'P') {
				mode = keyword;
				Log("ParsePatternFile5", "keyword");
			}
			else if (pk == '(') {
				mode = pat_name;
				Log("ParsePatternFile6", "pat_name");
				in.get(); //discard '(';

			}
			else if (pk == '@') {
				mode = init;
				in.get(); //discard '@';
				pk = in.peek();
				if (pk == '{') {
					in.get(); //discard '{'
				}
				else {
					setCursorPosition(5, 22);
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
					LogError("ParsePatternFile", "error in parsing: expecting an '{'");
					return -1;
				}
				Log("ParsePatternFile7", "init");
			}
			else if (pk == '/') {
				in.get(); //discard '';
				pk = in.peek();
				if (pk == '/') {

					in.get(); //discard '/'
					buf2 = " // ";
					mode = comment;
				}
				else if (pk == '*') {
					in.get(); //discard '/'
					buf2 = " /* ";
					mode = cl_comm1;

				}

				else {



					LogError("ParsePatternFile", "error in parsing: expecting a '/' - if this line is a comment");
					return -1;
				}



				/*		LogError("ParsePatternFile", "error in parsing: expecting a '/' - if this line is a comment for ");
				cout << "Error at current line       :" << currentLine << endl;
				if (pk != '*'){ // Added for one line block comments errors.  Example -> " One Line with Block Comment line " TAZ - 07142018
				// setCursorPosition(5, 22);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
				LogError("ParsePatternFile", "error in parsing: expecting a '/' - if this line is a comment");
				return -1;
				}
				in.get(); //discard '*'
				mode = cl_comm1;
				buf2 = "";
				}*/



				Log("ParsePatternFile", "comment");
			}
			else if (pk == '%') {
				in.get(); // discard '%'
				pk = in.peek();
				if (pk == '%') {
					in.get(); // discard '%'
					pk = in.peek();
					mode = vecdef;
					buf = "";
					Log("ParsePatternFile", "vecdef");
				}
				else {
					mode = instr;
					Log("ParsePatternFile", "instr");
				}

			}
			else {
				Log("ParsePatternFile", "Unknown char ->");
				_getch();
			}

		}
		else // if mode != none
		{
			if (mode != comment && mode != ln_comment && mode != cl_comm1 && mode != init) // watching for comment if inside a comment

				if (pk == '/') {

					in.get(ch); // discard '/';
					buf2 += ch;
					pk = in.peek();

					if (pk == '*')
					{
						in.get(); // discard '*'
						prev_mode = mode;
						mode = cl_comm1;
						buf2 = "";
						Log("ParsePatternFile", "enclosed comment");

					}
					else if (pk == '/') {
						in.get(); // discard '/'
						buf2 += ch;
						//addPatInst(buf);
						//buf = "";
						prev_mode = mode;
						mode = ln_comment;
						//buf2="";
						Log("ParseDefineFile", "line comment");

					}
					else {
						Log("ParseDefineFile", "malformed file: expecting an '*'  - if this is an inside comment or a '/' if line comment.");
						_getch();
						return -1;
					}

				}
		}

		switch (mode) {
		case incl:
		{
			if (pk == '\n') {
				in.get(); //discard '\n
				Log("ParsePatternFile8", buf);
				ParseIncludeFile(buf);
				buf = "";
				mode = none;
			}
			else {

				in.get(ch); //discard comment
				buf += ch;
			}
		}
		break;
		case keyword:
		{ in.get(ch); // get 'P'
		buf += ch;
		if (in.peek() == 'A') {
			in.get(ch);
			buf += ch;
		}
		else {
			in.get(ch);
			buf += ch;
			mode = prev_mode;
			break;
		}
		if (in.peek() == 'T')
			in.get(ch);
		buf += ch;
		if (in.peek() == 'T')
			in.get(ch);
		buf += ch;
		if (in.peek() == 'E')
			in.get(ch);
		buf += ch;
		if (in.peek() == 'R')
			in.get(ch);
		buf += ch;
		if (in.peek() == 'N')
			in.get(ch);
		buf += ch;

		Log("ParsePatternFile9", buf);
		buf = "";
		mode = none;

		}
		break;

		case pat_name:
		{

			if (isspace(pk)) {
				in.get(); //discard whitespace
			}
			else if (pk == ')') {
				in.get(); //discard ')'
				Log("ParsePatternFile10", buf);
				if (checkPatternExist(trim(buf))) {
					setCursorPosition(5, 22);
					LogError("ParsePatternFile", "Add new pattern, " + buf + " already exist.");
					//_getch();
					// LPCSTR msg = "Duplicate Pattern Encountered! " + buf.c_str +"\n\nRemove duplicate Pattern Name(s) and Rerun program.\n\n Exiting Program......";
					MessageBox(NULL, TEXT("\tDuplicate Pattern Encountered! \n\nRemove Duplicate Pattern Name(s) and Rerun Program.\n\n\tExiting Program......"), TEXT("APG Tool Error!"), MB_OK | MB_ICONEXCLAMATION);
					exit(EXIT_FAILURE);
					return -1;
				}
				//  int i = 0;

				//  for (vector < Pattern > ::iterator it = vvsPATTERNS.begin(); it != vvsPATTERNS.end(); ++it, ++i) {
				//		  cout << "################ m_SubVecDef ############# ->" << it->getSubVecDef() << "\n";
				//	  }

				addPattern(trim(buf));
				lastVecDef = "";

				buf = "";

				mode = none;
			}
			else {
				in.get(ch);
				buf += ch;
			}

		}
		break;
		case init:
		{
			if (pk == '@') {
				in.get(); //discard '@'
				pk = in.peek();
				if (pk == '}') {
					in.get(); //discard '}'
					Log("ParsePatternFile11", buf);
					if (G_MODE == MODE2) {
						//ProcessInitBlock(buf);
					}
					//stdvecPATINST.push_back(buf);
					buf = "";
					mode = none;
				}
				else {
					setCursorPosition(5, 22);
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
					LogError("ParsePatternFile", "error in parsing: expecting an '}'. at context: init");
					return -1;
				}
			}
			else {
				in.get(ch);
				buf += ch;

			}
		}
		break;
		case comment:
		{

			//cout <<"mode : "<< mode <<" peek : " << pk << " = "<<int(pk)<<'\n';
			//_getch();
			if (pk == '\n') {
				in.get(); //discard '\n
				addComment(buf2);
				buf2 = "";
				mode = none;
				pk = in.peek();

			}
			else {
				in.get(ch);
				buf2 += ch;
			}
		}
		break;

		case ln_comment:
		{
			if (pk == '\n') {

				//in.get(); //discard '\n
				addComment(buf2);
				buf2 = "";
				mode = prev_mode;
				//prev_mode = ln_comment;
				//pk = in .peek();
			}
			else {
				in.get(ch);
				buf2 += ch;
				//cout <<"buf : "<< buf << endl;
			}
		}
		break;

		case cl_comm1: {
			if (pk == '*')
			{
				in.get(); //discard '*'
				pk = in.peek();
				if (pk == '/') {
					in.get(); //discard '/'
					addComment(buf2);
					buf2 = "";
					mode = prev_mode;
				}
			}
			else {
				in.get(ch); //discard comment
				buf2 += ch;
			}

		} break;

		case vecdef:
		{
			//if(pk == '\n'){
			if (pk == '%') {
				lastVecDef = "";
				Log("ParsePatternFile->VECDEF", buf);
				if (lastVecDef.length() < 1) {
					lastVecDef = buf;
				}
				else {
					lastVecDef = lastVecDef;
				}
				addSubVecDef(buf);

				buf = "";
				in.get(); // discard %
				mode = instr;

			}
			else {
				in.get(ch);
				buf += ch;
			}
		}
		break;
		case instr:
		{
			if (pk == '\n') {
				in.get(ch);
				buf += ch;
				pk = in.peek();
				if (pk == 'P') {

					Log("ParsePatternFile12", buf);
					//cout  << "->lastVecDef 1 : " << lastVecDef << "  " <<  buf << "\n";

					addPatInst(buf, lastVecDef);
					//addPatInstVecDef(lastVecDef);
					//stdvecPATINST.push_back(buf);
					buf = "";
					prev_mode = mode;
					mode = keyword;
					Log("ParsePatternFile", "keyword  after \\n");
				}

			}
			else if (pk == '%') { // new instruction

				Log("ParsePatternFile13", buf);
				if (prev_mode != ln_comment)
					addPatInst(buf, lastVecDef);
				//  addPatInstVecDef(lastVecDef);

				//lastVecDef.erase(remove(lastVecDef.begin(), lastVecDef.end(), "\t"), lastVecDef.end());
				// lastVecDef.erase(remove(lastVecDef.begin(), lastVecDef.end(), "\n"), lastVecDef.end());
				// lastVecDef.erase(remove(lastVecDef.begin(), lastVecDef.end(), " "), lastVecDef.end());
				// cout << "->lastVecDef 2: " << lastVecDef << "  " << buf << "\n";

				//stdvecPATINST.push_back(buf);
				buf = "";
				mode = none;
			}
			else {
				in.get(ch);
				buf += ch;
			}

		}
		break;

		case micro:
		{}
		break;
		//default:{} break;
		}

	} while (!in.eof());

	in.close();
	if (mode == instr) { //output last pattern instruction
		Log("ParsePatternFile - last ", buf);
		addPatInst(buf, lastVecDef);
		//	addPatInstVecDef(lastVecDef);

		//cout << "->lastVecDef 3: " << lastVecDef << "  " << buf << "'\n";

	}
	Log("ParsePatternFile", "End of ParsePatternFile.");
	outfile.close();

	int i = 0;
	for (vector < Pattern > ::iterator it = vvsPATTERNS.begin(); it != vvsPATTERNS.end(); ++it, ++i) {

		it->UpdateLstIndexes();
		it->printdata(debugfile);
	}

	return 0;
}
