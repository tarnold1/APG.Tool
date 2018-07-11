// Pattern.cpp: implementation of the Pattern class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Pattern.h"
#include  <bitset>
#include <windows.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include "globals.h"

#include  "defs.h"
#include <regex>

#undef ENUM_BEGIN
#undef ENUM
#undef ENUM_END

#define ENUM_BEGIN(typ) const char * typ ## _table [] = {
#define ENUM(nam) #nam
#define ENUM_END(typ) };

#include  "enums.h"

#undef ENUM_BEGIN
#undef ENUM
#undef ENUM_END


Pattern::Pattern(){

	m_VectorLine.resize(G_VEC_LINE_SIZE,'X');
	m_rpt_cnt = "";
	m_MarDone = false;
	m_listVecConv =  new vector<string>; 
	m_listReqReg = new vector<string>;
	m_listPatInst =  new vector<PatInst>; 
	m_listComments =  new vector<string>;
	m_listDriveOnlyPins = new vector<string>;
	m_CurIndex = 0;
	m_TsetSel = "TSET1";
	m_PsMapSel = "PS1";
	m_VihhSel = "VIHH1";
	m_CounterSel = 0;
	m_isRead = false;
	m_isSubPat = false;
	m_isSubVecDefDefined = false;
	m_NestLevel =  0;
	m_SubVecDef ="";
	type = "";
	m_isCount = false;
	m_currentPat = "";
};
Pattern::~Pattern(){};

bool Pattern::isInLoop(){
return m_NestLevel > 0;
}

void Pattern::SnapCjmpz(string lbl_name){
	vector<long> counters;

	Log("SnapCjmpz", lbl_name);

	for (size_t c = 0;  c < 61; c++ ){
		counters.push_back(G_COUNTERS[c]);
		Log( toString(c), toString(counters.back()) );
	}

	m_CjmpzMap[lbl_name] = counters;

}
void Pattern::PopCjmpz(string lbl_name){
	//map<string,vector<long>>::iterator itr = m_CjmpzMap.find(lbl_name);
	//if(itr != m_CjmpzMap.end())

	m_CjmpzMap.erase(lbl_name);
}
bool Pattern::FindInCjmpzMap(string lbl_name){
	map< string,vector<long> >::iterator itr = m_CjmpzMap.find(lbl_name);
	if(itr != m_CjmpzMap.end()){
		return true;
	}
	else{
		return false;
	}
}

bool Pattern::GetCtrVal(string lbl_name, int counter, long &val){

	map< string,vector<long> >::iterator itr = m_CjmpzMap.find(lbl_name);
	if(itr != m_CjmpzMap.end()){
		val = itr->second.at(counter);
		return true;
	}
	else {
		return false;
	}
}

void Pattern::ClearVecLines(){

	m_listVecConv->clear();
	m_listVecConv =  new vector<string>; 

	
	for (size_t  b = 0; b < m_listPatInst->size();  b++){

		m_listPatInst->at(b).clearVecLines();
	}
}
int Pattern::InvertData(int dat_in) {
	int data_in = dat_in;
	int c, k;
	string binary_str;
	string binary_str_inv;
	for (c = 31; c >= 0; c--) {
		k = data_in >> c;
		if (k & 1) {
			//printf("1");
			binary_str = binary_str + "1";
			binary_str_inv = binary_str_inv + "0";
		}
		else {
			//printf("0");
			binary_str = binary_str + "0";
			binary_str_inv = binary_str_inv + "1";
		}
	}
	int bin_2_lng = std::bitset<32>(binary_str_inv).to_ulong();
	return bin_2_lng;
}

void Pattern::CommitVecLines(){

int pat_idx = 0;
bool vd_once = false;

if(!m_isSubPat){
	m_listVecConv->push_back(orig_name);
}

for (size_t  b = 0; b < m_listComments->size();  b++){
	
	if(m_listComments->at(b).compare("// %") == 0){
		if(!vd_once){
			vd_once = true;
			if(!m_isSubPat)
				m_listVecConv->push_back(G_VECDEF);
			
		}

		// head tags
		for (size_t  c = 0; c < m_listPatInst->at(pat_idx).m_listHeadTags->size(); c++) {
			m_listVecConv->push_back(m_listPatInst->at(pat_idx).m_listHeadTags->at(c) );
			
		}
		// vec lines
		for (size_t  d = 0; d < m_listPatInst->at(pat_idx).m_listVecConverted->size(); d++) {
			m_listVecConv->push_back(m_listPatInst->at(pat_idx).m_listVecConverted->at(d));
			Log("CommitVecLines", toString(d) + " " + m_listVecConv->back() );
			//if (G_DEBUG_ON)
			//	_getch();
		}
		// Tail tags
		for (size_t  e = 0; e < m_listPatInst->at(pat_idx).m_listTailTags->size(); e++) {
			m_listVecConv->push_back( m_listPatInst->at(pat_idx).m_listTailTags->at(e));	
		}
		pat_idx++;
	}
	else{ // if comment
		string cm = m_listComments->at(b);
		//int idx = cm.find_last_of("\r");
		//if(idx!=-1)
		//	cm = cm.substr(0,idx-1);
		
		// if (G_ADD_ORIG) // Option to remove comments if /no_orig_code argument used. - TAZ
		m_listVecConv->push_back(cm);
	}
}

if (G_DEBUG_ON && m_isSubPat){
	for (size_t  c = 0; c < m_listVecConv->size(); c++) {
			Log("CommitVecLines", m_listVecConv->at(c) ); 	
		}
	_getch();

}
}

void Pattern::SetTriStatePins(){
  
	int idx;
	vector<int> idxs;
	for (size_t  c = 0; c < vTriStatePins.size(); c++){
	//	cout <<"vTriStatePins : " << vTriStatePins.at(c) << endl;
		if( FindVecDefIndex(vTriStatePins.at(c),idx) ){
					idxs.push_back(idx);
					debugfile <<"SetTriStatePins - idx: " << idx  << " = " <<  vTriStatePins.at(c)<< endl;
		}		
	} 
	SetVecLineBits(idxs,'X');
}

void Pattern::setIndex (int idx){
	index = idx;
}
void Pattern::setName (string nm){
	name = nm;
}
int Pattern::getIndex (){
	return index;
}
void Pattern::setSubVecDef(string vecdef){
	m_SubVecDef =  vecdef;
}
	
string Pattern::getSubVecDef(){
	return m_SubVecDef;
}
	
string Pattern::getName (){
	return name;
} 
void Pattern::addPatInst (PatInst inst){

	m_listPatInst->push_back(inst);
}
int Pattern::getListSize (){
	return m_listPatInst->size();
}
void Pattern::printdata(ofstream &outfile){
	outfile << index << " : " << name << '\n';

	for (size_t  i = 0;  i < m_listPatInst->size() ; i++){
		m_listPatInst->at(i).printdata(outfile);
	}
}
void Pattern::UpdateLstIndexes(){

	string str;

	for (size_t  i = 0;  i < m_listPatInst->size() ; i++){
		m_listPatInst->at(i).setIndex(i);
	}

}

void Pattern::addHeadTag(int patinst_idx, string tag){
	
	Log("addHeadTag : ", tag);
	int idx =-1;

	if(FindFromList(m_listPatInst->at(patinst_idx).m_listHeadTags,tag,idx))
		return;

	if(m_listPatInst->at(patinst_idx).m_listHeadTags->size() > 0){
		m_listPatInst->at(patinst_idx).m_listHeadTags->insert(m_listPatInst->at(patinst_idx).m_listHeadTags->begin(),tag);
	}
	else{			
		m_listPatInst->at(patinst_idx).m_listHeadTags->push_back(tag);
	}

}

void Pattern::addTailTag(int patinst_idx, string tag){

		Log("addTailTag :", tag);
		m_listPatInst->at(patinst_idx).m_listTailTags->push_back(tag);

}

void Pattern::InsertStartLoopTag(int patinst_idx, int counter_val){
	string tag("STARTLOOP ");
	tag += toString(counter_val);
	Log("InsertStartLoop", "STARTLOOP : " + tag + "\r\n");
	
	if(m_listPatInst->at(patinst_idx).m_listHeadTags->size() > 0){
		m_listPatInst->at(patinst_idx).m_listHeadTags->insert(m_listPatInst->at(patinst_idx).m_listHeadTags->begin(),"%VEC");
		m_listPatInst->at(patinst_idx).m_listHeadTags->insert(m_listPatInst->at(patinst_idx).m_listHeadTags->begin(),tag);
		//vvsPATTERNS.at(G_PAT_IDX).m_listPatInst->at(patinst_idx).m_listHeadTags->push_back("%VEC");
		//vvsPATTERNS.at(G_PAT_IDX).m_listPatInst->at(patinst_idx).m_listHeadTags->push_back( tag);
		Log("HeadTag : ", m_listPatInst->at(patinst_idx).m_listHeadTags->back());
		Log("PatInst idx : ", toString(m_listPatInst->at(patinst_idx).index));

	}
	else{
		
		m_listPatInst->at(patinst_idx).m_listHeadTags->push_back(tag);
		Log("HeadTag : ", m_listPatInst->at(patinst_idx).m_listHeadTags->back());
	}
	
}



void Pattern::InsertEndLoopTag(int patinst_idx, string comment){

	if(comment.size() > 0){
		Log("TailTag : ", comment);
		m_listPatInst->at(patinst_idx).m_listTailTags->push_back(comment);
	}
	else{
	  	Log("TailTag : ", "ENDLOOP");
		m_listPatInst->at(patinst_idx).m_listTailTags->push_back("ENDLOOP");

	}
}


bool Pattern::FindLabelIndex(string jmp_lbl,int &idx){

for ( vector<PatInst>::iterator it_pat = m_listPatInst->begin(); it_pat != m_listPatInst->end(); ++it_pat){
	Log(" FindLabelIndex ", jmp_lbl + " == " +  it_pat->label);
	if(jmp_lbl.compare(it_pat->label) == 0){
	idx = it_pat->index;
	return true;
	}
}
return false;
}

