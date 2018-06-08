#include "stdafx.h"
#include "parsepatternfile.h"
#include <regex>
#include <windows.h>

extern ofstream debugfile;

extern vector < Pattern > vvsPATTERNS; // vector of pattern instructions

extern vector < string > vsEXECPATNAMES;

extern void addPattern(string pname);
extern bool checkPatternExist(string pname);
extern void addSubVecDef(string vecdef);
extern void addPatInstVecDef(string vecdef);
extern void addPatInst(string patinst,string vecdef);
extern void ParseIncludeFile(string fname);
extern void ProcessInitBlock(string init);
extern void addComment(string comment);
extern int G_MODE;
string lastVecDef;
string initVecDef = "";

int ParseGosubLabel(string fname_in) {

	string gs;
	vector <string >labelList;
	vector <string >gosubLabel;

	regex label_keywords("%+[[:s:]]*([[:w:]]+)[[:s:]]*:+");
	regex gosub_keywords("(mar)+[[:s:]]*(gosub)+[[:s:]]*,+[[:s:]]*([[:w:]]+)[[:s:]]*,*[[:s:]]*([[:w:]]*)");

	ifstream in2(fname_in.c_str(), ios::in | ios::binary);
	int i = 0;
	do {
		getline(in2, gs);
		smatch match;
		//cout << "Line : " << gs << endl;

		bool found = regex_search(gs, match, label_keywords);
		if (found) {
			//std::cout << "Label Found: " << match[1].str() << "'\n";
			labelList.push_back(match[1].str());
		}
		//cout << "Line : " << gs << endl;
		i++;
	} while (!in2.eof());
	in2.close();
	
	ifstream in3(fname_in.c_str(), ios::in | ios::binary);
	do {
		getline(in3, gs);
		smatch match;
		//cout << "Line : " << gs << endl;

		bool found = regex_search(gs, match, gosub_keywords);
		if (found) {
			//cout << "Found Gosub." << endl;

			//std::cout << "Gosub Found: " << match[3].str() << "'\n";
			gosubLabel.push_back(match[3].str());

			/*for (size_t i = 0; i < match.size(); ++i) {
			//std::cout << "Gosub Found: " << match[i].str() << "'\n";
			std::cout << "Label Found: " << match[i].str() << "'\n";
			}*/
		}
		//cout << "Line : " << gs << endl;
	} while (!in3.eof());
	in3.close();

	string str;
	for (vector<string>::iterator lb = labelList.begin(); lb != labelList.end(); ++lb) {
		for (vector<string>::iterator gs = gosubLabel.begin(); gs != gosubLabel.end(); ++gs) {
			str = *gs;
			//cout << "For loop Gosub." << str << endl;
			if (*gs == *lb) {
			cout << "Found a Gosub to a Label:" << str << endl;
			}
		}
	}


	return 0;
}


