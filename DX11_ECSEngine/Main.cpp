#include "Main.h"
#include "InitEngine.h"
#include <Windows.h>

// -------------------------------------Begin ECS----------------------------------------
// - Ensure EnTT is attached through Properties->C/C++-> Additional include directories
// - Entities are stored in the 'm_rendererRegistry' in InitEngine class
// - Components are structs attached to the registry using an entity, and they are stored
//	 in Components.h file. 
// - 
// --------------------------------------------------------------------------------------
int WINAPI WinMain(
	HINSTANCE hInstance,		// The handle to this app's instance
	HINSTANCE hPrevInstance,	// A handle to the previous instance of the app (always NULL)
	LPSTR lpCmdLine,			// Command line params
	int nCmdShow)				// How the window should be shown (we ignore this)
{
	// Ensure "Current Directory" (relative path) is always the .exe's folder
	// - Without this, the relative path is different when running through VS
	//    and when running the .exe directly, which makes it a pain to load files
	//    - Running through VS: Current Dir is the *project folder*
	//    - Running from .exe:  Current Dir is the .exe's folder
	// - This has nothing to do with DEBUG and RELEASE modes - it's purely a 
	//    Visual Studio "thing", and isn't obvious unless you know to look 
	//    for it.  In fact, it could be fixed by changing a setting in VS, but
	//    the option is stored in a user file (.suo), which is ignored by most
	//    version control packages by default.  Meaning: the option must be
	//    changed every on every PC.  Ugh.  So instead, I fixed it here.
	// - This is a new change this year to simplify a long-standing headache.  
	//    If it breaks something on your end, feel free to comment this section out
	{
		// Get the real, full path to this executable, end the string before
		// the filename itself and then set that as the current directory
		char currentDir[1024] = {};
		GetModuleFileName(0, currentDir, 1024);
		char* lastSlash = strrchr(currentDir, '\\');
		if (lastSlash)
		{
			*lastSlash = 0; // End the string at the last slash character
			SetCurrentDirectory(currentDir);
		}
	}

	// Store instance handle, used for window screen layout
	RenderWindow r_Win;
	r_Win.hInstance = hInstance;
	r_Win.titleBarStats = true;
	r_Win.titleBarText = "DirectX Window";

	// Create engine object
	InitEngine* engine = new InitEngine();

	// Run the engine with initialization of windows instance handle
	engine->InitEntt(r_Win);

	// Clean the registry.
	engine->Clean();

	delete engine;

	return 0;
}


Main::Main()
{
}


Main::~Main()
{
}
