/*
 * Copyright 2020, Stefano Ceccherini <stefano.ceccherini@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _BREADCRUMBS_H
#define _BREADCRUMBS_H


#include <Control.h>
#include <Path.h>
#include <SupportDefs.h>
#include <StringList.h>

class BTextControl;
class BreadCrumbs : public BControl {
public:
	BreadCrumbs(BPath path);
	void SetInitialPath(BPath path);

	virtual void AllAttached();
	virtual void MessageReceived(BMessage* message);
	virtual void Draw(BRect updateRect);

	virtual BSize MinSize();
	virtual BSize MaxSize();
	virtual BSize PreferredSize();
private:
	BTextControl* fTextControl;
	BStringList fPathComponents;
	BPath fPath;
	BPath fCurrentPath;
};


#endif // _BREADCRUMBS_H