PatInst* Pattern::getCurPatInst(){
	return &m_listPatInst->at(m_CurIndex);
}


void Pattern::SetVecLineBits(vector<int> &idxs, char ch){
	//cout << m_VectorLine << endl;
	//_getch();
	for(size_t  c=0; c <idxs.size(); c++ ){
	/*G_VECTOR_LINE*/m_VectorLine[idxs.at(c)] = ch;
	}
	//cout << m_VectorLine << endl;
	//_getch();
}
void Pattern::f_YAlu(vector<string> &params){

unsigned long source;



for(size_t  c = 0; c < params.size(); c++){
	
	#ifdef _debug_on_
	Log("f_YAlu", "\tparam : " + params.at(c) ); //
	#endif
	string p = params.at(c);
	int idx;
	if( FindFromTable( ADD_GEN_OPS_table, p, idx) ){ // do not convert to upper when searching
	 
		switch(idx){
		case HOLD:{
			getCurPatInst()->isHoldYalu = true;
		}break;
		case OYMAIN:{source = G_YMAIN;}break;
			
		case OYBASE:{source = G_YBASE;}break;
		
		case OYFIELD:{source = G_YFIELD;}break;
		}// switch

	}//if
	else if( FindFromTable( YALU_OP_1_2_table, p, idx) ){ // do not convert to upper when searching
	 
		switch(idx){
		
		case YUDATA:{
			G_YADDR = 0;
			for(int i = 1; i <= 16; i++){

				if( G_UDATA & ( 1 << i) ) {
					G_YADDR |= (1 << i );
				}
			} 
			}break;

		} // switch
 // switch
	} // else if
		else if( FindFromTable( YALU_OPS_table, p, idx) ){ // do not convert to upper when searching
	 
		switch(idx){
		
		case DYMAIN:{
			G_YMAIN = G_YADDR;
			/*	
			for(int i = 17; i <= 32; i++){
	
					if( G_UDATA & ( 1 << i) ) {
					G_YMAIN = ~G_YMAIN & (1 << (i-16) );
					}

				} 
			*/	}break;

		} // switch

	}// else if


}// for


bool inv;
if(getCurPatInst()->isYAddrChanged)
inv = true;
else
inv = getCurPatInst()->isInLoop && (!getCurPatInst()->isHoldYalu) ;

for(int i = 0; i <= 15; i++){
	string datres("T_Y");
	datres +=  toString(i);
	vector<int> idxs;

	if(UnScrambleResource(datres,idxs,m_PsMapSel)){
		
		
		bool read_mode = getCurPatInst()->isRead ;
		
		if( source & ( 1 << i) ) { // if 1
			if(read_mode){
				if(inv == true )
					SetVecLineBits( idxs, 'R');
				else
					SetVecLineBits( idxs, 'H');
			
			}
			else
				if(inv == true)
					SetVecLineBits( idxs, 'S');
				else
					SetVecLineBits( idxs, '1');
		}
		else{ //else 0
		
			if(read_mode){
					if(inv == true){
						SetVecLineBits( idxs, 'S');
					}
					else
					{
						SetVecLineBits( idxs, 'L');
					}
		
			}
			else
				if(inv == true){
						SetVecLineBits( idxs, 'S');
					}
					else
					{
						SetVecLineBits( idxs, '0');
					}
			
		}
	}
}

}
void Pattern::f_XAlu(vector<string> &params){


long source;

for(size_t c = 0; c < params.size(); c++){

	#ifdef _debug_on_
	Log("f_XAlu", "\tparam : " + params.at(c) ); //
	#endif
	string p = params.at(c);
	int idx;
	if( FindFromTable( ADD_GEN_OPS_table, p, idx) ){ // do not convert to upper when searching
	 
		switch(idx){
		case HOLD:{
			getCurPatInst()->isHoldXalu = true;
		}break;
		case OXMAIN:{source = G_XMAIN;}break;
		case OXBASE:{source = G_XBASE;}break;
		case OXFIELD:{source = G_XFIELD;}break;

		} // switch

	}//if
	else if( FindFromTable( XALU_OP_1_2_table, p, idx) ){ // do not convert to upper when searching
	 
		switch(idx){
		
		case XUDATA:{
				G_XADDR = 0;
				for(int i = 17; i <= 32; i++){
	
					if( G_UDATA & ( 1 << i) ) {
					G_XADDR |=  (1 << (i-16) );
					}

				} 
				//cout << "G_XADDR = " << G_XADDR <<endl;
					}break;

		} // switch

	}// else if
	else if( FindFromTable( XALU_OPS_table, p, idx) ){ // do not convert to upper when searching
	 
		switch(idx){
		
		case DXMAIN:{
				G_XMAIN = G_XADDR;
				/*
				for(int i = 1; i <= 16; i++){
	
					if( G_XADDR & ( 1 << i) ) {
					G_XMAIN = ~G_XMAIN & (1 << i) );
					}

				} */
				// cout << "DXMAIN : G_XMAIN = " << G_XMAIN << endl;
				
					}break;

		} // switch

	}// else if


}// for

bool inv;
if(getCurPatInst()->isXAddrChanged)
inv = true;
else
inv = getCurPatInst()->isInLoop && (!getCurPatInst()->isHoldXalu);


//	cout << getCurPatInst()->label<< (getCurPatInst()->isInLoop ? "inLoop" : "notInLoop" )<<endl;
//	_getch();


for(int i = 0; i <= 15; i++){
	string datres("T_X");
	datres +=  toString(i);
	vector<int> idxs;

	if(UnScrambleResource(datres,idxs,m_PsMapSel)){
		
		
		bool read_mode = getCurPatInst()->isRead ;
		if( source & ( 1 << i) ) { // if 1
			if(read_mode){
				if(inv == true )
					SetVecLineBits( idxs, 'R');
				else
					SetVecLineBits( idxs, 'H');
			
			}
			else
				if(inv == true)
					SetVecLineBits( idxs, 'S');
				else
					SetVecLineBits( idxs, '1');
		}
		else{ //else 0
			if(read_mode){
					if(inv == true){
						SetVecLineBits( idxs, 'S');
					}
					else
					{
						SetVecLineBits( idxs, 'L');
					}
		
			}
			else
				if(inv == true){
						SetVecLineBits( idxs, 'S');
					}
					else
					{
						SetVecLineBits( idxs, '0');
					}
			
		}
	}
}


}
void Pattern::f_DatGen(vector<string> &params){

long source;
bool inv = false;
bool isProtected = false;
bool update = false;

if (params.size() == 2) { // Fix issue with jamjam,udatajam and datdat,udatadr sequence with vector upating before register load.  TAZ -011518
	ToUpper(params.at(0));
	ToUpper(params.at(1));
	if (( (params.at(0).find("JAMJAM") != -1) && (params.at(1).find("UDATAJAM") != -1) ) || ((params.at(0).find("DATDAT") != -1) && (params.at(1).find("UDATADR") != -1))) {
		string temp_parm = params.at(0);// Swap register command with source command to load register before vector build. TAZ -011518
		params.at(0) = params.at(1);
		params.at(1) = temp_parm;
	}
}

for(size_t c = 0; c < params.size(); c++){

	#ifdef _debug_on_
	Log("f_DatGen", "\tparam : " + params.at(c) ); //
	#endif
	string p = params.at(c);
	int idx;
	if( FindFromTable( DATGEN_OPS_table, p, idx) ){ // do not convert to upper when searching
	 
		switch(idx){
		case DATDAT:{
			source = G_DATREG;
			update = true; 
				}break;
		case JAMJAM:{
			source = G_JAMREG;
			update = true; 
			//Log("Load from jamreg", toString(source));
			 //_getch();
					}break;
		case NOTINV:{
			inv = false;
			isProtected = true;
					}break;
		case UDATAJAM :  {
			 G_JAMREG = G_UDATA;
			 //Log("Load udata to jamreg", toString(G_JAMREG));
			 //_getch();
						 } break;
		case UDATADR :  {
			 G_DATREG = G_UDATA;
					} break;
		case BUFBUF: { // Added for DBM register. - TAZ 05/25/2018
			 source = G_DBM;
			 update = true;
			 inv = true;
				   }break;
	
		case CMPLDR: { // Added for inverting datareg. - TAZ 05/04/2018
				G_DATREG = InvertData(G_DATREG);
				   }break;
		case CNTDNDR: { // Added for datareg+1. - TAZ 06/08/2018
			if (m_isCount) {
				G_DATREG = G_COUNTERS[m_CounterSel] - 1;
			}
			else {
				G_DATREG = G_DATREG - 1;
			}
					}break;
		case CNTUPDR: { // Added for datareg-1. - TAZ 06/08/2018
			if (m_isCount) {
				G_DATREG = G_COUNTERS[m_CounterSel] + 1;
			}
			else {
				G_DATREG = G_DATREG + 1;
			}
		}break;
		case EQFDIS :  {}
		case XYEQB :  {}
		case YEQB :  {}
		case XEQB :  {}
		case YEQBORF :  {}
		case XEQBORF :  {}
		case YLTB :  {}
		case XLTB :  {}
		case YLEB :  {}
		case XLEB :  {}
		case XYLTBYF :  {}
		case XYLEBYF :  {}
		case XYLTBXF :  {}
		case XYLEBXF :  {}
		case XEQYPN :  {}
		case XEQYBPN :  {}
		case CNTDNYN :  { // Added for yindex-1. - TAZ 06/08/2018
			G_YADDR = G_YADDR - 1;
				}break;
		case CNTUPYN :  { // Added for yindex+1. - TAZ 06/08/2018
			G_YADDR = G_YADDR + 1;
				}break;
		case HOLDYN :  {}
		case XORINV :  {}
		case BCKFEN :  {}
		case INVSNS:{if(!isProtected) inv = true;}break;

		} // switch
	//cout << DATGEN_OPS_table[idx] << endl;
	//_getch();
	}//if

}// for

//do the crunch work

//G_PSMAP_SEL

if (!getCurPatInst()->isRead && getCurPatInst()->isADHIZ){ //Fix to remove strobes from vectors aftera read which have an adhiz and without a read. TAZ-11182018
   replace(m_VectorLine.begin(), m_VectorLine.end(), 'H', G_DEFAULT_PIN_STATE);
   replace(m_VectorLine.begin(), m_VectorLine.end(), 'L', G_DEFAULT_PIN_STATE);
}

if(!update)
	return;

for(int i = 0; i <= 35; i++){
	string datres("T_D");
	datres +=  toString(i);
	vector<int> idxs;

	if(UnScrambleResource(datres,idxs,m_PsMapSel)){
	
		
		bool read_mode = getCurPatInst()->isRead; // && getCurPatInst()->isADHIZ; Fix read with adhiz bug.  TAZ -011518
		
		if (getCurPatInst()->isReadUdata) {  // if use UDATA as strobe mask
			if (G_UDATA & ((long)1 << i)) {

				//	cout << datres << " " << m_VectorLine  << " " << "isReadUdata && G_UDATA bit(i) == 1" << endl;
				//SetVecLineBits(idxs, '1'); // TAZ removed (Regression).
				//continue;

			}
			else {

				//	cout << datres << " " << m_VectorLine  << " " << "isReadUdata && G_UDATA bit(i) == 0" << endl;          

				SetVecLineBits(idxs, '0');

				continue;
			}
		}
		if( source & ((long) 1 << i) ) { // if integer[i] bit value is 1
			if(read_mode/*m_isRead*/){
				if(inv == true )
					SetVecLineBits( idxs, 'R'); // DSP Recieve alias
				else		
					SetVecLineBits( idxs, 'H');
			}
			else {
				if (inv == true)
				{
					SetVecLineBits(idxs, 'S'); // DSPSEND alias
				}
				else
				{
					if (!getCurPatInst()->isRead && !getCurPatInst()->isADHIZ) // noread && !adHIZ == copy data
					{
						if (G_ADHIZ == DRIVE)
						{
							SetVecLineBits(idxs, '1');
						} //drive data TAZ Debug
						else if (G_ADHIZ == RECEIVE)
						{
							SetVecLineBits(idxs, 'X');
						}
					}
					else if (!getCurPatInst()->isRead && getCurPatInst()->isADHIZ) // noread && adHIZ = X
					{
						if (G_ADHIZ == DRIVE)
						{
							SetVecLineBits(idxs, 'X');
						}
						else if (G_ADHIZ == RECEIVE)
						{
							SetVecLineBits(idxs, '1');
						}
					} //drive data
					SetTriStatePins();

				} // non inv mode block end.
			} // write mode block end.
		} // Bit is 1 logic block end.
		else{							  // if integer[i] bit value is 0

			
		//	cout << "if integer[i] bit value is 0" << endl;
			if(read_mode/*m_isRead*/){
					if(inv == true){
				  		SetVecLineBits( idxs, 'R');
					}
					else
					{
						SetVecLineBits( idxs, 'L');
					}
		
			}
			else {
				if (inv == true) {
					SetVecLineBits(idxs, 'S');
				}
				else
				{
						if (!getCurPatInst()->isRead && !getCurPatInst()->isADHIZ)
						{
							if (G_ADHIZ == DRIVE)
							{
								SetVecLineBits(idxs, '0');
							} //drive data TAZ Debug
							else if (G_ADHIZ == RECEIVE)
							{
								SetVecLineBits(idxs, 'X');
							}
						}
						else if (!getCurPatInst()->isRead && getCurPatInst()->isADHIZ)
						{
							if (G_ADHIZ == DRIVE)
							{
								SetVecLineBits(idxs, 'X');
							}
							else if (G_ADHIZ == RECEIVE)
							{
								SetVecLineBits(idxs, '0');
							}
						} //drive data
						SetTriStatePins();
					} // inv == false
				} // read_mode = false
			} // Bit is 0 logic block end.
		} // UnScrambleResource logic block
	} // T_D0-35 for loop block

}

