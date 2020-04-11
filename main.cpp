/*
 * Copyright 2018, Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "BreadCrumbs.h"

#include <Application.h>
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
	BPath path("/home/config/settings");
	BView* view = new BreadCrumbs(path);
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(view)
		.End();
	
}

