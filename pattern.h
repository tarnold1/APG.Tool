// Pattern.h: interface for the Pattern class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATTERN_H__07FD771B_4250_4D2D_A854_9F4FD061CB6E__INCLUDED_)
#define AFX_PATTERN_H__07FD771B_4250_4D2D_A854_9F4FD061CB6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "patinst.h"

#include "common.h"


class Pattern  
{

private:
	int m_NestLevel;
	string m_TriStatePin;
	string m_VectorLine;
	string m_rpt_cnt;
	string m_GoSubLabel;
	string	m_JumpLabel;
	int  m_CurIndex;
	bool m_MarDone;
	bool m_isRead;
	bool m_isSubVecDefDefined;
	string m_TsetSel;
	string m_PsMapSel;
	string m_VihhSel;
	string m_Prefix;
	int m_CounterSel;
	map< string,vector<long> > m_CjmpzMap;
	map< string, string> m_GlobalInitMap;
	bool m_isCount;
	string m_currentPat;


public:
	string m_SubVecDef;
	bool m_isSubPat;
	int index;
	string name;
	string orig_name;
	string type;
	vector<string> m_listTriStatePins;
	vector<string> *m_listVecConv;
	vector<PatInst> *m_listPatInst;
	vector<string> *m_listReqReg;
	vector<string> *m_listComments;
	
	vector<string> *m_listDriveOnlyPins;
		
	Pattern();
	~Pattern();
	bool isInLoop();
	void SnapCjmpz(string lbl_name);
	void PopCjmpz(string lbl_name);
	bool FindInCjmpzMap(string lbl_name);
	bool GetCtrVal(string lbl_name, int counter,long &val);
	
	
void ClearVecLines();
void CommitVecLines();
unsigned long InvertData(long dat_in);

	void setIndex (int idx);
	void setName (string nm);
	void SetTriStatePins();
	int getIndex ();
	string getName ();
	void addPatInst (PatInst inst);
	int getListSize ();
	void setSubVecDef(string vecdef);
	string getSubVecDef();
	void printdata(ofstream &outfile);
	void UpdateLstIndexes();

	void addHeadTag(int patinst_idx, string tag);
	
	void addTailTag(int patinst_idx, string tag);


	void InsertStartLoopTag(int patinst_idx, int counter_val);
	

	void InsertEndLoopTag(int patinst_idx, string comment);


	bool FindLabelIndex(string jmp_lbl,int &idx);

	PatInst* getCurPatInst();
	string RemoveChar(string string_in, string remove_char);
void SetVecLineBits(vector<int> &idxs, char ch);
void f_YAlu(vector<string> &params);
void f_XAlu(vector<string> &params);
void f_DatGen(vector<string> &params);
void f_Chips(vector<string> &params);
void f_PinFunc(vector<string> &params);
void f_Count(vector<string> &params);
void f_Mar(vector<string> &params);
void f_Udata(vector<string> &params);
void f_Vec(vector<string> &params,string vecdef);
void f_Rpt(vector<string> &params,string vecdef);

void f_YAlu_Req(vector<string> &params);
void f_XAlu_Req(vector<string> &params);
void f_DatGen_Req(vector<string> &params);
void f_Chips_Req(vector<string> &params);
void f_PinFunc_Req(vector<string> &params);
void f_Count_Req(vector<string> &params);
void f_Mar_Req(vector<string> &params);
void f_Udata_Req(vector<string> &params);

int ExecuteMicroInst(string mi,string vecdef);
int ExecuteMicroInstReq(string mi);
string UndefineMicroInst(string mi);

int UndefineSubVecDef(string &vecdef);

int UndefinePatInst(PatInst &inst);
int JumpToGosubFix(PatInst &inst);

int CheckFutureJump2(int cur_idx,vector<string> &params);
int CheckFutureJump(int cur_idx);

bool CheckFutureAddrChange2(string func, vector<string> &params);

int CheckFutureAddrChange(int start_idx,int end_idx);

int ExecutePatInst(int idx);

void Reset();
int ProcPatInst();

void LoadPatInits(string patname);
bool getGlobalInit(string &kv);
int Convert(string dirout);

void addPatternReq(string req_reg, string comment = "");



};

#endif // !defined(AFX_PATTERN_H__07FD771B_4250_4D2D_A854_9F4FD061CB6E__INCLUDED_)
