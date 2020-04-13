/*
 * Copyright 2020, Stefano Ceccherini <stefano.ceccherini@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "BreadCrumbs2.h"

#include <Button.h>
#include <ControlLook.h>
#include <Entry.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <RadioButton.h>
#include <SpaceLayoutItem.h>
#include <String.h>
#include <TextControl.h>

#include <iostream>

const uint32 kMessageCode = '1234';
const uint32 kTextControlMessage = '9999';

using BPrivate::BControlLook;

namespace BC2 {

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


class RootElement : public Element {
public:
	RootElement();
	virtual void SetValue(bool value);
	
	virtual void AttachedToWindow();
	virtual void MouseDown(BPoint where);
	
	virtual void Draw(BRect rect);
	
	virtual BSize MinSize();
	virtual BSize MaxSize();
};

};

using namespace BC2;

BreadCrumbs2::BreadCrumbs2(BPath path)
	:
	BControl("breadcrumbs", "breadcrumbs", new BMessage(), B_WILL_DRAW),
	fTextControl(NULL),
	fPath(path)
{
	SetLayout(new BCardLayout());
	SetViewColor(ui_color(B_DOCUMENT_BACKGROUND_COLOR));
	SetInitialPath(path);
}


void
BreadCrumbs2::SetInitialPath(BPath path)
{
	fPath = path;
	BLayout* layout = GetLayout();

	// Remove existing views
	if (fPathComponents.CountStrings() > 0) {
		fPathComponents.MakeEmpty();
		layout->RemoveView(ChildAt(0));
		layout->RemoveView(fTextControl);
	}

	// Split path and create elements
	BPath parent;
	while (path.GetParent(&parent) == B_OK) {
		BString pathComponent = path.Leaf();
		fPathComponents.Add(pathComponent, 0);
		path = parent;
	}
	
	BGroupView* groupView = new BGroupView(B_HORIZONTAL, 0);
	for (int32 i = 0; i < fPathComponents.CountStrings(); i++) {
		Element* element = new Element(fPathComponents.StringAt(i));
		groupView->AddChild(element);
		groupView->AddChild(new SeparatorElement());
	}
	groupView->GroupLayout()->AddItem(BSpaceLayoutItem::CreateGlue());
	//((BCardLayout*)layout)->SetInsets(4, 4, 4, 4);
	fTextControl = new BTextControl("", fPath.Path(), new BMessage(kTextControlMessage));
	fTextControl->SetTarget(this, Window());

	layout->AddView(groupView);
	layout->AddView(fTextControl);

	((BCardLayout*)layout)->SetVisibleItem(0);
}


void
BreadCrumbs2::Toggle()
{
	BCardLayout* layout = (BCardLayout*)GetLayout();
	if (layout->VisibleIndex() == 0)
		layout->SetVisibleItem(1);
	else
		layout->SetVisibleItem(0);
}


/* virtual */
void
BreadCrumbs2::AllAttached()
{
	BView::AllAttached();
	fTextControl->SetTarget(this, Window());
}


/* virtual */
void
BreadCrumbs2::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMessageCode:
		{
			Element* sourceControl = NULL;
			if (message->FindPointer("source", (void**)&sourceControl) == B_OK) {
				BPath newPath = "/";
				for (int32 i = 0; i < fPathComponents.CountStrings(); i++) {
					BString pathComponent = fPathComponents.StringAt(i);
					newPath.Append(pathComponent);
					if (::strcmp(pathComponent.String(), sourceControl->Label()) == 0)
						break;
				}
				SetInitialPath(newPath);
			}
			break;
		}
		case kTextControlMessage:
		{
			BPath newPath = fTextControl->Text();
			if (BEntry(newPath.Path()).Exists()) {
				SetInitialPath(newPath);
			} else {
				fTextControl->SetText(fPath.Path());
			}
			break;
		}
		case 'TOGL':
		{
			Toggle();
			break;
		}
		default:
			BControl::MessageReceived(message);
			break;
	}
}


