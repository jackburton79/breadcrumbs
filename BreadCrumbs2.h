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

	bool TextControlShown() const;
	
	void RetrievePathHint(const BString& current, const BString& newText);

	virtual void AllAttached();
	virtual void MessageReceived(BMessage* message);
	virtual void Draw(BRect updateRect);
	virtual	void DrawAfterChildren(BRect updateRect);
	virtual void MouseDown(BPoint where);
	virtual void KeyDown(const char* bytes, int32 numBytes);

	virtual BSize MinSize();
	virtual BSize MaxSize();
	virtual BSize PreferredSize();
private:	
	BTextControl* fTextControl;
	BStringList fPathComponents;
	BPath fPath;
	BPath fPathHint;
};


#endif // _BREADCRUMBS_H
