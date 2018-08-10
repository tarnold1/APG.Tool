#include "stdafx.h"
#include "parsedefinefile.h" 
#include <windows.h>
#include "common.h"

extern void addDefinition(string def_str,string fname);
extern bool FindDef(string &str); 
bool check_ifdef(string str){
	str = trim(str);
	if(FindDef(str))
		return true;
	else
		return false;
} 
extern int XPosLast;
extern int YPosLast;

int ParseDefineFile(string dirout,string filename){
	if (dirout.size() == 0) {
		dirout = "output_files\\";
	}
	ofstream outfile;
	string fname_out = dirout+"ParseDefine_";
	fname_out += filename;
	outfile.open (fname_out.c_str());

	enum Mode{none,def,ifdef,endif, incl, ln_comm, ln_comm1, cl_comm, cl_comm1 };

	int mode = none;
	int prev_mode = none;
	int sub_mode = none;
	bool allow = true;
	bool continuation = true; // Added for '\' line continuation. TAZ 10/27/2017

	string buf;
	char pk,ch;
	
	ifstream in(filename.c_str(), ios::in | ios::binary);

	  if(!in) {
		  XPosLast, YPosLast = currentCurPos();
		  //cout << XPosLast << " : " << YPosLast << endl;
		  setCursorPosition(XPosLast, YPosLast);

		setCursorPosition(5, 22);
		 setTextColor(12);
		LogError("ParseDefineFile", "Cannot open input file - " + filename);
		 setTextColor(10);
		_getch();
		return 0;
	  }

	  Log("ParseDefineFile","before do...");
	  do {
		
		pk = in.peek();
		#ifdef _debug_parsing_
		cout <<"mode : "<< mode <<" peek : " << pk << '\n'; //
		#endif

		if(mode == none){
		
			if(isspace(pk))
				  in.get(ch);
			else if(pk == '#'){
				buf="";
				in.get(); // discard #
				if(in.peek() == 'd'){
					in.get(ch);
					buf+= ch;
					if(in.peek() == 'e')
					in.get(ch);
					buf+= ch;
					if(in.peek() == 'f')
					in.get(ch);
					buf+= ch;
					if(in.peek() == 'i')
					in.get(ch);
					buf+= ch;
					if(in.peek() == 'n')
					in.get(ch);
					buf+= ch;
					if(in.peek() == 'e')
					in.get(ch);
					buf+= ch;
					if(buf.compare("define")==0){
						buf="";
						mode = def;
						Log("ParseDefineFile","define" );
					}
					else
					{
						LogError("ParseDefineFile", pk + " : malformed file: expecting a \"define\" keyword");
						_getch();
						return -1;
					}
				}
				else if(in.peek() == 'e'){
					in.get(ch);
					buf+= ch;
					if(in.peek() == 'n')
					in.get(ch);
					buf+= ch;
					if(in.peek() == 'd')
					in.get(ch);
					buf+= ch;
					if(in.peek() == 'i')
					in.get(ch);
					buf+= ch;
					if(in.peek() == 'f')
					in.get(ch);
					buf+= ch;
					if(buf.compare("endif")==0){
						buf="";
						allow = true;
						mode = none;
						Log("ParseDefineFile","endif");
					}
					else
					{
						LogError("ParseDefineFile", pk + " : malformed file: expecting an \"endif\" keyword");
						_getch();
						return -1;
					}
				}
				else if(in.peek() == 'i'){
					in.get(ch); //consume i
					buf+= ch;
					if(in.peek() == 'f'){
						in.get(ch);
						buf+= ch;
						if(in.peek() == 'd')
						in.get(ch);
						buf+= ch;
						if(in.peek() == 'e')
						in.get(ch);
						buf+= ch;
						if(in.peek() == 'f')
						in.get(ch);
						buf+= ch;
						if(buf.compare("ifdef")==0){
							buf="";
							mode = ifdef;
							Log("ParseDefineFile", "ifdef");
						}
						else
						{
							Log("ParseDefineFile", pk + " : malformed file: expecting an \"ifdef\" keyword");
							_getch();
							return -1;
						}
					}
					else if(in.peek() == 'n'){
						in.get(ch);
						buf+= ch;
						if(in.peek() == 'c')
						in.get(ch);
						buf+= ch;
						if(in.peek() == 'l')
						in.get(ch);
						buf+= ch;
						if(in.peek() == 'u')
						in.get(ch);
						buf+= ch;
						if(in.peek() == 'd')
						in.get(ch);
						buf+= ch;
						if(in.peek() == 'e')
						in.get(ch);
						buf+= ch;     
						if(buf.compare("include")==0){
							buf="";
							mode = incl;
							Log("ParseDefineFile","incl" );
						}
						else
						{
							LogError("ParseDefineFile", pk + " : malformed file: expecting an \"include\" keyword");
							_getch();
							return -1;
						}
				
					}//else if
				}

			}
			else if(pk == '/'){
				Log("ParseDefineFile","comment");
				in.get(); //discard '/';
				
				pk = in.peek();

				if( pk == '/' ){
					in.get(); //discard '{'
					mode = ln_comm;
				}
				else if( pk == '*')
				{
					in.get();
					mode = cl_comm;
				}
				else
				{
					Log("ParseDefineFile", pk + " : malformed file: expecting a '/' or '*' - if this is a comment.");
					_getch();
					return -1;
				}
			}
				
		}
		else // mode != none
		{
			if( mode!= ln_comm && mode!= ln_comm1 && mode != cl_comm && mode != cl_comm1 ) // watching for comment if inside a comment

				if(pk == '/'){

					in.get(); // discard '/';
				
					pk = in.peek();

					if( pk == '*')
					{
						in.get(); // discard '*'
						prev_mode =  mode;
						mode = cl_comm1;
						Log("ParseDefineFile","enclosed comment");

					}
					else if( pk == '/')
					{
						in.get(); // discard '/'
						prev_mode = mode;
						mode = ln_comm1;
					}
					else
					{
						Log("ParseDefineFile","malformed file: expecting an '*'  - if this is an inside comment or a '/' if line comment.");
						_getch();
						return -1;
					}
					
				}
			
		}
	

		switch(mode){
		case def:{
			int checkEntry = 0;
			checkEntry = buf.find("TIF_GUARD_RLD");
			if (checkEntry != -1) {
				//cout << checkEntry << endl;
			}

			if((pk == '\n') && (continuation != true))  // Modified for '\' line continuation. TAZ 10/27/2017
			{
				in.get(); //discard '\n
				Log("ParseDefineFile", buf);
				if(allow)
					addDefinition(buf,filename);

				buf="";
				mode = none;
			}
			else if ((pk == '\r') && (continuation == true))  // Added for '\' line continuation. TAZ 10/27/2017
			{
				in.get(); //discard '\n
				Log("ParseDefineFile", buf);
			}
			else if (pk == '\\')  // Added for '\' line continuation. TAZ 10/27/2017
			{
				//in.get(ch); //accumulate chars
				//buf += ch;
				in.get(); //discard '\n
				Log("ParseDefineFile", buf);
				continuation = true;  // Added for '\' line continuation. TAZ 10/27/2017
			}

			else if((pk == '\n') && (continuation == true))  // Added for '\' line continuation. TAZ 10/27/2017
			{
				in.get(); //discard '\n
				Log("ParseDefineFile", buf);
				continuation = true;  // Added for '\' line continuation. TAZ 10/27/2017

			}
			else{

				in.get(ch); //accumulate chars
				buf+=ch;
				continuation = false;  // Added for '\' line continuation. TAZ 10/27/2017
			}
		} break;
		case ifdef:{
			if(pk == '\n')
			{
				in.get(); //discard '\n
				Log("ParseDefineFile",buf);
				allow = check_ifdef(buf);
				buf="";
				mode = none;
			}
			else{
				
				in.get(ch); //accumulate chars
				buf+=ch;
			}
		} break;

		case incl:{
			if(pk == '\n')
			{
				in.get(); //discard '\n
				Log("ParseDefineFile",buf);
				//_getch();
				buf="";
				mode = none;
			}
			else{

				in.get(ch); //discard comment
				buf+=ch;
			}
		} break;

		case ln_comm: {
			if(pk == '\n')
			{
				in.get(); //discard '\n
				buf="";
				mode = none;
		
			}
			else
				in.get(); //discard comment
		} break;

		case ln_comm1: {
			if(pk == '\n')
			{
				//in.get(); //discard '*'
				//pk = in.peek();
				//if(pk == '/'){
				//	in.get(); //discard '/'
					mode = prev_mode;
				//}
			}
			else
				in.get(); //discard comment

		} break;

		case cl_comm: {
			if(pk == '*')
			{
				in.get(); //discard '*'
				pk = in.peek();
				if(pk == '/'){
					in.get(); //discard '/'
					buf="";
					mode = none;
				}
			}
			else
				in.get(); //discard comment

		} break;
		case cl_comm1: {
			if(pk == '*')
			{
				in.get(); //discard '*'
				pk = in.peek();
				if(pk == '/'){
					in.get(); //discard '/'
					mode = prev_mode;
				}
			}
			else
				in.get(); //discard comment

		} break;

		//default:{} break;

	}  //case
		
  } while(!in.eof());
  
 
	  in.close();
return 0;

}