/* virtual */
void
BreadCrumbs2::Draw(BRect updateRect)
{
	BControl::Draw(updateRect);
	BRect rect(Bounds());
	rgb_color base = LowColor();
	be_control_look->DrawTextControlBorder(this, rect, updateRect,
									base, be_control_look->Flags(this));
		
	_DrawBackground(updateRect);
	_DrawText(updateRect);
}


/* virtual */
void
BreadCrumbs2::MouseDown(BPoint where)
{
	BControl::MouseDown(where);
	
}


/* virtual */
BSize
BreadCrumbs2::MinSize()
{
	return MaxSize();
}


/* virtual */
BSize
BreadCrumbs2::MaxSize()
{
	float maxWidth = 400;
	float maxHeight = 25;
	/*BLayout* layout = GetLayout();
	for (int32 i = 0; i < layout->CountItems(); i++) {
		BLayoutItem* item = layout->ItemAt(i);
		if (dynamic_cast<Element*>(item) != NULL) {
			BSize maxSize = item->MaxSize();
			maxWidth += maxSize.width + 20;
			maxHeight = std::max(maxHeight, maxSize.height); 
		}
	}*/
	
	return BSize(maxWidth, maxHeight);
}


/* virtual */
BSize
BreadCrumbs2::PreferredSize()
{
	return MinSize();
}


void
BreadCrumbs2::_DrawBackground(BRect updateRect)
{
	rgb_color base = LowColor();
	rgb_color background = ViewColor();
	uint32 flags = be_control_look->Flags(this);
	BRect frame(Bounds());
	//be_control_look->DrawButtonBackground(this, frame, updateRect, base, flags);
}


void
BreadCrumbs2::_DrawText(BRect updateRect)
{
	/*BRect bounds(Bounds());
	BPoint location = bounds.LeftTop();	
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	location.y += (fontHeight.ascent + fontHeight.descent);
	
	MovePenTo(location);
	SetLowColor(ui_color(B_CONTROL_TEXT_COLOR));
	
	for (int32 i = 0; i < fPathComponents.CountStrings(); i++) {
		DrawString(fPathComponents.StringAt(i).String());
		MovePenBy(5, 0);
		DrawString(">");
		MovePenBy(5, 0);
	}*/
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
	BRect frame = Bounds().InsetByCopy(-1, 0);
	uint32 flags = be_control_look->Flags(this);
	//be_control_look->DrawButtonFrame(this, frame, updateRect,
	//								base, background, flags);
	//be_control_look->DrawButtonBackground(this, frame, updateRect, base, flags);
	be_control_look->DrawLabel(this, Label(), frame, updateRect, base, flags,
									BAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER),
									&textColor);
}


/* virtual */
BSize
Element::MinSize()
{
	return MaxSize();
}


/* virtual */
BSize
Element::MaxSize()
{
	const float kPadding = 15;
	float width = StringWidth(Label()) + kPadding;
	float height = 25;
	return BSize(width, height);
}


// SeparatorElement
SeparatorElement::SeparatorElement()
	:
	Element(">")
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
	be_control_look->DrawLabel(this, Label(), frame, updateRect, base, flags,
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
	float width = 20;
	float height = 20;
	return BSize(width, height);
}


// RootElement
RootElement::RootElement()
	:
	Element("/")
{
}


/* virtual */
void
RootElement::AttachedToWindow()
{
	BControl::AttachedToWindow();
}


/* virtual */
void
RootElement::SetValue(bool value)
{
}


/* virtual */
void
RootElement::MouseDown(BPoint where)
{
	BControl::MouseDown(where);
}


/* virtual */
void
RootElement::Draw(BRect updateRect)
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
RootElement::MinSize()
{
	float width = 20;
	float height = 20;
	return BSize(width, height);
}


/* virtual */
BSize
RootElement::MaxSize()
{
	const float kPadding = 20;
	float width = 20;
	float height = 20;
	return BSize(width, height);
}