void Pattern::f_Chips(vector<string> &params){

Log("f_Chips", "CHIPS\r\n");

int idx, op = 0;
for(size_t c = 0; c < params.size(); c++)
{
	#ifdef _debug_on_
	Log("f_Chips", "\tparam : " + params.at(c) ); //
	#endif
	string p = params.at(c);
	
	if( FindFromTable( CHIPS_OP_table, p, idx) ){ // do not convert to upper when searching
		 
		switch(idx){
		case NOCLKS:{}break;
		case LBDATA:{}break;
		//case RESET:{}break; //TAZ -  case 1 needs to be reserved for cs1t from the CHIPS_OP_table ENUM table. 
							  //Reset was also 1 but in another ENUM Table 1 so cs1t was not processed.
		default:{
			ToUpper(p);
			string cs = p.substr(0,3);
			
			string sig = p.substr(3);
			int cs_idx = atoi(cs.substr(2,1).c_str()) -1;
			Log("f_Chips" ,cs + " , " + toString(cs_idx) + " , " + sig + "\r\n");
			if(sig.compare("PT") == 0)
				sig="T";
			if(sig.compare("PF") == 0)
				sig="F";

			if((sig.compare("F") == 0)  || (sig.compare("T")==0)){
				
				vector<int> idxs;
				cs = "T_" + cs;
				if(UnScrambleResource(cs,idxs, m_PsMapSel)){
					bool flg;
					if( sig.compare("T") == 0)
						flg = true;
					else
						flg = false;

					if (G_CS_ACTIVE_H[cs_idx]){ //if active high
					  if(flg == true)
						SetVecLineBits( idxs, '1');
					  else
						 SetVecLineBits( idxs, '0');
					}
					else{
						if(flg == true)
						 SetVecLineBits( idxs, '0');
					  else
						 SetVecLineBits( idxs, '1');
					}
					
				for(size_t c=0; c < idxs.size(); c++ )
					debugfile << "m_Vector_Line[" << idxs.at(c)<< " ] = " << m_VectorLine[ idxs.at(c) ]<< endl;
				}
			

			}
			//else if(sig == "T"){}
			else if(sig == "RDF" || sig == "RDT" ){

				string sn = sig.substr(2);
				
				vector<int> idxs;
				cs = "T_" + cs;
				if(UnScrambleResource(cs,idxs, m_PsMapSel)){
					bool flg;
					
					if( sn.compare("T") == 0)
						flg = true;
					else
						flg = false;

					//if(flg)
					//	debugfile << "flg=true" << endl;
					//else
					//	debugfile << "flg=false" << endl;


					if (G_CS_ACTIVE_H[cs_idx]){ //if active high
					  if(flg == true)
						SetVecLineBits( idxs, 'H');
					  else
						 SetVecLineBits( idxs, 'L');
					}
					else{
						if(flg == true)
						 SetVecLineBits( idxs, 'L');
					  else
						 SetVecLineBits( idxs, 'H');
					}
					
				for(size_t c=0; c < idxs.size(); c++ )
					debugfile << "m_Vector_Line[" << idxs.at(c)<< " ] = " << m_VectorLine[ idxs.at(c) ]<< endl;
				}
		}
			//else if(sig == "RDT"){}
			else if(sig == "HIZ"){

				vector<int> idxs;
				cs = "T_" + cs;
				if(UnScrambleResource(cs,idxs, m_PsMapSel)){
				
				SetVecLineBits( idxs, 'X');
					
				for(size_t c=0; c < idxs.size(); c++ )
					debugfile << "m_Vector_Line[" << idxs.at(c)<< " ] = " << m_VectorLine[ idxs.at(c) ]<< endl;
				}

			
			}
			else if(sig == "DV"){}
			else if(sig == "DZ"){}

				} break;
			} //switch
		} // if( FindFromTable )
	
	}// for	
}
void Pattern::f_PinFunc(vector<string> &params){

Log("f_PinFunc","PINFUNC\r\n");
// TAZ need to reset m_VectorLine to default pin state before begining new micro instruction.
int idx, op;
for(size_t c = 0; c < params.size(); c++)
{
	
	Log("f_PinFunc", "\tparam : " +  params.at(c) + "\r\n");

	if( FindFromTable( PINFUNC_OP_1_table, params.at(c), idx) ){
		/*G_PSMAP_SEL*/ m_PsMapSel = PINFUNC_OP_1_table[idx];
	op = 1;
	}
	else if( FindFromTable( PINFUNC_OP_2_table, params.at(c), idx) ){
		/*G_VIHH_SEL*/ m_VihhSel = PINFUNC_OP_2_table[idx];
	op = 2;	
	}
	else if( FindFromTable( PINFUNC_OP_3_table, params.at(c), idx) ){
			/*G_TSET_SEL*/m_TsetSel = PINFUNC_OP_3_table[idx];
	op = 3;
	}
	else if( FindFromTable( PINFUNC_OP_4_table, params.at(c), idx) ){
		op = 4;
		if(idx == ADHIZ)
			getCurPatInst()->isADHIZ = true;
		else if(idx == VPULSE){
			getCurPatInst()->isVPULSE = true;

			}
		}
	}
}
void Pattern::f_Count(vector<string> &params){

Log("f_Count","COUNT");


int idx;
int count_str_size;
string count_check;
string aon_check;
for(size_t c = 0; c < params.size(); c++)
{
	#ifdef _debug_on_
	Log("f_", "\tparam : " + params.at(c) ); //
	#endif
	switch(c){ // operand
	case 0:{ // 
		int idx;
		if( FindFromTable( COUNT_OP_1_table, params.at(c), idx) ){
			// Added for reload counter fix. - TAZ 05/06/2018
			m_isCount = true;
			count_str_size = 5;
			count_check = params.at(0);
			ToUpper(count_check);
			if (count_check.find("RELOAD") != -1)
			{
				count_str_size = 6;
				Log("count_str_size", toString(count_str_size));
			 }

			string cnt = params.at(c).substr(count_str_size).c_str();
			m_CounterSel = atoi(cnt.c_str())-1 ;
			string msg =  "counter:";
				msg.append( COUNT_OP_1_table[idx] );
				msg.append(" = " + toString(G_COUNTERS[m_CounterSel]) ); 
			Log("f_Count", msg) ;
			Log("G_COUNTER_SEL = ",toString(m_CounterSel));
			
		}
		   } break;
	case 1:{
			if( FindFromTable( COUNT_OP_2_table, params.at(c), idx) ){
				switch(idx){
				case COUNTUDATA: {
					if (count_check.find("RELOAD") != -1) // Added for reload counter fix. - TAZ 05/06/2018
					{
						G_COUNTERS_RLD[m_CounterSel] = G_UDATA;
						Log("G_COUNTERS_RLD", params.at(0));
					}
					if (count_check.find("COUNT") != -1)
					{
						G_COUNTERS[m_CounterSel] = G_UDATA;
					}
					Log("G_COUNTERS[" + toString(m_CounterSel) +"] = G_UDATA = ",toString(G_COUNTERS[m_CounterSel]));
					}break;
				} //switch idx
			}
			if (FindFromTable(COUNT_OP_3_table, params.at(c), idx)) { // Added for reload counter fix. - TAZ 05/06/2018
				switch (idx) {
				case AON: {
					if (G_COUNTERS[m_CounterSel] == 0)
						G_COUNTERS[m_CounterSel] = G_COUNTERS_RLD[m_CounterSel];
					Log("G_COUNTERS[" + toString(m_CounterSel) + "] is reloaded with ", toString(G_COUNTERS[m_CounterSel]));
					//if(G_DEBUG_ON)
					//_getch();
				}break;
				} //switch idx
			 }
		   } break;

	case 2:{
			if( FindFromTable( COUNT_OP_3_table, params.at(c), idx) ){
				switch(idx){
				case AON: { 
					if(G_COUNTERS[m_CounterSel] == 0)
					G_COUNTERS[m_CounterSel] = G_COUNTERS_RLD[m_CounterSel];
					Log("G_COUNTERS[" + toString(m_CounterSel) +"] is reloaded with ",toString(G_COUNTERS[m_CounterSel]));
					//if(G_DEBUG_ON)
					//_getch();
					}break;
				} //switch idx
			}
		   } break;
		}
	} // for
}
void Pattern::f_Mar(vector<string> &params){
int jmp_cond = -1;
string jmp_lbl;
m_rpt_cnt = "";

for(size_t c = 0; c < params.size(); c++)
{
	#ifdef _debug_on_
	Log("f_Mar", "\tparam : " + params.at(c) ); //
	#endif
	int idx;
	switch(c){ // operand
	case 0:{ // 
		
		if( FindFromTable( MAR_OP_1_table, params.at(c), idx) ){
			debugfile << "MAR_OP_1:" << MAR_OP_1_table[idx] << endl;
			jmp_cond = idx;
		}
		if ((params.at(c)) == "read") { getCurPatInst()->isRead = true; } // TAZ Fix for "mar read" uinst. The read arg can be in multiple positions. Example: mar read/mar arg1,arg2,read 
		if ((params.at(c)) == "readudata") { getCurPatInst()->isRead = true; getCurPatInst()->isReadUdata = true; } // TAZ Fix for "mar isReadUdata" uinst. The isReadUdata arg can be in multiple positions. Example: mar isReadUdata/mar arg1,arg2,isReadUdata 
	} break;
	case 1:{
		if( FindFromTable( MAR_OP_2_table, params.at(c), idx) ){
			debugfile << "MAR_OP_2:" << MAR_OP_1_table[idx] << endl;
		}else
			if(params.at(c).size() > 0){
				jmp_lbl = trim(params.at(c));
			}
		   } break;
	case 2:{
		if( FindFromTable( MAR_OP_3_table, params.at(c), idx) ){
			debugfile << "MAR_OP_3:" << MAR_OP_1_table[idx] << endl;

			switch(idx){
			case READ: {
				//m_isRead = true;
				getCurPatInst()->isRead = true;
					}break;
			case NOREAD:{
			//m_isRead = true;
				getCurPatInst()->isRead = false;
					}break;
			case READUDATA:{
			//m_isRead = true;
				getCurPatInst()->isRead = true;
				getCurPatInst()->isReadUdata =true;
					}break;

			
			}
		}
		   } break;


	} // switch
} //for

if(jmp_cond == -1)
	return;

int idx;
switch( jmp_cond ){
case RETURN:{m_MarDone = true; } break;	
case DONE:{m_MarDone = true; } break;
	case GOSUB:{
		Log("GOSUB", jmp_lbl);
		m_GoSubLabel = jmp_lbl;
			   }break;
	case CJMPE:{}
	case CJMPNE:{}
	case CJMPZ:{
	/*	Log("CJMPZ",jmp_lbl);
		addTailTag(m_CurIndex,"mar jump, " + jmp_lbl);
		if(FindLabelIndex(jmp_lbl, idx)){
			addHeadTag(idx, "// " + jmp_lbl + ":");
		}
	

	*/
	if(!FindInCjmpzMap(jmp_lbl))
		SnapCjmpz(jmp_lbl);
	}
	case JUMP:{
		Log("JUMP",jmp_lbl);
		addTailTag(m_CurIndex,"var jump, " + jmp_lbl);
		if(FindLabelIndex(jmp_lbl, idx)){
			addHeadTag(idx, "// " + jmp_lbl + ":");
		}

	}break;

	case CJMPNZ:{

		Log("CJMPNZ", jmp_lbl);

		
		if(FindLabelIndex(jmp_lbl, idx)){
			Log("Label found : ", jmp_lbl + " index : "  + toString(idx));

			if(m_CurIndex == idx){

				G_RPT_OPT = "RPT ";
				Log("f_Mar, RPT, ",G_RPT_OPT );
			
			}
			else{
				if(idx < m_CurIndex ){

					
					InsertStartLoopTag(idx,G_COUNTERS[m_CounterSel] + 1);
					InsertEndLoopTag(m_CurIndex,"ENDLOOP // " + toString(G_COUNTERS[m_CounterSel]+1));
					Log("f_Mar", "idx < m_CurIndex : " + idx ); 

				}	
				else{	//InsertEndLoopTag(idx,"this instruction sets a jump to a location beyond its index");
						m_JumpLabel = jmp_lbl;
						addTailTag(m_CurIndex,"var jump, " + jmp_lbl);
						//if(FindLabelIndex(jmp_lbl, idx)){
						//	addHeadTag(idx, "// " + jmp_lbl + ":");
					}
				

			}
		
		} // if FindLabel...
	}break;
default:{}break;

} //switch
}

