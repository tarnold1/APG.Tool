// PatInst.h: interface for the PatInst class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATINST_H__8C222DD7_C1A7_48A2_A87B_A6741B8F6453__INCLUDED_)
#define AFX_PATINST_H__8C222DD7_C1A7_48A2_A87B_A6741B8F6453__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "common.h"

class PatInst  
{

public:
	public:
	bool isRead;
	bool isReadUdata;
	bool isADHIZ;
	bool isVPULSE;
	bool isVTSET;
	bool isInLoop;
	bool isHoldYalu;
	bool isHoldXalu;
	bool isXAddrChanged;
	bool isYAddrChanged;
	int index;
	string label;
	vector<string> *v_mi;

	vector<string> *m_listMicroInst;
	vector<string> *m_listVecConverted;
	vector<string> *m_listHeadTags;
	vector<string> *m_listTailTags;
	vector<string> *m_listOrigLines;
	string m_subvecdef;
PatInst();

virtual ~PatInst();

void setIndex (int idx);
void setLabel (string lbl);
void setVecDef(string vecdef);
int getIndex ();
string getLabel ();
string getVecDef();
void addMicroInst(string mi);
void setOrigLines(string lines);
void PatInst::clearVecLines();
void copyOrigLines(vector<string> *vec_lines);
void printdata(ofstream &outfile);
//void setSubVec(string sbvcdf);
//string getSubVec();
};
	
	
#endif // !defined(AFX_PATINST_H__8C222DD7_C1A7_48A2_A87B_A6741B8F6453__INCLUDED_)
