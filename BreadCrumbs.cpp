/*
 * Copyright 2018, Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "BreadCrumbs.h"

#include <Button.h>
#include <ControlLook.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <RadioButton.h>
#include <String.h>

#include <iostream>

using BPrivate::BControlLook;

class Element : public BControl {
public:
	Element(BString string);
	virtual void SetValue(bool value);
	
	virtual void MouseDown(BPoint where);
	
	virtual void Draw(BRect rect);
	
	virtual BSize MinSize();
	virtual BSize MaxSize();
};


BreadCrumbs::BreadCrumbs(BPath path)
	:
	BControl("breadcrumbs", "bre", new BMessage(), B_WILL_DRAW),
	fPath(path)
{
	SetLayout(new BGroupLayout(B_HORIZONTAL, 0));
	SetPath(path);
}


void
BreadCrumbs::SetPath(BPath path)
{
	fElements.MakeEmpty();
	fPath = fCurrentPath = path;
	
	BPath parent;
	while (path.GetParent(&parent) == B_OK) {
		Element* element = new Element(path.Leaf());
		fElements.AddItem(element, 0);
		path = parent;
	}
	
	for (int32 i = 0; i < fElements.CountItems(); i++) {
		Element* element = fElements.ItemAt(i);
		GetLayout()->AddView(element);
	}
}


void
BreadCrumbs::Test()
{
}


// Element
Element::Element(BString string)
	:
	BControl(string.String(), string.String(), new BMessage(), B_WILL_DRAW)
{
}


/* virtual */
void
Element::SetValue(bool value)
{
	if (value != Value()) {
		SetValueNoUpdate(value);
		Invalidate(Bounds());
	}
	
	if (value == B_CONTROL_OFF)
		return;
	
	std::cout << Label() << ": B_CONTROL_ON" << std::endl;
	BView* parent = NULL;
	BView* child = NULL;
	
	if (Window() != NULL)
		child = Window()->ChildAt(0);
	
	while (child != NULL) {
		Element* element = dynamic_cast<Element*>(child);
		if (element != NULL && element != this)
			element->SetValue(B_CONTROL_OFF);
		child = child->NextSibling();
	}
}


/* virtual */
void
Element::MouseDown(BPoint where)
{
	BControl::MouseDown(where);
	SetValue(B_CONTROL_ON);
}


/* virtual */
void
Element::Draw(BRect updateRect)
{
	BControl::Draw(updateRect);
	
	rgb_color highColor = HighColor();
	rgb_color lowColor = LowColor();
	BRect frame = Bounds();
	be_control_look->DrawButtonFrame(this, frame, updateRect, highColor, lowColor);
	be_control_look->DrawButtonBackground(this, frame, updateRect, lowColor);
	be_control_look->DrawLabel(this, Label(), frame, updateRect, highColor,
									BControlLook::B_IS_CONTROL,
									BAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER));
}


/* virtual */
BSize
Element::MinSize()
{
	float width = StringWidth(Label());
	float height = 30;
	return BSize(width, height);
}


/* virtual */
BSize
Element::MaxSize()
{
	const float kPadding = 20;
	float width = StringWidth(Label()) + kPadding;
	float height = 30;
	return BSize(width, height);
}