void Pattern::f_Udata(vector<string> &params){

for(size_t c = 0; c < params.size(); c++)
{
	#ifdef _debug_on_
	Log("f_Udata", "\tparam : " + params.at(c) ); //
	#endif
	//int idx;
	switch(c){ // operand
		case 0:{
			string param = params.at(c);
			if(param.find(".") != -1){
				param = "0x" + param.substr(param.find(".")+1);
				unsigned long val;
				if(StrTol(param, val)){
					G_UDATA = val;
					Log(param, toString(G_UDATA));
				}
				else{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
					LogError("f_Udata","Failed to convert " + param + " to long.");
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
					if(G_HALT_E)
						_getch();
				}
			}
			else if(param.find("0x") != -1){
				unsigned long val;
				if(StrTol(param, val)){
					G_UDATA = val;
					Log(param, toString(val));
				}
				else{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
					LogError("f_Udata","Failed to convert " + param + " to long.");
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
					if(G_HALT_E)
						_getch();
				}
			}
			else{
				G_UDATA = atoi(trim(param).c_str());
			}

		}break;
	}// switch

	
} // for
	

}// f_Udata()

string Pattern::RemoveChar(string string_in, string remove_char) {
	for (size_t c = 0; c < remove_char.size(); c++) {
		string_in.erase(std::remove(string_in.begin(), string_in.end(), remove_char.at(c)), string_in.end());
	}
	return string_in;
}

