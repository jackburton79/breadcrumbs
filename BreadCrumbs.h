/*
 * Copyright 2020, Stefano Ceccherini <stefano.ceccherini@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _BREADCRUMBS_H
#define _BREADCRUMBS_H


#include <Control.h>
#include <ObjectList.h>
#include <Path.h>
#include <SupportDefs.h>

class Element;
class BreadCrumbs : public BControl {
public:
	BreadCrumbs(BPath path);
	virtual void MessageReceived(BMessage* message);
	
	void SetPath(BPath path);
	void Test();
private:
	BObjectList<Element> fElements;
	BPath fPath;
	BPath fCurrentPath;
};


#endif // _BREADCRUMBS_H