int ParsePatternFile(string dirdebug,string fname_in) {
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
  fname_out = dirdebug+fname_out;
  Log("ParsePatternFile1", "outfile = " + fname_out);
  outfile.open(fname_out.c_str());

  int mode = none;
  int prev_mode = none;

  string buf,buf2;
  char pk, ch;

  ifstream in (fname_in.c_str(), ios:: in | ios::binary);

  if (! in ) {
    LogError("ParsePatternFile", "Cannot open input file - patterns.pat.");
    return -1;
  }

  #define _debug_parsing_

  Log("ParsePatternFile2", "ParsePatFile() : before do...");
  do {

    pk = in .peek();
	#ifdef _debug_parsing_
    Log("ParsePatternFile3","pos: " + toString(in.tellg()) +" mode : " + toString(mode) + " peek : " + toString(pk)  ); //

    #endif

    if (mode == none) {

      if (isspace(pk)) { 
	  in.get(ch);
      } else if (pk == '#') {
	    in.get(); //discard #
        if ( in .peek() == 'i')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'n')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'c')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'l')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'u')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'd')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'e')
          in.get(ch);
        buf += ch;
        if (buf.compare("include") == 0) {
          buf = "";
          mode = incl;
          Log("ParsePatternFile4", "incl");
        } else {
          LogError("ParsePatternFile :" + fname_in, pk + " error in parsing: expecting an \"include\" keyword");
          _getch();
          return -1;
        }
      } else if (pk == 'P') {
        mode = keyword;
        Log("ParsePatternFile5", "keyword");
      } else if (pk == '(') {
        mode = pat_name;
        Log("ParsePatternFile6", "pat_name"); 
		in.get(); //discard '(';

      } else if (pk == '@') {
        mode = init; 
		in.get(); //discard '@';
        pk = in .peek();
        if (pk == '{') { 
		in.get(); //discard '{'
        } else {
          LogError("ParsePatternFile", "error in parsing: expecting an '{'");
          return -1;
        }
        Log("ParsePatternFile7", "init");
      } else if (pk == '/') { 
	  in.get(); //discard '';
        pk = in .peek();
        if (pk == '/') {

          in.get(); //discard '/'
		  buf2=" // ";
          mode = comment;
        } else {
          LogError("ParsePatternFile", "error in parsing: expecting a '/' - if this line is a comment");
          return -1;
        }
        Log("ParsePatternFile", "comment");
      } else if (pk == '%') { 
	  in.get(); // discard '%'
        pk = in .peek();
        if (pk == '%') { in.get(); // discard '%'
          pk = in .peek();
          mode = vecdef;
          buf = "";
          Log("ParsePatternFile", "vecdef");
        } else {
          mode = instr;
          Log("ParsePatternFile", "instr");
        }

      } else {
        Log("ParsePatternFile", "Unknown char ->");
        _getch();
      }

    } else // if mode != none
    {
      if (mode != comment && mode != ln_comment && mode != cl_comm1 && mode != init) // watching for comment if inside a comment

        if (pk == '/') {

        in.get(ch); // discard '/';
		buf2 += ch;
        pk = in .peek();

        if( pk == '*')
			{
				in.get(); // discard '*'
				prev_mode =  mode;
				mode = cl_comm1;
				buf2="";
				Log("ParsePatternFile","enclosed comment");

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

        } else {
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
        } else {

          in.get(ch); //discard comment
          buf += ch;
        }
      }
      break;
    case keyword:
      { in.get(ch); // get 'P'
        buf += ch;
        if ( in .peek() == 'A'){
          in.get(ch);
		  buf += ch;
		}
		else{
			in.get(ch);
		  buf += ch;
		  mode = prev_mode;
		  break;
		}
        if ( in .peek() == 'T')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'T')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'E')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'R')
          in.get(ch);
        buf += ch;
        if ( in .peek() == 'N')
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
        } else if (pk == ')') { 
			in.get(); //discard ')'
          Log("ParsePatternFile10", buf);
          if (checkPatternExist(trim(buf))) {
            LogError("ParsePatternFile", "Add new pattern, " + buf + " already exist.");
            //_getch();
			// LPCSTR msg = "Duplicate Pattern Encountered! " + buf.c_str +"\n\nRemove duplicate Pattern Name(s) and Rerun program.\n\n Exiting Program......";
			MessageBox(NULL, TEXT("\tDuplicate Pattern Encountered! \n\nRemove Duplicate Pattern Name(s) and Rerun Program.\n\n\tExiting Program......"), TEXT("APG Tool Error!"),  MB_OK | MB_ICONEXCLAMATION);
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
        } else { in.get(ch);
          buf += ch;
        }

      }
      break;
    case init:
      {
        if (pk == '@') { 
			in.get(); //discard '@'
          pk = in .peek();
          if (pk == '}') { 
		    in.get(); //discard '}'
            Log("ParsePatternFile11", buf);
            if (G_MODE == MODE2) {
              //ProcessInitBlock(buf);
            }
            //stdvecPATINST.push_back(buf);
            buf = "";
            mode = none;
          } else {
            LogError("ParsePatternFile", "error in parsing: expecting an '}'. at context: init");
            return -1;
          }
        } else { in.get(ch);
          buf += ch;

        }
      }
      break;
    case comment:
      {

        //cout <<"mode : "<< mode <<" peek : " << pk << " = "<<int(pk)<<'\n';
        //_getch();
        if (pk == '\n') { in.get(); //discard '\n
          addComment(buf2);
          buf2 = "";
          mode = none;
          pk = in .peek();

        } else { 
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
        } else { 
		 in.get(ch);
          buf2 += ch;
		 //cout <<"buf : "<< buf << endl;
        }
      }
      break;

	  case cl_comm1: {
			if(pk == '*')
			{
				in.get(); //discard '*'
				pk = in.peek();
				if(pk == '/'){
					in.get(); //discard '/'
					addComment(buf2);
					buf2="";
					mode = prev_mode;
				}
			}
			else{
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

        } else { in.get(ch);
          buf += ch;
        }
      }
      break;
    case instr:
      {
        if (pk == '\n') { 
		in.get(ch);
          buf += ch;
          pk = in .peek();
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

		} else if (pk == '%') { // new instruction

		  Log("ParsePatternFile13", buf);
		  if(prev_mode != ln_comment)
		  addPatInst(buf, lastVecDef);
		//  addPatInstVecDef(lastVecDef);

		  //lastVecDef.erase(remove(lastVecDef.begin(), lastVecDef.end(), "\t"), lastVecDef.end());
		 // lastVecDef.erase(remove(lastVecDef.begin(), lastVecDef.end(), "\n"), lastVecDef.end());
		 // lastVecDef.erase(remove(lastVecDef.begin(), lastVecDef.end(), " "), lastVecDef.end());
		 // cout << "->lastVecDef 2: " << lastVecDef << "  " << buf << "\n";

		  //stdvecPATINST.push_back(buf);
		  buf = "";
		  mode = none;
		} else { in.get(ch);
		  buf += ch;
		}

      }
      break;

    case micro:
      {}
      break;
      //default:{} break;
    }

  } while (! in .eof());

  in .close();
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

    it-> UpdateLstIndexes();
    it-> printdata(debugfile);
  }

  return 0;
}