void Pattern::f_Vec(vector<string> &params,string vecdef){
	// Added input for vecdef change.TAZ -011518
string final_bits; 
int idx;
m_rpt_cnt = "";
if (params.size() == 0) { return; } // TAZ for none VEC statement

//###############################################################
// Added logic for multiple vecdef handling.TAZ -011518
//###############################################################
vector<string> vecdef_str = Tokenize(vecdef, ' ');
vecdef = "";

for (size_t c = 0; c < vecdef_str.size(); c++) {
	string str = vecdef_str.at(c);
	if (trim(str).length() > 0)
		if (FindDef(str))
			vecdef += str;
		else {
			vecdef += trim(vecdef_str.at(c));
		}

}
int id = vecdef.find("VECDEF");
if (id != -1)
vecdef = vecdef.substr(id + 6);
vecdef_str = Tokenize(vecdef, ',');
vecdef = "";
//###############################################################



for(size_t c = 0; c < params.size(); c++)
{
	
	//cout << "f_Vec" << "\tparam : " << params.at(c) << endl; //
	//int idx;
	
	switch(c){ // operand
		case 0:{

			//cout << "final_bits : " << final_bits << endl;

			if((type.compare("mixedsync") == 0) || (type.size() == 0))
			{
			vector<string> bits = Tokenize(params.at(c),' ');

			for(size_t c = 0; c < bits.size(); c++){	
				final_bits += bits.at(c);
			//	cout << "bits.at(c) : " << bits.at(c) << endl;
			}
			 final_bits = RemoveChar(final_bits," \t");

				m_Prefix =  "VEC";
				if (m_SubVecDef.length() > 0) {					
					vector<string> pins;
					if (vecdef_str.size() > 0) {
						pins = vecdef_str;
					}
					else {
						pins = Tokenize(m_SubVecDef, ',');
					}
					int idx;
					for(size_t d = 0; d < pins.size(); d++){
						if(trim(pins.at(d)).compare("X")==0)
							continue;

						if( FindVecDefIndex(pins.at(d),idx) ){
							if(d < final_bits.length())
								m_VectorLine[idx] = final_bits.at(d);
							else
								LogWarning("mixedsync", "vector width is less than VECDEF width");
						}
						else{
							LogWarning("mixedsync", "pin name " + pins.at(d) + " not in vector defnition");
						}

					}
				}
				else
				{
					for(size_t d = 0; d < final_bits.size(); d++){

						m_VectorLine[d] = final_bits.at(d);
					}
				}
			}
			//cout << "m_VectorLine : " << m_VectorLine << endl;
			//_getch();
			
		}break;
			// Added Vihh Selection capabilty to Vec uinstruction. - TAZ
		case 1: {
			if (FindFromTable(PINFUNC_OP_2_table, params.at(c), idx)) {
				/*G_VIHH_SEL*/ m_VihhSel = PINFUNC_OP_2_table[idx];
			}
		}break;
	}// switch
} // for

	vector<int> idxs;

	if(m_SubVecDef.length() > 0){

		vector<string> pins;
		if (vecdef_str.size() > 0) {
			pins = vecdef_str;
		}
		else {
			pins = Tokenize(m_SubVecDef, ',');
		}

		int idx;
		for(size_t d = 0; d < pins.size(); d++){
			if(trim(pins.at(d)).compare("X")==0)
				continue;

			if( FindVecDefIndex(trim(pins.at(d)),idx) ){
				idxs.push_back(idx);
			}
		}
		for(size_t d = 0; d < m_VectorLine.size(); d++){
			bool add = false;
			for(size_t  e = 0; e < idxs.size(); e++)
				if(idxs.at(e) == d){
					add = true;
					break;
				}

			if(!add)
				 m_VectorLine[d]=G_DEFAULT_PIN_STATE;
		}
	}

}// f_Vec()


void Pattern::f_Rpt(vector<string> &params,string vecdef){

string str_cnt;
string final_bits;

if (params.size() == 0) { return; } // TAZ for no RPT statement/parameter.
vector<string> vecdef_str = Tokenize(vecdef, ' ');
vecdef = "";

for (size_t c = 0; c < vecdef_str.size(); c++) {
	string str = vecdef_str.at(c);
	if (trim(str).length() > 0)
		if (FindDef(str))
			vecdef += str;
		else {
			vecdef += trim(vecdef_str.at(c));
		}
}

int id = vecdef.find("VECDEF");
if (id != -1)
vecdef = vecdef.substr(id + 6);
vecdef_str = Tokenize(vecdef, ',');
vecdef = "";

for(size_t c = 0; c < params.size(); c++)
{
	
	//cout << "f_Rpt" << "\tparam : " << params.at(c) << endl; //
	
	//int idx;
	
	switch(c){ // operand
		case 0:{

			if(type.compare("mixedsync") == 0)
			{
				//cout << params.at(c) << endl;
				//_getch();
				m_Prefix =  "RPT";
				m_rpt_cnt = "";
				vector<string> rpt_cmd;
				regex keywords("([[:d:]]+)[[:s:]]+([[:w:]]*[[:s:]]*){100}");
				smatch match;
				bool found = regex_search(params.at(c), match, keywords);

				// TAZ- Fix for RPT uInstuction with vector staring with 1 or 0 and/or seperated with space(s) 
				//int  start_of_bits = -1;

				if (found) {
				m_rpt_cnt = RemoveChar(match[1].str()," \t");
				}

				string str_all = match.str();
				int vec_start = str_all.find_first_not_of(m_rpt_cnt);
				string str_bits = str_all.substr(vec_start, str_all.size());
				str_bits = RemoveChar(str_bits, " \t");
				vector<string> bits = Tokenize(str_bits,' ');
				
				//cout << "str_bits : " << str_bits << endl;

				for(size_t c = 0; c < bits.size(); c++){	
					final_bits += bits.at(c);
				//	cout << "bits.at(c) : " << bits.at(c) << endl;
				}
		
				//cout << "final_bits : " << final_bits << endl;
			
				if(m_SubVecDef.length() > 0){

					vector<string> pins;
					if (vecdef_str.size() > 0) {
						pins = vecdef_str;
					}
					else {
						pins = Tokenize(m_SubVecDef, ',');
					}
					//vector<string> pins =  Tokenize(m_SubVecDef,',');
					int idx;
					for(size_t d = 0; d < pins.size(); d++){
						if(trim(pins.at(d)).compare("X")==0)
							continue;

						if( FindVecDefIndex(pins.at(d),idx) ){
							if( d < final_bits.length())
								m_VectorLine[idx] = final_bits.at(d);
							else
								LogWarning("mixedsync", "vector width is less than VECDEF width");
						}
						else{
							LogWarning("mixedsync", "pin name " + pins.at(d) + " not in vector defnition");
						}

					}
				}
				else
				{
					for(size_t d = 0; d < final_bits.size(); d++){

						m_VectorLine[d] = final_bits.at(d);
					}
				}
			}
		}break;
	}// switch
} // for

	vector<int> idxs;

	if(m_SubVecDef.length() > 0){
		vector<string> pins;
		if (vecdef_str.size() > 0) {
			pins = vecdef_str;
		}
		else {
			pins = Tokenize(m_SubVecDef, ',');
		}
		//vector<string> pins =  Tokenize(m_SubVecDef,',');
		int idx;
		for(size_t d = 0; d < pins.size(); d++){
			if(trim(pins.at(d)).compare("X")==0)
				continue;

			if( FindVecDefIndex(trim(pins.at(d)),idx) ){

				idxs.push_back(idx);
			}
		}
		for(size_t d = 0; d < m_VectorLine.size(); d++){
			bool add = false;
			for(size_t  e = 0; e < idxs.size(); e++)
				if(idxs.at(e) == d){
					add = true;
					break;
				}

			if(!add)
				 m_VectorLine[d]=G_DEFAULT_PIN_STATE;

		}

	}
	//m_VectorLine = str_cnt + " " + m_VectorLine; 
	m_VectorLine =  m_VectorLine; // Fix for RPT and VEC uinstruction. TAZ 11/11/2017
	str_cnt = str_cnt + " ";
	m_rpt_cnt = m_rpt_cnt + " ";

		Log("m_VectorLine",m_VectorLine);

		//cout << "m_VectorLine : " << m_VectorLine << endl;
		//_getch();

}// f_Rpt()

int Pattern::ExecuteMicroInstReq(string mi){

string s_apg_inst = mi.substr(0,mi.find_first_of(" \t"));
string s_params =  mi.substr(mi.find_first_of(" \t")+1);
vector<string> v_params = Tokenize(s_params,',');
	
ToUpper(s_apg_inst);

int retval;

if( !FindFromTable(APG_INST_table, s_apg_inst, retval)){
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	LogError("ExecuteMicroInst", s_apg_inst + "not found");
	return -1;
}
	
switch(retval){
case YALU:{
	f_YAlu_Req(v_params);
		  }break;
case XALU:{
	f_XAlu_Req(v_params);
		  }break;
case DATGEN:{
	f_DatGen_Req(v_params);
			}break;
case CHIPS:{
	f_Chips_Req(v_params);
		   }break;
case PINFUNC:{
	f_PinFunc_Req(v_params);
			 }break;
case COUNT:{
	f_Count_Req(v_params);
		   }break;
case MAR:{
	f_Mar_Req(v_params);
		 }break;
case UDATA:{
	f_Udata_Req(v_params);
		   }break;
/*default:{
	cout << "unrecognized function." <<endl; //
		} break;*/
}

return 0;
}

int Pattern::ExecuteMicroInst(string mi,string vecdef){

string s_apg_inst = mi.substr(0,mi.find_first_of(" \t"));
string s_params =  mi.substr(mi.find_first_of(" \t")+1);

s_params = RemoveChar(s_params, "()");

//s_params.erase(std::remove(s_params.begin(), s_params.end(), ')'), s_params.end());
//s_params.erase(std::remove(s_params.begin(), s_params.end(), '('), s_params.end());

vector<string> v_params = Tokenize(s_params,',');
	
ToUpper(s_apg_inst);

int retval;

if( !FindFromTable(APG_INST_table, s_apg_inst, retval)){
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
	LogError("ExecuteMicroInst", s_apg_inst + "not found");
	return -1;
}
	
switch(retval){
case YALU:{
	f_YAlu(v_params);
		  }break;
case XALU:{
	f_XAlu(v_params);
		  }break;
case DATGEN:{
	f_DatGen(v_params);
			}break;
case CHIPS:{
	f_Chips(v_params);
		   }break;
case PINFUNC:{
	f_PinFunc(v_params);
			 }break;
case COUNT:{
	f_Count(v_params);
		   }break;
case MAR:{
	f_Mar(v_params);
		 }break;
case UDATA:{
	f_Udata(v_params);
		   }break;
case VEC:{

	f_Vec(v_params,vecdef);
		   }break;
case RPT:{
	f_Rpt(v_params,vecdef);
		   }break;
/*default:{
	cout << "unrecognized function." <<endl; //
		} break;*/
}

return 0;
}


string Pattern::UndefineMicroInst(string mi){
//extract parameters
string s_apg_inst = mi.substr(0,mi.find_first_of(" \t"));
string s_params =  mi.substr(mi.find_first_of(" \t")+1);
vector<string> v_params = Tokenize(s_params,',');

for(size_t c = 0; c < v_params.size(); c++){
	string param = v_params.at(c); 

	if( FindDef(param) )
		v_params.at(c).assign(param);
	else{
	//	printl(	"param not defined :", param);
		debugfile <<"UndefineMicroInst: " << "param not defined -> " << param << endl;

	}

}

// re-compose instruction

s_apg_inst += " ";
for(size_t c = 0; c < v_params.size(); c++){
	s_apg_inst.append(v_params.at(c));
	if(c < v_params.size()-1)
		s_apg_inst.append(",");

}
debugfile <<"Undef'd : " << s_apg_inst << endl;

return s_apg_inst;

}

