/*
 * Copyright 2020, Stefano Ceccherini <stefano.ceccherini@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "BreadCrumbs.h"
#include "BreadCrumbs2.h"

#include <Application.h>
#include <Button.h>
#include <GroupLayoutBuilder.h>
#include <LayoutBuilder.h>
#include <Window.h>

class Window : public BWindow {
public:
	Window(BRect rect);
};


int main()
{
	BApplication app("application/x-vnd.testapp");
	Window* win = new Window(BRect(100, 100, 400, 300));
	win->Show();
	app.Run();
}


Window::Window(BRect rect)
	:
	BWindow(rect, "window", B_TITLED_WINDOW, B_QUIT_ON_WINDOW_CLOSE|B_SUPPORTS_LAYOUT)
{
	BPath path("/boot/home/config/settings");
	BView* view = new BreadCrumbs(path);
	BView* view2 = new BreadCrumbs2(path);
	BButton* button = new BButton("Toggle", new BMessage('TOGL'));
	BButton* button2 = new BButton("Toggle", new BMessage('TOGL'));
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGlue()
		/*.AddGroup(B_HORIZONTAL)
			.Add(view)
			.Add(button)
		.End()*/
		.AddGroup(B_HORIZONTAL)
			.Add(view2)
			.Add(button2)
		.End()
		.AddGlue()
	.End();
	
	//button->SetTarget(view);
	button2->SetTarget(view2);
}

