/*
 * Copyright 2020, Stefano Ceccherini <stefano.ceccherini@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "BreadCrumbs.h"

#include <Button.h>
#include <ControlLook.h>
#include <Directory.h>
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

const float kHorizontalPadding = 3;
const float kVerticalPadding = 4;

class Element : public BControl {
public:
	Element(BString string);
	virtual void SetValue(bool value);

	virtual void AttachedToWindow();
	virtual void MouseDown(BPoint where);
	virtual void MouseMoved(BPoint where, uint32 code,
									const BMessage* dragMessage);
	virtual void Draw(BRect rect);

	virtual BSize MinSize();
	virtual BSize MaxSize();

	void SetHovering(bool hover) { fHovering = hover; };
	bool IsHovering() const { return fHovering; };
private:
	bool fHovering;
	uint8 fHoveringLevel;
};


class SeparatorElement : public Element {
public:
	SeparatorElement();
	virtual void SetValue(bool value);
	
	virtual void AttachedToWindow();
	virtual void MouseDown(BPoint where);
	
	virtual void Draw(BRect rect);
};


class ContainerView : public BGroupView {
public:
	ContainerView();
	virtual void Draw(BRect updateRect);
	virtual void MouseDown(BPoint where);
};

}

using namespace BC2;

BreadCrumbs::BreadCrumbs(BPath path)
	:
	BControl("breadcrumbs", "breadcrumbs", new BMessage(), B_WILL_DRAW|B_DRAW_ON_CHILDREN),
	fTextControl(NULL),
	fPath(path),
	fPathHint("")
{
	SetLayout(new BCardLayout());
	SetViewColor(ui_color(B_DOCUMENT_BACKGROUND_COLOR));
	SetInitialPath(path);
}


void
BreadCrumbs::SetInitialPath(BPath path)
{
	fPath = path;
	fPathHint = "";
	BLayout* layout = GetLayout();

	// Remove existing views
	if (fPathComponents.CountStrings() > 0) {
		fPathComponents.MakeEmpty();
		while (layout->CountItems())
			layout->RemoveView(ChildAt(0));
	}

	// Split path and create elements
	BPath parent;
	while (path.GetParent(&parent) == B_OK) {
		BString pathComponent = path.Leaf();
		fPathComponents.Add(pathComponent, 0);
		path = parent;
	}
	
	BGroupView* groupView = new ContainerView();
	for (int32 i = 0; i < fPathComponents.CountStrings(); i++) {
		groupView->AddChild(new SeparatorElement());
		Element* element = new Element(fPathComponents.StringAt(i));
		groupView->AddChild(element);
	}
	groupView->GroupLayout()->AddItem(BSpaceLayoutItem::CreateGlue());

	fTextControl = new BTextControl("", fPath.Path(), new BMessage(kTextControlMessage));
	fTextControl->SetTarget(this, Window());

	BView* view = BLayoutBuilder::Group<>(B_VERTICAL)
		.AddGroup(B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING, 0, B_USE_DEFAULT_SPACING, 0)
			.Add(fTextControl)
		.End()
		.View();
		
	layout->AddView(groupView);
	layout->AddView(view);

	((BCardLayout*)layout)->SetVisibleItem(0);
}


void
BreadCrumbs::Toggle()
{
	BCardLayout* layout = (BCardLayout*)GetLayout();
	if (layout->VisibleIndex() == 0) {
		layout->SetVisibleItem(1);
		fTextControl->MakeFocus();
		// the TextInput inside the BTextControl catches B_ENTER, and does
		// not send a message if the content hasn't changed, so we need to catch it
		// in another way. Thus we receive all keyboard events
		SetEventMask(B_KEYBOARD_EVENTS, 0);
	} else {
		SetEventMask(0, 0);
		layout->SetVisibleItem(0);
	}
}


bool
BreadCrumbs::TextControlShown() const
{
	BCardLayout* layout = (BCardLayout*)GetLayout();
	return layout->VisibleIndex() == 0;
}


void
BreadCrumbs::RetrievePathHint(const BString& current, const BString& newText)
{
	BString leaf = BPath(newText).Leaf();
	BPath parent(newText);
	parent.GetParent(&parent);

	BPath currentPath = parent;
	BEntry entry;
	BDirectory directory(currentPath.Path());
	BPath newPathHint = "";
	while (directory.GetNextEntry(&entry) == B_OK) {
		if (entry.IsDirectory() && BString(entry.Name()).StartsWith(leaf)) {
			newPathHint = currentPath;
			newPathHint.Append(entry.Name());
			break;
		}
	}
	fPathHint = newPathHint;
}


/* virtual */
void
BreadCrumbs::AllAttached()
{
	BView::AllAttached();
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
		case B_KEY_DOWN:
		{
			int8 byte;
			if (message->FindInt8("byte", &byte) == B_OK
				&& byte == B_TAB) {
				SetInitialPath(fPathHint.Path());
			}
			BControl::MessageReceived(message);
			break;
		}
		case B_KEY_UP:
			RetrievePathHint(fPath.Path(), fTextControl->Text());
			Invalidate();
			BControl::MessageReceived(message);
			break;
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
	BRect rect(Bounds());
	rgb_color base = LowColor();
	be_control_look->DrawTextControlBorder(this, rect, updateRect,
									base, be_control_look->Flags(this));
		
}


