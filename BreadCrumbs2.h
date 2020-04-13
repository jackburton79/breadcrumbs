/*
 * Copyright 2020, Stefano Ceccherini <stefano.ceccherini@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _BREADCRUMBS2_H
#define _BREADCRUMBS2_H


#include <Control.h>
#include <Path.h>
#include <SupportDefs.h>
#include <StringList.h>

class BTextControl;
class BreadCrumbs2 : public BControl {
public:
	BreadCrumbs2(BPath path);
	void SetInitialPath(BPath path);
	void Toggle();
	
	virtual void AllAttached();
	virtual void MessageReceived(BMessage* message);
	virtual void Draw(BRect updateRect);
	virtual void MouseDown(BPoint where);

	virtual BSize MinSize();
	virtual BSize MaxSize();
	virtual BSize PreferredSize();
private:
	void _DrawBackground(BRect updateRect);
	void _DrawText(BRect updateRect);
	
	BTextControl* fTextControl;
	BStringList fPathComponents;
	BPath fPath;
};


#endif // _BREADCRUMBS_H