int Pattern::UndefineSubVecDef(string &vecdef){

	vector<string> strings =  Tokenize(vecdef,' ');

	vecdef = "";


	for(size_t c = 0; c < strings.size(); c++){
		string str = strings.at(c);
		if(trim(str).length() > 0)
		if( FindDef(str) )
			vecdef += str;
		else{
			vecdef += trim(strings.at(c));
			}

	}

	int idx =  vecdef.find("VECDEF");
	if(idx != -1)
		vecdef =  vecdef.substr(idx+6);

	strings =  Tokenize(vecdef,',');

	vecdef="";

	for(size_t c = 0; c < strings.size(); c++){
		string str = trim(strings.at(c));
		if(trim(str).length() > 0){
			vecdef += str;
			if(c < strings.size()-1 )
				vecdef += ",";

		}

	}

	setSubVecDef(vecdef);
	m_isSubVecDefDefined = true;

	return 0;

}

int Pattern::UndefinePatInst(PatInst &inst){

int j = 0;    

for (size_t  j = 0; j < inst.m_listMicroInst->size(); j++){  // for each micro instruction
			string mi = inst.m_listMicroInst->at(j);
			int pos = mi.find(' ');
			if(pos == -1){ // no spaces, try if its a #define
				map<string,string>::iterator iter = stdmapDEFINE.find( mi );
				if( iter != stdmapDEFINE.end() ){ // stdmapDEFINE.end() found
					string define = stdmapDEFINE[ mi ];
					debugfile << "\n" << mi << " == " << define<< endl;
					
					if(define.find("\\n") != -1){
						vector<string> lines = Tokenize(define,'\\');
						for(size_t  i = 0; i < lines.size(); i++){
							string line = lines.at(i); 
							line = ltrim(rtrim(line));
							if(i > 0)
								line = line.substr(2);

							if(i == 0)
								inst.m_listMicroInst->at(j).assign(line);
							else
								inst.m_listMicroInst->insert( inst.m_listMicroInst->begin()+j+1 ,line);
						}

					}
					else
						inst.m_listMicroInst->at(j).assign(define);
					 
				}
				
			}
}


for (size_t  i = 0; i < inst.m_listMicroInst->size() ; i++){
	Log("UndefinePatInst", inst.m_listMicroInst->at(i) );
	//_getch();
	string undef =  UndefineMicroInst( inst.m_listMicroInst->at(i) );
	undef = UndefineMicroInst( undef);
	inst.m_listMicroInst->at(i).assign( undef );
	Log("UndefinePatInst", undef );
	//_getch();

	}

return 0;
}

int Pattern::JumpToGosubFix(PatInst &inst) {

	int j = 0;
	map <string, string> tempMap;
	map <string, map <string, string>> tempMap2;
	map <string, string> tempMap3;

	tempMap = jump2gosub;
	int pos;
	int pos2;
	string name = currentPattern;
	regex JumpGosub_keywords("^[[:s:]]*%*mar[[:s:]]*jump[[:s:]]*,[[:s:]]*([[:w:]]+)", std::regex_constants::icase);
	smatch match9;

	for (size_t j = 0; j < inst.m_listMicroInst->size(); j++) {  // for each micro instruction

		string mi = inst.m_listMicroInst->at(j);
		pos = mi.find("jump,");
		if (pos != -1) { // no spaces, try if its a #define
			bool foundJump = regex_search(mi, match9, JumpGosub_keywords);
			string tempMatch = match9[1];

			for (map<string, map <string, string>>::iterator iter = pat_JumpLabel.begin(); iter != pat_JumpLabel.end(); ++iter) {
					//cout << iter->first << " = " ;
					tempMap3 = iter->second;

					for (map<string, string>::iterator iter2 = tempMap3.begin(); iter2 != tempMap3.end(); ++iter2) {
						pos2 = tempMatch.find(iter2->first);
						if (pos2 != -1) {
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
							cout  << "  Changing jump to gosub for pattern " << iter->first  << " and label " << iter2->first << endl;
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
							mi.replace(pos, 4, "gosub");
							inst.m_listMicroInst->at(j).assign(mi);
					}
				}
			}
		}
	}
	return 0;
}

int Pattern::CheckFutureJump2(int cur_idx,vector<string> &params){
int ret_val = 0;
int jmp_cond = -1;
string jmp_lbl;
for(size_t c = 0; c < params.size(); c++)
{
	//cout << "CheckFutureJump2\tparam : " << params.at(c) << endl;
	//_getch();
	int idx;
	switch(c){ // operand
	case 0:{ // 
		
		if( FindFromTable( MAR_OP_1_table, params.at(c), idx) ){
			debugfile << "MAR_OP_1:" << MAR_OP_1_table[idx] << endl;
			jmp_cond = idx;
		}
		   } break;
	case 1:{
		if( FindFromTable( MAR_OP_2_table, params.at(c), idx) ){
			debugfile << "MAR_OP_2:" << MAR_OP_1_table[idx] << endl;
		}else
			if(params.at(c).size() > 0){
				jmp_lbl = ltrim(rtrim(params.at(c)));
			
			}
		   } break;
	case 2:{
		if( FindFromTable( MAR_OP_3_table, params.at(c), idx) ){
			debugfile << "MAR_OP_3:" << MAR_OP_1_table[idx] << endl;

			switch(idx){
			case READ:{
			//m_isRead = true;
			///	getCurPatInst()->isRead = true;
					  }break;
			}
		}
		   } break;


	} // switch
} //for

if(jmp_cond == -1)
	return ret_val;

int tgt_idx;
switch( jmp_cond ){
case RETURN:{} break;	
case DONE:{} break;
	case GOSUB:{}break;
	case CJMPE:{}
	case CJMPNE:{}
	case CJMPZ:{
	/*	Log("CJMPZ",jmp_lbl);
		addTailTag(m_CurIndex,"mar jump, " + jmp_lbl);
		if(FindLabelIndex(jmp_lbl, idx)){
			addHeadTag(idx, "// " + jmp_lbl + ":");
		}
	*/
	}
	case JUMP:{
	}break;

	case CJMPNZ:{
		if(FindLabelIndex(jmp_lbl, tgt_idx)){

			if(tgt_idx <= cur_idx){
			ret_val++;
			//cout <<"CJMPNZ : " <<jmp_lbl<<" tgt_idx:" <<tgt_idx<<" cur_idx:" << cur_idx<< endl;
			//_getch();

			//Set the insInloop of PatInst range to true

				for(int c = tgt_idx; c <= cur_idx; c++)
					m_listPatInst->at(c).isInLoop = true;


				CheckFutureAddrChange(tgt_idx, cur_idx);

				}

				
				

				

		} // if FindLabelIndex
		}break;
default:{}break;

} //switch

return ret_val;
}

int Pattern::CheckFutureJump(int cur_idx){

int ret_val = 0;

for(size_t  i = cur_idx; i < m_listPatInst->size(); i++){

	for (size_t  j = 0; j< m_listPatInst->at(i).m_listMicroInst->size(); j++){  // for each micro instruction
		
		string mi = m_listPatInst->at(i).m_listMicroInst->at(j);
		string func = mi;
		ToUpper(func);

		if(func.find("MAR") == 0){
			
			string s_apg_inst = mi.substr(0,mi.find_first_of(" \t")); // MAR
			string s_params =  mi.substr(mi.find_first_of(" \t")+1); 
			vector<string> v_params = Tokenize(s_params,',');
			ret_val += CheckFutureJump2(i,v_params); // CheckFutureJump2 sets the isInloop flags for the rnage of PatInst within the loop
			
		}
	}
}  // for( int i
	return ret_val;
}

bool Pattern::CheckFutureAddrChange2(string func, vector<string> &params){

	if(func == "XALU"){
		Log("CheckFutureAddrChanged2","called " + func );
		//_getch();
		for(size_t c = 0; c < params.size(); c++){


			string p = params.at(c);
			int idx;
			Log("CheckFutureAddrChanged2","param - " + p );
			if( FindFromTable( ADD_GEN_OPS_table, p, idx) ){ // do not convert to upper when searching
			 
				switch(idx){
				case HOLD:{
					Log("CheckFutureAddrChanged2","isHold " );
					//_getch();
				 return false;
				}break;
				
				}
			}//if


		}// for
	Log("CheckFutureAddrChanged2","XAdd changed");
	//_getch();
	return true;
	
	}
	else if(func == "YALU"){

		Log("CheckFutureAddrChanged","called " + func );
		//_getch();
	
		for(size_t c = 0; c < params.size(); c++){

		
		string p = params.at(c);

		int idx;
		if( FindFromTable( ADD_GEN_OPS_table, p, idx) ){ // do not convert to upper when searching
		 
			switch(idx){
			case HOLD:{
				return false;
			}break;
			}// switch

		}//if

	}// for
	
	Log("CheckFutureAddrChanged2","YAdd changed");
	//_getch();
	return true;

	}else
		return false;

}

