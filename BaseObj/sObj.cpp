#include "sObj.h"

sObj::sObj(sObjParmsDef) {

	name=sname_;
	parent=parent_;

	if (parent==nullptr) {
		depth=0;
		name->update(depth, nullptr);
	} else {
		depth=parent->depth+1;
		name->update(depth, parent->name);
		parent->child.push_back(this);
	}


	if (dbg_==nullptr) {
		if (parent==nullptr) {
			dbg = defaultdbg;
		} else {
			dbg = parent->dbg;
		}
	} else {
		dbg = dbg_;
	}

	if (parent==nullptr) {
		dbg->createOutFile(name->base, this, depth);
	} else {
		if (dbg!=parent->dbg) dbg->createOutFile(name->base, this, depth);
	}

}

sObj::~sObj() {
	while (child.size()>0) {
		delete child[child.size()-1];
	}
	delete name;
	if (parent!=nullptr) {
		if (dbg!=parent->dbg) delete dbg;	//-- to avoid a child deleting its parent's dbg
	} else {
		delete dbg;
	}

	if (parent!=nullptr) {
		parent->child.pop_back();
	}
}

void sObj::findChild(const char* relName, sObj** retObj) {

	int l=(int)strlen(relName);
	if (l==0) return;

	char cname[ObjNameMaxLen];
	int seppos=0;
	int i;

	//-- find first '/', and split the first step from the remainder of relName
	for (i=0; i<l; i++) {
		cname[i]=relName[i];
		if (relName[i]=='/') {
			break;
		}
	}
	cname[i]='\0';

	//-- compare cname with short name of every child in this object
	(*retObj)=nullptr;
	int c;
	for (c=0; c<child.size(); c++) {
		if (_stricmp(cname, child[c]->name->base)==0) {
			(*retObj)=child[c];
			break;
		}
	}

	//-- if first step is found, call child's findChild with the remainder of relName
	if ((*retObj)!=nullptr && l>i) {
		(*retObj)->findChild(&relName[i+1], retObj);
	}

}
