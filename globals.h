
#include "common.h"
extern vector<int> G_COUNTERS;
extern vector<int> G_COUNTERS_RLD;
extern string G_JMP_LABEL;
extern int G_VEC_LINE_SIZE;

extern char G_DEFAULT_PIN_STATE;
extern string G_INPUT_FILE;
extern string G_PINSCRAMBLE_NAME;
extern string G_RPT_OPT; // used if repeat condition is required
extern int G_COUNTER_SEL; // index of the counter selected through recent call of COUNT micro instruction
extern int G_PAT_IDX;
extern int G_PATINST_IDX;
extern int G_PATINST_SEL; // index of pattern instruction currently processed in ConvertPatInst

extern unsigned long G_JAMREG;
extern unsigned long G_DATREG;
extern unsigned long G_UDATA;
extern unsigned long G_DBM;
extern unsigned long G_XADDR;
extern unsigned long G_YADDR;


extern unsigned long G_YMAIN;
extern unsigned long G_YBASE;
extern unsigned long G_YFIELD;
extern unsigned long G_XMAIN;
extern unsigned long G_XBASE;
extern unsigned long G_XFIELD;
extern unsigned long G_NUMX;
extern unsigned long G_NUMY;

extern int G_ADHIZ;

extern string  currentPattern;
extern map<string,string>  jump2gosub;
extern map<string, string> jumpPatternMap;
extern map<string, map <string, string>> pat_JumpLabel;


extern bool G_CS_ACTIVE_H[8];

extern vector<string> vTriStatePins;

extern map<string,string> stdmapDEFINE;


extern map<string,int> stdmapVECDEF;

extern map<int,char> stdmapDEFCHAR;


//typedef pair<int,char> def_char_pair;
//vector<def_char_pair> vpairDEFCHAR;

extern map<string,string> stdmapPATINITS;


extern ofstream debugfile;
extern ofstream warnlogfile;
extern ofstream errlogfile;


extern int G_MODE;

extern bool G_ADD_ORIG;

extern bool G_DEBUG_ON;

extern bool G_HALT_E;

extern bool G_HALT_W;

extern bool G_VERBOSE;

extern string G_VECDEF;

extern int G_WARN_CNT;

extern bool G_WARN_ALU;

extern bool G_ALU_ON;

extern string directory;

extern int XPosLast;
extern int YPosLast;


//#define _debug_on_

