
#include "common.h"

#include "pattern.h"

int ParsePatternFile(string directoryDebug , string fname_in);
int ParseGosubLabel(string fname_in,vector<string>);
int creatSubPattern(string patName, string newPatName, int startLine, int stopLine);