int Pattern::CheckFutureAddrChange(int start_idx,int end_idx){

	Log("CheckFutureAddrChange" , "start= " + toString(start_idx) + " end=" + toString(end_idx) );
	//_getch();
int ret_val = 0;
	bool xaddr_changed = false;
	bool yaddr_changed = false;

for(int i = start_idx; i <= end_idx; i++){

	for (size_t  j = 0; j< m_listPatInst->at(i).m_listMicroInst->size(); j++){  // for each micro instruction
		
		string mi = m_listPatInst->at(i).m_listMicroInst->at(j);
		string func = mi;
		ToUpper(func);
		
		if(!xaddr_changed)
		if(func.find("XALU") == 0 ){
		Log("CheckFutureAddrChanged","called " + func + " , " + toString(i) + " , " + getName() );
		//_getch();
		
			string s_apg_inst = mi.substr(0,mi.find_first_of(" \t")); // MAR
			string s_params =  mi.substr(mi.find_first_of(" \t")+1); 
			vector<string> v_params = Tokenize(s_params,',');
			xaddr_changed =  CheckFutureAddrChange2("XALU",v_params);
			
		}

		if(!yaddr_changed)
		if(func.find("YALU") == 0 ){
		
		Log("CheckFutureAddrChanged","called " + func + " , " + toString(i) + " , " + getName() );
		//_getch();			
			string s_apg_inst = mi.substr(0,mi.find_first_of(" \t")); // MAR
			string s_params =  mi.substr(mi.find_first_of(" \t")+1); 
			vector<string> v_params = Tokenize(s_params,',');
			yaddr_changed = CheckFutureAddrChange2("YALU",v_params);
			
		}

	}
}  // for( int i

if(xaddr_changed){
	for(int i = start_idx; i <= end_idx; i++){
		m_listPatInst->at(i).isXAddrChanged = true;
	Log("xaddr_changed",getName() + " : " + toString(i));
	}

Log("xaddr_changed", "true");
//_getch();
}
if(yaddr_changed){
	for(int i = start_idx; i <= end_idx; i++){
	m_listPatInst->at(i).isYAddrChanged = true;

	Log("yaddr_changed",getName() + " : " + toString(i));
	}
Log("yaddr_changed", "true");
//_getch();

}

return ret_val;
}

int Pattern::ExecutePatInst(int idx){

m_CurIndex = idx;


PatInst *inst= &m_listPatInst->at(idx);
//inst->m_listHeadTags->clear();
//inst->m_listVecConverted->clear();
//inst->m_listTailTags->clear();

//m_VectorLine = "";
m_VectorLine.resize(G_VEC_LINE_SIZE,G_DEFAULT_PIN_STATE);
SetBitDefaults(m_VectorLine);

//if (inst->getVecDef().length() > 0)
//cout << "Current VecDef:" << inst->getVecDef() << "\n";
//cout << "ExecutePatInst : " << m_VectorLine << endl;

m_Prefix = "";
m_GoSubLabel = "";
m_JumpLabel = "";
G_PATINST_IDX = inst->index;
G_RPT_OPT = "";
m_TsetSel.assign("TSET1");
m_PsMapSel.assign("PS1");
m_VihhSel.assign("VIHH1");
G_PATINST_SEL = inst->index;

if( !inst->getLabel().empty() ){
	if( FindInCjmpzMap(inst->getLabel()) ){

		Log("FindInCjmpzMap",inst->getLabel());
		for(size_t c = 0; c < 61; c++){
			long v;
			GetCtrVal(inst->getLabel(),c,v);
			G_COUNTERS[c] = v;
		}
		//GetCtrVal(inst->getLabel(),m_CounterSel,SelCounterVal);
		PopCjmpz(inst->getLabel());
	
	}
}
	
//if(getCurPatInst()->label.length() > 1){
	if(!inst->isInLoop)
	if( CheckFutureJump(inst->index) > 0){
		getCurPatInst()->isInLoop = true; // getCurPatInst()-> == inst->
		Log("ExecutePatInst", inst->index + ":" + getCurPatInst()->label + " <- isInLoop ");
//		_getch();
	}


//}

debugfile << "<START> =================================================================" << endl;



// Re-arrange the micro-ints for proper sequence of execution

for (size_t  j = 0; j< inst->m_listMicroInst->size(); j++){  // for each micro instruction
	string mi = inst->m_listMicroInst->at(j);
	ToUpper(mi);
	debugfile <<"mi : "<< mi <<endl;
	
	if(mi.find("YALU") == 0)
		inst->v_mi->at(s_yalu) = inst->m_listMicroInst->at(j);
	
	else if(mi.find("XALU") == 0)
		inst->v_mi->at(s_xalu) = inst->m_listMicroInst->at(j);
	
	else if(mi.find("DATGEN") == 0)
		inst->v_mi->at(s_datgen) = inst->m_listMicroInst->at(j);
	
	else if(mi.find("CHIPS") == 0)
		inst->v_mi->at(s_chips) = inst->m_listMicroInst->at(j);
	
	else if(mi.find("PINFUNC") == 0)
		inst->v_mi->at(s_pinfunc) = inst->m_listMicroInst->at(j);
	
	else if(mi.find("COUNT") == 0)
		inst->v_mi->at(s_count) = inst->m_listMicroInst->at(j);
	
	else if(mi.find("MAR") == 0)
		inst->v_mi->at(s_mar) = inst->m_listMicroInst->at(j);
	
	else if(mi.find("UDATA") == 0)
		inst->v_mi->at(s_udata) = inst->m_listMicroInst->at(j);
	
	else if(mi.find("VEC") == 0)
		inst->v_mi->at(s_vec) = inst->m_listMicroInst->at(j);
	
	else if(mi.find("RPT") == 0)
		inst->v_mi->at(s_rpt) = inst->m_listMicroInst->at(j);
}
string vd = inst->m_subvecdef;
ToUpper(vd);
string s_vd = vd.substr(0, vd.find_first_of(" "));

if (vd.find("VECDEF") == 0) {
	vd = s_vd;
	}
m_isCount = false;
for (size_t   j = 0; j < inst->v_mi->size(); j++){  // for each micro instruction

	if( inst->v_mi->at(j).size() > 0){

		
		debugfile << "ExecuteMicroInst : " << inst->v_mi->at(j) <<endl ;///inst.m_listMicroInst->at(j)<< endl;
		if( G_MODE == MODE1)
			ExecuteMicroInst(inst->v_mi->at(j), vd);
		else// (G_MODE == MODE2)
			ExecuteMicroInstReq(inst->v_mi->at(j));

	}
}

if(G_MODE == MODE2)
	return idx +1;

string rpt_str;
string vec_line;

long SelCounterVal =  G_COUNTERS[m_CounterSel];

vec_line +="\n%";

if(inst->getLabel().length() > 0)
	 vec_line += inst->getLabel() +":\n";
	

if(( m_Prefix.length() > 0) && (G_RPT_OPT.length() == 0)) // TAZ Fix for vec with cjmpnz uinstr. 
{
	vec_line += m_Prefix + " ";
}
else
{
	if(G_RPT_OPT.length() > 1) 
	rpt_str =  G_RPT_OPT + " " + toString( SelCounterVal + 1) + " ";
	else
	rpt_str = "";

	if(rpt_str.length() > 1)
		vec_line += rpt_str +" ";
	else
		vec_line += "VEC ";

}


vec_line +=  m_rpt_cnt + m_VectorLine + " ," +  m_TsetSel + ",PS1," + m_VihhSel + (inst->isVPULSE ? ",VPULSE":"") + "\t// "  \
+ toString( inst->getIndex()) + " PS Map:" + m_PsMapSel + " Counter" + toString(m_CounterSel + 1) + ":";

if(G_DEBUG_ON ){
	vec_line += " = " + toString( SelCounterVal ) + " patname: " + getName();
}
	


inst->m_listVecConverted->push_back( vec_line  ); // capture a snapshot of the vector line

if((index > 0) && G_DEBUG_ON){
	Log("vec_line" , vec_line );
}

	if(G_ADD_ORIG)
		inst->copyOrigLines(inst->m_listTailTags);

	Log("ExecutePatInst", "inst->m_listVecConverted->back() = " + inst->m_listVecConverted->back());

//	if(G_DEBUG_ON)
//		_getch();


	int retval;	
	
	if(m_GoSubLabel.length() > 1){
		Log("ExecutePatInst","m_GoSubLabel : " +  m_GoSubLabel + "\r\n");

		inst->m_listVecConverted->push_back("// GOSUB to: " +  m_GoSubLabel + " from: " + name);
		GoSub(getIndex(), inst->index, m_GoSubLabel);
		retval =  idx + 1;
	}
	else{
	/*
		if(m_JumpLabel.length() > 1){
			cout<<"m_JumpLabel : " << m_JumpLabel << endl; //

			int jmp_idx;
			if(FindLabelIndex(m_JumpLabel,jmp_idx))
				retval =  jmp_idx;
			else{
				
				cout << "error: jump to unknown label : " << m_JumpLabel; //
				retval =  -1;
			}
		}
		else{
			cout << "idx + 1 : " << idx+1<< endl; //
	*/
			retval =  idx + 1;

		
	} //if m_GoSubLabel

	
	return retval;
//return -1;
}

void Pattern::Reset(){
	m_MarDone = false;
	m_CurIndex = 0;
	ClearVecLines();
}

int Pattern::ProcPatInst(){

bool done = false;

UpdateLstIndexes();

int NextIndex = m_CurIndex;
while (!done){

	NextIndex = ExecutePatInst(NextIndex);
	 Log("NextIndex" , toString(NextIndex));
	 Log("m_CurIndex" , toString(m_CurIndex));
	 
	if(m_MarDone || NextIndex==m_listPatInst->size() ||  NextIndex == -1)
		done = true;


}

return 0;
}

bool Pattern::getGlobalInit(string &kv){
	map<string,string>::iterator it = m_GlobalInitMap.find(kv);
	if(it != m_GlobalInitMap.end()){
		
		kv = it->second;
				Log("getGlobalInit", kv);
				if(G_DEBUG_ON)
					_getch();
		return true;
	}
	else
		return false;    
}

bool SetIntReg(unsigned int &tgt_reg, string &reg_name, string &str_val){

	unsigned long val;
			if( StrTol( str_val ,val) ){
				
				tgt_reg = val;
				Log("LoadPatInits - " + reg_name + " to ",toString(val));
				if(G_DEBUG_ON )
					_getch();
				return true;
			}
			else{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
				LogError("LoadPatInits - load " + reg_name,"failed to convert " + str_val +" to long.");
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
				if(G_HALT_E)
					_getch();
				return false;
			}
}

