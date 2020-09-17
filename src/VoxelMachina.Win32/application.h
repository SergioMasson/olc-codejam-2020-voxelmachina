#pragma once

class IGameApp
{
public:
	// This function can be used to initialize application state and will run after essential
	// hardware resources are allocated.  Some state that does not depend on these resources
	// should still be initialized in the constructor such as pointers and flags.
	virtual void Startup(void) = 0;
	virtual void Cleanup(void) = 0;

	// Decide if you want the app to exit.  By default, app continues until the 'ESC' key is pressed.
	virtual bool IsDone(void) = 0;

	// The update method will be invoked once per frame.  Both state updating and scene
	// rendering should be handled by this method.
	virtual void Update(float deltaT, float totalTime) = 0;

	// Official rendering pass
	virtual void RenderScene(void) = 0;

	virtual void RenderUI(void) = 0;

	virtual void Resize(uint32_t width, uint32_t height) = 0;

	virtual ~IGameApp() {};
};

extern HWND g_coreWindow;

void RunApplication(IGameApp& app, HINSTANCE instance, const wchar_t* className);