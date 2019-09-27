#include "sCfgLine.h"
//#include <vld.h>

sCfgLine::sCfgLine(sObjParmsDef, char* rawLine_) : sObj(sObjParmsVal) {

	//-- 1. set startpos, raw, clean, naked
	strcpy_s(raw, XMLLINE_MAXLEN, rawLine_);
	strcpy_s(clean, XMLLINE_MAXLEN, rawLine_);
	stripChar(clean, ' '); stripChar(clean, '\t'); stripChar(clean, '\n');
	strcpy_s(naked, XMLLINE_MAXLEN, clean);
	stripChar(naked, '<'); stripChar(naked, '/'); stripChar(naked, '>');

	//-- 2. set type
	if (isComment()) {
		type=cfgLine_Comment;
	} else if (clean[0]=='<' && clean[1]!='/' && clean[1]!='!' && clean[strlen(clean)-1]=='>') {
		type=cfgLine_KeyStart;
	} else if (clean[0]=='<' && clean[1]=='/' && clean[strlen(clean)-1]=='>') {
		type=cfgLine_KeyEnd;
	} else if (clean[0]=='<' && clean[1]=='!' && clean[strlen(clean)-1]=='>') {
		stripChar(naked, '!');
		type=cfgLine_Include;

	} else {
		type=cfgLine_Parm;
		//-- separate parameter name from value
		if(!getValuePair(naked, pname, pval, '=')) fail("wrong parameter format: %s", naked);
	}

}

sCfgLine::~sCfgLine(){}