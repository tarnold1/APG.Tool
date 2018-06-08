// PatInst.cpp: implementation of the PatInst class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PatInst.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



PatInst::PatInst(){
	isHoldYalu =  false;
	isHoldXalu =  false;
	isXAddrChanged =  false;
	isYAddrChanged =  false;
	isInLoop = false;
	isRead = false;
	isReadUdata = false;
	isADHIZ = false;
	isVPULSE =  false;
	m_listMicroInst = new vector<string>;
	m_listVecConverted =  new vector<string>;
	m_listHeadTags =  new vector<string>;
	m_listTailTags =  new vector<string>;
	m_listOrigLines =  new vector<string>;
	m_subvecdef = "";
	v_mi = new vector<string>(10,"");
	v_mi->at(s_datgen).assign("DATGEN SDMAIN, SDMAIN, DMAIN, HOLDDR, HOLDYN, EQFDIS,BCKFDIS, NOTINV, DATDAT, JAMRAMHOLD");
	v_mi->at(s_yalu).assign("YALU XCARE, XCARE, COFF, HOLD, NODEST, OYMAIN");
	v_mi->at(s_xalu).assign("XALU XCARE, XCARE, COFF, HOLD, NODEST, OXMAIN");
	v_mi->at(s_vec).assign("VEC ");
	v_mi->at(s_rpt).assign("RPT ");

}

PatInst::~PatInst(){}

void PatInst::setIndex (int idx){
	index = idx;
}
void PatInst::setVecDef(string vecdef) {
	m_subvecdef = vecdef;
}
void PatInst::setLabel (string lbl){
	label = lbl;
}

int PatInst::getIndex (){
	return index;
}
string PatInst::getLabel (){
	return label;
}
string PatInst::getVecDef() {
	return m_subvecdef;
}
void PatInst::addMicroInst(string mi){
	
	m_listMicroInst->push_back(mi);
}

void PatInst::setOrigLines(string lines){
	vector<string> v_lines = Tokenize(lines,'\n');
	for(size_t  c = 0; c < v_lines.size(); c++){
		string cm = trim(v_lines.at(c));
		//remove_ch(cm,'\r');
		

		if(cm.find(":")!= -1)
			continue;

		int idx = cm.find_last_of(" \r");
		if(idx==0 && cm.length()==1){
			continue;	
		}
		idx = cm.find_last_of("\r");
		if(idx == (cm.length()-1) ){
			cm = cm.substr(0,idx-1);	
		}


		if(cm.length() > 1)
			if(cm.find("//",0)!= string::npos){
				if( cm.find("//",0) < cm.find_first_not_of(" /") )
					m_listOrigLines->push_back(cm);
				else
					m_listOrigLines->push_back("// " + cm);
			}
			else{
				m_listOrigLines->push_back("// " + cm);
			}
		//m_listOrigLines->push_back("//" + v_lines.at(c));
	}
}
void PatInst::clearVecLines(){
	m_listVecConverted->clear();
	m_listHeadTags->clear();
	m_listTailTags->clear();
	m_listVecConverted =  new vector<string>;
	m_listHeadTags =  new vector<string>;
	m_listTailTags =  new vector<string>;
	
}

void PatInst::copyOrigLines(vector<string> *vec_lines){
	for(size_t  c = 0; c < m_listOrigLines->size(); c++){
		vec_lines->push_back(m_listOrigLines->at(c));
	}

}
void PatInst::printdata(ofstream &outfile){
	
	outfile <<  index << " : " << label << '\n';
	outfile <<  "size : " << m_listMicroInst->size() << '\n';
	for (size_t  i = 0;i < m_listMicroInst->size() ; i++){
		 
	 outfile << i << ":" << m_listMicroInst->at(i) << '\n';
	
	}

}

//void PatInst::setSubVec(string sbvcdf) {
//	m_subvecdef = sbvcdf;
//}

//string PatInst::getSubVec() {
//	return m_subvecdef;
//}
