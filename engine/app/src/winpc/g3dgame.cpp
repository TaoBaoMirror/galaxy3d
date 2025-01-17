#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <cstdio>

#include "World.h"
#include "GraphicsDevice.h"
#include "Screen.h"
#include "Camera.h"
#include "Input.h"
#include "GTTime.h"
#include "Cursor.h"
#include "Debug.h"
#include "SplashScreen.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "openal32.lib")

using namespace Galaxy3D;

static const char g_title[] = "Viry3D Engine";
static int g_screen_w = 1280;
static int g_screen_h = 720;
static HINSTANCE g_hinst;
static HCURSOR g_cursor_default;
HWND g_hwnd;
static bool g_fullscreen = false;

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, int width, int height);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	AllocConsole();
	SetConsoleTitle("Galaxy3D Console");
	FILE* fstdout = 0;
	freopen_s(&fstdout, "CONOUT$", "w", stdout);

    if(g_fullscreen)
    {
        g_screen_w = GetSystemMetrics(SM_CXSCREEN);
        g_screen_h = GetSystemMetrics(SM_CYSCREEN);
    }

	if(FAILED(InitWindow(hInstance, nCmdShow, g_screen_w, g_screen_h)))
		return 0;

	Screen::SetSize(g_screen_w, g_screen_h);
	GraphicsDevice::GetInstance()->Init(g_hwnd);
	World::Init();

	GameObject::Create("launcher")->AddComponent<SplashScreen>();

	// Main message loop
	MSG msg = {0};
	while(WM_QUIT != msg.message)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
            // Sleep(1);

            World::Update();
            Camera::RenderAll();
		}
	}

	World::Done();
    GraphicsDevice::Done();

	if(fstdout != 0)
	{
		fclose(fstdout);
	}

	return (int) msg.wParam;
}

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, int width, int height)
{
    g_cursor_default = LoadCursor(0, IDC_ARROW);

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = g_cursor_default;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = g_title;
	wcex.hIconSm = 0;
	if(!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hinst = hInstance;
	RECT rc = {0, 0, width, height};
	
    DWORD style;
    int w;
    int h;
    if(g_fullscreen)
    {
        style = WS_POPUP | WS_MINIMIZEBOX;
        w = width;
        h = height;
    }
    else
    {
        style = WS_SYSMENU | WS_MINIMIZEBOX;
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        w = rc.right - rc.left;
        h = rc.bottom - rc.top;
    }

	g_hwnd = CreateWindow(g_title, g_title, style,
		0, 0, w, h, NULL, NULL, hInstance,
		NULL);
	if(!g_hwnd)
		return E_FAIL;

	ShowWindow(g_hwnd, nCmdShow);

	return S_OK;
}

static bool g_input_down = false;
extern std::vector<Touch> g_input_touches;
extern std::list<Touch> g_input_touch_buffer;
extern bool g_key_down[KeyCode::COUNT];
extern bool g_key[KeyCode::COUNT];
extern bool g_key_up[KeyCode::COUNT];
extern bool g_key_held[KeyCode::COUNT];
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];

static int get_key_code(int wParam)
{
    int key = -1;

    if(wParam >= 48 && wParam < 48 + 10)
    {
        key = KeyCode::Alpha0 + wParam - 48;
    }
    else if(wParam >= 96 && wParam < 96 + 10)
    {
        key = KeyCode::Keypad0 + wParam - 96;
    }
    else if(wParam >= 65 && wParam < 65 + 'z' - 'a')
    {
        key = KeyCode::A + wParam - 65;
    }
    else if(wParam == VK_BACK)
    {
        key = KeyCode::Backspace;
    }
    else if(wParam == VK_SPACE)
    {
        key = KeyCode::Space;
    }
    else if(wParam == VK_ESCAPE)
    {
        key = KeyCode::Escape;
    }
    else if(wParam == VK_RETURN)
    {
        key = KeyCode::Return;
    }

    return key;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
    case WM_KEYDOWN:
        {
            int key = get_key_code(wParam);

            if(key >= 0)
            {
                if(!g_key_held[key])
                {
                    g_key_down[key] = true;
                    g_key_held[key] = true;
                    g_key[key] = true;
                }
            }
        }
        break;

    case WM_KEYUP:
        {
            int key = get_key_code(wParam);

            if(key >= 0)
            {
                g_key_up[key] = true;
                g_key_held[key] = false;
                g_key[key] = false;
            }
        }
        break;

	case WM_LBUTTONDOWN:
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			if(!g_input_down)
			{
				Touch t;
				t.deltaPosition = Vector2(0, 0);
				t.deltaTime = 0;
				t.fingerId = 0;
				t.phase = TouchPhase::Began;
				t.position = Vector2((float) x, (float) Screen::GetHeight() - y - 1);
				t.tapCount = 1;
				t.time = GTTime::GetRealTimeSinceStartup();

				if(!g_input_touches.empty())
				{
					g_input_touch_buffer.push_back(t);
				}
				else
				{
					g_input_touches.push_back(t);
				}

				g_input_down = true;
			}

            g_mouse_button_down[0] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
            g_mouse_button_held[0] = true;
		}
		break;

    case WM_RBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_down[1] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
            g_mouse_button_held[1] = true;
        }
        break;

    case WM_MBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_down[2] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
            g_mouse_button_held[2] = true;
        }
        break;

	case WM_MOUSEMOVE:
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			if(g_input_down)
			{
				Touch t;
				t.deltaPosition = Vector2(0, 0);
				t.deltaTime = 0;
				t.fingerId = 0;
				t.phase = TouchPhase::Moved;
				t.position = Vector2((float) x, (float) Screen::GetHeight() - y - 1);
				t.tapCount = 1;
				t.time = GTTime::GetRealTimeSinceStartup();

				if(!g_input_touches.empty())
				{
					if(g_input_touch_buffer.empty())
					{
						g_input_touch_buffer.push_back(t);
					}
					else
					{
						if(g_input_touch_buffer.back().phase == TouchPhase::Moved)
						{
							g_input_touch_buffer.back() = t;
						}
						else
						{
							g_input_touch_buffer.push_back(t);
						}
					}
				}
				else
				{
					g_input_touches.push_back(t);
				}
			}

            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
		}
		break;

	case WM_LBUTTONUP:
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			if(g_input_down)
			{
				Touch t;
				t.deltaPosition = Vector2(0, 0);
				t.deltaTime = 0;
				t.fingerId = 0;
				t.phase = TouchPhase::Ended;
				t.position = Vector2((float) x, (float) Screen::GetHeight() - y - 1);
				t.tapCount = 1;
				t.time = GTTime::GetRealTimeSinceStartup();

				if(!g_input_touches.empty())
				{
					g_input_touch_buffer.push_back(t);
				}
				else
				{
					g_input_touches.push_back(t);
				}

				g_input_down = false;
			}

            g_mouse_button_up[0] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
            g_mouse_button_held[0] = false;
		}
		break;

    case WM_RBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_up[1] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
            g_mouse_button_held[1] = false;
        }
        break;

    case WM_MBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            g_mouse_button_up[2] = true;
            g_mouse_position.x = (float) x;
            g_mouse_position.y = (float) Screen::GetHeight() - y - 1;
            g_mouse_button_held[2] = false;
        }
        break;

    case WM_SETCURSOR:
        {
            auto cursor = (HCURSOR) Cursor::GetCursor(Cursor::GetCurent());
            if(cursor == NULL)
            {
                cursor = g_cursor_default;
            }

            SetCursor(cursor);
        }
        break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