/* virtual */
void
BreadCrumbs::DrawAfterChildren(BRect updateRect)
{
	if (!TextControlShown()) {
		float textWidth = fTextControl->StringWidth(fTextControl->Text());
		BString hintLeaf = fPathHint.Leaf();
		BString pathLeaf = BPath(fTextControl->Text()).Leaf();
		float leafWidth = fTextControl->StringWidth(pathLeaf);
		MovePenTo(textWidth - leafWidth + 15, 5 + Bounds().Height() / 2);
		if (hintLeaf != pathLeaf) {
			SetHighColor(tint_color(HighColor(), B_LIGHTEN_2_TINT));
			DrawString(hintLeaf);
		}
	}
	BControl::DrawAfterChildren(updateRect);
}


/* virtual */
void
BreadCrumbs::MouseDown(BPoint where)
{
	BControl::MouseDown(where);
}


/* virtual */
void
BreadCrumbs::KeyDown(const char* bytes, int32 numBytes)
{
	switch (bytes[0]) {
		case B_ENTER:
		case B_ESCAPE:
		{
			BPath newPath = fTextControl->Text();
			if (BEntry(newPath.Path()).Exists()) {
				SetInitialPath(newPath);
			} else {
				fTextControl->SetText(fPath.Path());
			}
		}
		default:
		{
			BControl::KeyDown(bytes, numBytes);
			break;
		}
	}
}


/* virtual */
BSize
BreadCrumbs::MinSize()
{
	return MaxSize();
}


/* virtual */
BSize
BreadCrumbs::MaxSize()
{
	float maxWidth = 400;
	float maxHeight = 30;
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
BreadCrumbs::PreferredSize()
{
	return MinSize();
}


// Element
Element::Element(BString string)
	:
	BControl(string.String(), string.String(), new BMessage(kMessageCode), B_WILL_DRAW)
{
	SetHovering(false);
	fHoveringLevel = 0;
	SetViewColor(ui_color(B_DOCUMENT_BACKGROUND_COLOR));
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
Element::MouseMoved(BPoint where, uint32 code,
									const BMessage* dragMessage)
{
	switch (code) {
		case B_ENTERED_VIEW:
			SetHovering(true);
			Invalidate();
			break;
		case B_EXITED_VIEW:
			SetHovering(false);
			Invalidate();
			break;
		default:
			break;
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
	if (IsHovering()) {
		be_control_look->DrawButtonFrame(this, frame, updateRect, base, background, flags);
	}
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
	float width = StringWidth(Label()) + kHorizontalPadding;
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	float height = fontHeight.ascent + fontHeight.descent + kVerticalPadding;
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
	// Empty
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


// ContainerView
ContainerView::ContainerView()
	:
	BGroupView(B_HORIZONTAL, 0)
{
	//GroupLayout()->SetSpacing(2);
	SetViewColor(ui_color(B_DOCUMENT_BACKGROUND_COLOR));
}


/* virtual */
void
ContainerView::Draw(BRect updateRect)
{
	BGroupView::Draw(updateRect);
	StrokeRect(Bounds());
}


/* virtual */
void
ContainerView::MouseDown(BPoint where)
{
	BGroupView::MouseDown(where);
	BreadCrumbs* view = dynamic_cast<BreadCrumbs*>(Parent());
	if (view != NULL) {
		view->Toggle();
	}
}
