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

const uint32 kMessageCode = '1234';

using BPrivate::BControlLook;

class Element : public BControl {
public:
	Element(BString string);
	virtual void SetValue(bool value);
	
	virtual void AttachedToWindow();
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
	SetLayout(new BGroupLayout(B_HORIZONTAL, 1));
	SetPath(path);
}


/* virtual */
void
BreadCrumbs::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMessageCode:
		{
			Element* sourceControl = NULL;
			if (message->FindPointer("source", (void**)&sourceControl) == B_OK) {
				fCurrentPath.Unset();
				fCurrentPath = "/";
				for (int32 i = 0; i < fElements.CountItems(); i++) {
					Element* element = fElements.ItemAt(i);
					fCurrentPath.Append(element->Label());
					if (::strcmp(element->Label(), sourceControl->Label()) == 0)
						break;
				}
			}
			break;
		}
		default:
			BControl::MessageReceived(message);
			break;
	}
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
	BControl(string.String(), string.String(), new BMessage(kMessageCode), B_WILL_DRAW)
{
}


/* virtual */
void
Element::AttachedToWindow()
{
	SetTarget(Parent(), Window());
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
	
	BView* parent = Parent();
	BView* child = NULL;
	
	if (parent != NULL) {
		child = parent->ChildAt(0);
	} else if (Window() != NULL)
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
	if (Value() != B_CONTROL_ON) {
		SetValue(B_CONTROL_ON);
		Invoke();
	}
}


/* virtual */
void
Element::Draw(BRect updateRect)
{
	BControl::Draw(updateRect);
	
	rgb_color background = ViewColor();
	rgb_color textColor = ui_color(B_CONTROL_TEXT_COLOR);
	rgb_color base = LowColor();
	BRect frame = Bounds();
	uint32 flags = be_control_look->Flags(this);
	be_control_look->DrawButtonFrame(this, frame, updateRect,
									base, background, flags);
	be_control_look->DrawButtonBackground(this, frame, updateRect, base, flags);
	be_control_look->DrawLabel(this, Label(), frame, updateRect, base, flags,
									BAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER),
									&textColor);
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
