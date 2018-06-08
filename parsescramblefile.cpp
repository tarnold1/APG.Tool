#include "stdafx.h"
#include "parsescramblefile.h" 
#include "globals.h"

extern void addPinScramble(string name);
extern void addScrambleMap(string name);
extern void addScramble(string val);



int ParseScrambleFile(string dirname,string fname){
	if (dirname.size() == 0) {
		dirname = "input_files\\";
	}

	//ofstream outfile;
	//outfile.open ("debug_files\\ParseScramble_output.txt");

	enum Mode{none,incl,kw_pin_scr,scr_name,kw_scr_map, map_name,kw_scramble, scr_val, ln_comm, cl_comm, cl_comm1 };

	int mode = none;
	int prev_mode = none;
	int sub_mode = none;
	//string inputFileName = G_INPUT_FILE;

	string buf;
	char pk,ch;
	//fname = inputFileName + "\\" + fname;
	//fname = "input_files\\" + fname;
	fname = dirname + fname;

	ifstream in( fname.c_str() , ios::in | ios::binary);

	  if(!in) {
		LogError("ParseScrmableFile", "Cannot open input file - pin_scramble.cpp."); 
		return 0;
	  }

	  Log("ParseScrambleFile","before do...");
	  do {

		
		pk = in.peek();
		#ifdef _debug_parsing_
		Log("ParseScrambleFile","mode : " + toString(mode) + " peek : " + toString (pk) ); //
		#endif
	
		if(mode == none){

			if(isspace(pk))
				in.get(ch);

			if(pk == '#'){
				buf="";
				in.get(); // discard #
				if(in.peek() == 'i')
				in.get(ch); // get 'P'
				buf+= ch;
				if(in.peek() == 'n')
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
					Log("ParseScrambleFile", "incl" );
				}
				else
				{
					LogError("ParseScrambleFile", pk + " : malformed file: expecting an \"include\" keyword");
					_getch();
					return -1;
				}
			}
			else if( pk == 'P'){
				Log("ParseScrambleFile","kw_pin_scr");
				mode = kw_pin_scr;
			}
			else if( pk == '(' ){
				if(prev_mode == kw_pin_scr){
					Log("ParseScrambleFile","scr_name" );
					mode = scr_name;
				}
				else if(prev_mode == kw_scr_map){
					Log("ParseScrambleFile","map_name");
					mode = map_name;
				}
				else if(prev_mode == kw_scramble){
					Log("ParseScrambleFile","scr_val" );
					mode = scr_val;
				}
				else if(prev_mode){
					
				}
				
				in.get(); //discard '(';
				
			}
			else if( pk == '{' ){

				in.get(); //discard '(';
				
			}
			else if( pk == '}' ){

				if(prev_mode == scr_val)
						prev_mode = scr_name;  


				in.get(); //discard '(';
				
			}

			else if( pk == 'S'){
				if(prev_mode == scr_name){
					Log("ParseScrambleFile","kw_scr_map");
					mode = kw_scr_map;
				}
				else{
					Log("ParseScrambleFile","kw_scramble" );
					mode = kw_scramble;
				}
				
			}
			else if(pk == '/'){
				Log("ParseScrambleFile","comment");
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
					Log("ParseScrambleFile", pk + " : malformed file: expecting a '/' - if this is a comment.");
					_getch();
					return 0;
				}
			}
				
		}
		else // mode != none
		{
			if( mode!= ln_comm && mode != cl_comm && mode != cl_comm1  && mode != incl) // watching for comment if inside a comment

				if(pk == '/'){

				in.get(); // discard '/';
			
				pk = in.peek();

				if( pk == '*')
				{
					in.get(); // discard '*'
					prev_mode =  mode;
					mode = cl_comm1;
					Log("ParseScrambleFile","enclosed comment");

				}
				if( pk == '/')
				{
					in.get(); // discard '/'
					prev_mode = mode;
					mode = cl_comm;
				}
				else
				{
					LogError("ParseScrambleFile","malformed file: expecting a'*'  - if this is an inside comment or an '/' if line comment.\n");
					_getch();
					return 0;
				}

			 }
			
		}

		switch(mode){
		case incl:{
			if(pk == '\n')
			{
				in.get(); //discard '\n
				Log("ParseScrambleFile", buf );
				buf="";
				mode = none;
			}
			else{

				in.get(ch); //discard comment
				buf+=ch;
			}
		} break;
		case kw_pin_scr: {
		in.get(ch); // get 'P'
		buf+= ch;
		if(in.peek() == 'I')
		in.get(ch);
		buf+= ch;
		if(in.peek() == 'N')
		in.get(ch);
		buf+= ch;
		if(in.peek() == '_')
		in.get(ch);
		buf+= ch;
		if(in.peek() == 'S')
		in.get(ch);
		buf+= ch;
		if(in.peek() == 'C')
		in.get(ch);
		buf+= ch;
		if(in.peek() == 'R')
		in.get(ch);
		buf+= ch;
		if(in.peek() == 'A')
		in.get(ch);
		buf+= ch;
		if(in.peek() == 'M')
		in.get(ch);
		buf+= ch;
		if(in.peek() == 'B')
		in.get(ch);
		buf+= ch;
		if(in.peek() == 'L')
		in.get(ch);
		buf+= ch;
		if(in.peek() == 'E')
		in.get(ch);
		buf+= ch;
		
		
		Log("ParseScrambleFile", buf );
		buf="";
		prev_mode = mode;
		mode = none;
				
		} break;
		case scr_name: {

			if(isspace(pk)){
				in.get(); //discard whitespace
			}
			else if(pk == ')'){
				in.get(); //discard ')'
				 Log("ParseScrambleFile","scr_name : " + buf );
				 //create a  new pin_scramble and assign this name
				addPinScramble(buf);
				buf="";
				prev_mode = mode;
				mode = none;
			}
			else{
				in.get(ch);
				buf+= ch;
			}

		} break;
		case kw_scr_map: {
			in.get(ch); // get 'S'
			buf+= ch;
			if(in.peek() == 'C')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'R')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'A')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'M')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'B')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'L')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'E')
			in.get(ch);
			buf+= ch;
			if(in.peek() == '_')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'M')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'A')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'P')
			in.get(ch);
			buf+= ch;

			Log("ParseScrambleFile", buf );
			buf="";
			prev_mode = mode;
			mode = none;
			
		} break;
		case map_name: {

			if(isspace(pk)){
				in.get(); //discard whitespace
			}
			else if(pk == ')'){
				in.get(); //discard ')'
				 Log("ParseScrambleFile","map_name : " + buf );
				 //create a new scramble_map,assign it with this name, add to last pin_scramble's scramble_map list
				 addScrambleMap(buf);
				buf="";
				prev_mode = mode;
				mode = none;
			}
			else{
				in.get(ch);
				buf+= ch;
			}

		} break;
		case kw_scramble: {
			in.get(ch); // get 'S'
			buf+= ch;
			if(in.peek() == 'C')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'R')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'A')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'M')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'B')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'L')
			in.get(ch);
			buf+= ch;
			if(in.peek() == 'E')
			in.get(ch);
			buf+= ch;

			Log("ParseScrambleFile", buf );
			buf="";
			prev_mode = mode;
			mode = none;
		}break;

		case scr_val: {

			if(isspace(pk)){
				in.get(); //discard whitespace
			}
			else if(pk == ')'){
				in.get(); //discard ')'
				 Log("ParseScramble" ,"scr_val : " + buf );
				 // create a new pair of value and add on to the recent scramble_map
				 addScramble(buf);
				buf="";
				prev_mode = mode;
				mode = none;
			}
			else{
				in.get(ch);
				buf+= ch;
			}
		}break;
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
					buf="";
					mode = prev_mode;
				}
			}
			else
				in.get(); //discard comment

		} break;


		
		//default:{} break;
					  }  
		
  } while(!in.eof());
  
	  in.close();
return 0;

}
