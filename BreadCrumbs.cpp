/*
 * Copyright 2020, Stefano Ceccherini <stefano.ceccherini@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "BreadCrumbs.h"

#include <Button.h>
#include <ControlLook.h>
#include <Entry.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <RadioButton.h>
#include <String.h>
#include <TextControl.h>

#include <iostream>

const uint32 kMessageCode = '1234';
const uint32 kTextControlMessage = '9999';

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


class SeparatorElement : public Element {
public:
	SeparatorElement();
	virtual void SetValue(bool value);
	
	virtual void AttachedToWindow();
	virtual void MouseDown(BPoint where);
	
	virtual void Draw(BRect rect);
	
	virtual BSize MinSize();
	virtual BSize MaxSize();
};


BreadCrumbs::BreadCrumbs(BPath path)
	:
	BControl("breadcrumbs", "breadcrumbs", new BMessage(), B_WILL_DRAW),
	fPath(path)
{
	BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);
	SetInitialPath(path);
}


/* virtual */
void
BreadCrumbs::AllAttached()
{
	BControl::AllAttached();
	fTextControl->SetTarget(this, Window());
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
				fTextControl->SetText(fCurrentPath.Path());
			}
			break;
		}
		case kTextControlMessage:
		{
			BPath newPath = fTextControl->Text();
			std::cout << newPath.Path() << std::endl;
			if (BEntry(newPath.Path()).Exists()) {
				SetInitialPath(newPath);
			} else {
				fTextControl->SetText(fCurrentPath.Path());
			}	
			break;
		}
		default:
			BControl::MessageReceived(message);
			break;
	}
}


/* virtual */
void
BreadCrumbs::Draw(BRect updateRect)
{
	BControl::Draw(updateRect);

	/*
	BRect rect(Bounds());
	rect.InsetBy(2, 2);
	SetLowColor(ui_color(B_CONTROL_TEXT_COLOR));
	StrokeRect(rect);
	*/
}


void
BreadCrumbs::SetInitialPath(BPath path)
{
	fPath = fCurrentPath = path;

	BLayout* layout = GetLayout();
	
	if (fElements.CountItems() > 0) {
		fElements.MakeEmpty();
		layout->RemoveView(ChildAt(0));
		layout->RemoveView(fTextControl);
	}

	BPath parent;
	while (path.GetParent(&parent) == B_OK) {
		Element* element = new Element(path.Leaf());
		fElements.AddItem(element, 0);
		path = parent;
	}
	
	BGroupView* groupView = new BGroupView(B_HORIZONTAL, -1);
	for (int32 i = 0; i < fElements.CountItems(); i++) {
		Element* element = fElements.ItemAt(i);
		groupView->AddChild(element);
		groupView->AddChild(new SeparatorElement());
	}
	
	fTextControl = new BTextControl("Path:", fPath.Path(), new BMessage(kTextControlMessage));
	fTextControl->SetTarget(this, Window());
	layout->AddView(groupView);
	layout->AddView(fTextControl);
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


// SeparatorElement
SeparatorElement::SeparatorElement()
	:
	Element("")
{
}


/* virtual */
void
SeparatorElement::AttachedToWindow()
{
	BControl::AttachedToWindow();
}


/* virtual */
void
SeparatorElement::SetValue(bool value)
{
}


/* virtual */
void
SeparatorElement::MouseDown(BPoint where)
{
	BControl::MouseDown(where);
}


/* virtual */
void
SeparatorElement::Draw(BRect updateRect)
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
	be_control_look->DrawLabel(this, "/", frame, updateRect, base, flags,
									BAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER),
									&textColor);
}


/* virtual */
BSize
SeparatorElement::MinSize()
{
	float width = 20;
	float height = 20;
	return BSize(width, height);
}


/* virtual */
BSize
SeparatorElement::MaxSize()
{
	const float kPadding = 20;
	float width = 20;
	float height = 20;
	return BSize(width, height);
}