bool SetReg(unsigned long &tgt_reg, string &reg_name, string &str_val){

	unsigned long val;
			if( StrTol( str_val ,val) ){
				
				tgt_reg = val;
				Log("LoadPatInits - " + reg_name + " to ",toString(val));
				if(G_DEBUG_ON )
					_getch();
				return true;
			}
			else{
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
				LogError("LoadPatInits - load " + reg_name,"failed to convert " + str_val +" to long.");
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
				if(G_HALT_E)
					_getch();
				return false;
			}
}

void Pattern::LoadPatInits(string  patname ){

  //vTriStatePins.clear();

map<string,string>::iterator iter = stdmapPATINITS.find( patname );
if( iter == stdmapPATINITS.end() ){ // if found
	Log("LoadPatInits", patname + " not found." );
	//_getch();
	return;
}
	//Log("LoadPatInits",getName() + " = "+iter->first + " found.");
	//_getch();
	string str = iter->second;
	
	
	Log("LoadPatInits: iter->second;", iter->second );
	if(G_DEBUG_ON)
		_getch();

	if(str.length() <= 1){

		return;
	}

	vector<string> regs = Tokenize(str,'|');

	for(size_t c = 0; c < regs.size(); c++){

		if(regs.at(c).length() <= 1)
			continue;

		vector<string> pair = Tokenize(regs.at(c),'=');
		string reg = trim(pair.at(0));

		string str_val;

		if(pair.size() < 2){
			Log("ParsePatInits : ", reg + " not set. will try to load from global init");

			if(G_HALT_W)
				_getch();
			string tmp = reg;
			if(getGlobalInit(tmp)){
				str_val = tmp;

				Log("ParsePatInits : ", reg + " will default to " + str_val);

				if(G_HALT_W)
					_getch();

			}
			else{

				LogWarning("ParsePatInits : ", reg + " is not set.");
				if(G_HALT_W)
				_getch();
				continue;
			}
		} // if pair.size() >= 2
		else 
			str_val = trim(pair.at(1));

		if(reg.find("count") == 0){
				int ctr = atoi( reg.substr(5).c_str() );
				if(ctr > 0 && ctr <= 61){
					string tmp;
					unsigned long val;
					if( StrTol( str_val ,val) ){
						G_COUNTERS[ctr-1] = val;
						G_COUNTERS_RLD[ctr-1]=val;
						Log("LoadPatInits - Set counter " + toString(ctr)," = "+ toString(val));
						Log("count" +toString(ctr),toString(G_COUNTERS[ctr-1]) );
						if(patname == "_global_")
							m_GlobalInitMap[reg] = str_val;
					}
					else{
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
						LogError("LoadPatInits - load counter","failed to convert " + trim(pair.at(1)) +" to long.");
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
						if(G_HALT_E)
							_getch();
					}
				}
				else{
					    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
						LogError("LoadPatInits", "Invalid counter number : " + toString(ctr));
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
						if(G_HALT_E)
							_getch();
				}
		}
		else if(reg.compare("tri_state")== 0){
			vector<string> cs = Tokenize( str_val,',');
			//cout << "reg == tri_state. str_val = " << str_val << endl;
			vTriStatePins.clear();
			for(size_t c = 0; c < cs.size(); c++){
				Log("LoadPatInits - tristate pin: " , cs.at(c));
				vTriStatePins.push_back(cs.at(c));
			}
		}
		else if(reg.compare("adhiz")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			if(str_val.compare("DRIVE")==0)
				G_ADHIZ = DRIVE;
			else if(str_val.compare("RECEIVE")==0)
				G_ADHIZ = RECEIVE;
		}
		else if(reg.compare("numx")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			SetReg(G_NUMX,reg, str_val );
		}
		else if(reg.compare("numy")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_NUMY, reg, str_val );
		}
		else if(reg.compare("xmain")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_XMAIN,reg, str_val );
		}
		else if(reg.compare("xbase")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_XBASE,reg, str_val );
		}
		else if(reg.compare("xfield")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_XFIELD,reg, str_val );
		}
		else if(reg.compare("ymain")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_YMAIN, reg, str_val );
		}
		else if(reg.compare("ybase")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_YBASE, reg, str_val );
		}
		else if(reg.compare("yfield")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_YFIELD, reg, str_val );
		}
		else if(reg.compare("datreg")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_DATREG, reg, str_val );
		}
		else if(reg.compare("jamreg")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_JAMREG, reg, str_val );
		}
		else if (reg.compare("dbm") == 0) {
			if (patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
			else
				SetReg(G_DBM, reg, str_val);
		}
		else if(reg.compare("cs_active_high")== 0){
			if(patname == "_global_")
				m_GlobalInitMap[reg] = str_val;
		
			vector<string> cs = Tokenize( str_val,',');
			for(size_t c = 0; c < 8; c++){ // set all to false
					G_CS_ACTIVE_H[c]=false;
			}
			for(c = 0; c < cs.size(); c++){
				string t_cs = cs.at(c);
				ToUpper(t_cs);
				if(t_cs.find("T_CS") != 1){
					int cs_num = atoi( t_cs.substr(4).c_str() );
					if(cs_num > 0 && cs_num <= 8){
						G_CS_ACTIVE_H[cs_num-1] = true;
					//	Log("LoadPatInits", "Set to true : t_cs" + toString(cs_num));
					//	_getch();
					}
					else{
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
						LogError("LoadPatInits", "Invalid t_cs number : " + toString(cs_num));
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
						if(G_HALT_E)
							_getch();
					}

				}
				else{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
					LogError("LoadPatInits", "Invalid tester function : " + t_cs);
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
					if(G_HALT_E)
						_getch();
				}
			}
		}
		else
		{
		 SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
		 LogError("LoadPatInits", reg + " - unknown register or function.");
		 SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
		 if(G_HALT_E)
			_getch();
		}

	}

}
int Pattern::Convert(string dirout){
	if (dirout.size() == 0) {
		dirout = "output_files\\";
	}

//if(type.compare("logic") == 0)
//	return 0;

if(G_MODE ==  MODE1){
	LoadPatInits(getName());
}

G_PAT_IDX = index;

int i = 0;

if((getSubVecDef().length() >  0) && !m_isSubVecDefDefined)
	UndefineSubVecDef(this->getSubVecDef());
for (  vector<PatInst>::iterator it_pat = m_listPatInst->begin(); it_pat != m_listPatInst->end(); ++it_pat,++i){
	
	UndefinePatInst(*it_pat);	
	JumpToGosubFix(*it_pat); 
}

string filename(dirout + getName());
filename += "_undef.pat";

ofstream ofile(filename.c_str());

printdata(ofile);



UpdateLstIndexes();

ProcPatInst();

CommitVecLines();

//vTriStatePins.clear();

return 0;

}

void Pattern::addPatternReq(string req_reg, string comment){
	bool found = false;
	string str;
	for(size_t c = 0; (c < m_listReqReg->size()); c++ ){
		str = m_listReqReg->at(c);
		if(str.size() <= 1)
			continue;

		str = Tokenize(str,' ')[0];
		str = ltrim(rtrim(str));
		if( req_reg.compare(str) == 0 ){
			found = true;
			break;
		}
	}

	if(!found){
		string str  = trim(req_reg);
		if(comment.size() > 0)
			str += " //" + comment;
		Log(name + " : addPatternReq", str);
		m_listReqReg->push_back(str);

	}

}




void Pattern::f_YAlu_Req(vector<string> &params){

	int idx;
	for(size_t c = 0; c < params.size(); c++)
	{
		#ifdef _debug_on_
		Log("f_f_YAlu", "\tparam : " + params.at(c) ); //
		#endif
		string p = params.at(c);
		if(c <= 1)
			if( FindFromTable( YALU_OP_1_2_table, p, idx) ){ // do not convert to upper when searching 
				switch(idx){
				case YMAIN:{};
				case YBASE:{};
				case YFIELD:{addPatternReq(p);}break;
				case YUDATA:{addPatternReq("udata",p);}break;
				default:{} break;
				}
			}
	}

}
#define _debug_on_
void Pattern::f_XAlu_Req(vector<string> &params){
	//trap
	int idx;
	for(size_t c = 0; c < params.size(); c++)
	{
		#ifdef _debug_on_
		Log("f_XAlu", "\tparam : " +  params.at(c) ); //
		#endif
		string p = params.at(c);

		if(c <= 1)
			if( FindFromTable( XALU_OP_1_2_table, p, idx) ){ // do not convert to upper when searching
				switch(idx){
				case XMAIN:{};
				case XBASE:{};
				case XFIELD:{addPatternReq(p);}break;
				case XUDATA:{addPatternReq("udata",p);}break;
				default:{} break;
				}
			}
	}
}
void Pattern::f_DatGen_Req(vector<string> &params){
	int idx;
	for(size_t c = 0; c < params.size(); c++)
	{
		#ifdef _debug_on_
		Log("f_DatGen", "\tparam : " + params.at(c) ); 
		#endif
		string p = params.at(c);

		if( FindFromTable( DATGEN_OPS_table, p, idx) ){ // do not convert to upper when searching 
			switch(idx){
				case SDMAIN:{addPatternReq("dmain",p);}break;
				case SDMAIN2:{addPatternReq("dmain2",p);}break;
				case SDBASE:{addPatternReq("dbase"),p;} break;
				case SDBASE2:{addPatternReq("dbase2",p);}break;
				case SUDATA:{addPatternReq("udata"),p;}break;
				case JAMJAM:{addPatternReq("jamreg",p);}break;
				case DATDAT:{addPatternReq("datreg",p);}break;
				//case DATBUF:{addPatternReq("datreg",p);}break;
				default:{} break;
			}
		}
	}
}

void Pattern::f_Chips_Req(vector<string> &params){}
void Pattern::f_PinFunc_Req(vector<string> &params){}
void Pattern::f_Count_Req(vector<string> &params){}
void Pattern::f_Mar_Req(vector<string> &params){}
void Pattern::f_Udata_Req(vector<string> &params){}

