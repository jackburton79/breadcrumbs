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
class BStringView;
class BTextControl;
class BreadCrumbs : public BControl {
public:
	BreadCrumbs(BPath path);
	virtual void AllAttached();
	virtual void MessageReceived(BMessage* message);
	virtual void Draw(BRect updateRect);
	
	void SetInitialPath(BPath path);
	void Test();
private:
	BStringView *fStringView;
	BTextControl* fTextControl;
	BObjectList<Element> fElements;
	BPath fPath;
	BPath fCurrentPath;
};


#endif // _BREADCRUMBS_H
