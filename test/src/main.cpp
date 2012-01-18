#include <windows.h> 
#include <iostream>
#include <fstream>

#include <redirect.hpp>


using namespace std;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); 

const char* AppName="HelloWorld";
const char* AppTitle="Title";

HGLRC hRC = NULL;
HDC hDC = NULL;
HWND hWnd = NULL;
HINSTANCE hInstance = NULL;

int WinMain(HINSTANCE hInst,HINSTANCE,LPSTR,int nCmdShow) 
{ 
  RedirectIOToConsole();
  std::cout << "Hello world" << std::endl;
  
  hInstance = hInst;
  
  
  WNDCLASS wc; 
  MSG msg;

  wc.style=CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc=WindowProc; 
  wc.cbClsExtra=0; 
  wc.cbWndExtra=0; 
  wc.hInstance=hInst; 
  wc.hIcon=LoadIcon(NULL,IDI_WINLOGO); 
  wc.hCursor=LoadCursor(NULL,IDC_ARROW); 
  wc.hbrBackground=(HBRUSH)COLOR_WINDOWFRAME; 
  wc.lpszMenuName=NULL; 
  wc.lpszClassName=TEXT(AppName);

  if (!RegisterClass(&wc)) {
	std::cerr << "Failed to register class" << std::endl;
    return 0; 
  }

  hWnd = CreateWindow(TEXT(AppName),TEXT(AppTitle), 
    WS_OVERLAPPEDWINDOW, 
    CW_USEDEFAULT,CW_USEDEFAULT,500,500, 
    NULL,NULL,hInst,NULL); 

  if (!hWnd) {
    std::cerr << "Failed to create window" << std::endl;
    return 0; 
  }

  ShowWindow(hWnd,nCmdShow); 
  UpdateWindow(hWnd); 
  
  std::cout << "Main loop starting" << std::endl;
  
  while (GetMessage(&msg,NULL,0,0)) 
  { 
	std::cout << "Loop iteration" << std::endl;
    TranslateMessage(&msg); 
    DispatchMessage(&msg); 
  }
  
  
  return msg.wParam;
} 

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{ 
  switch (msg) 
  { 
	case WM_CREATE:
	{
	  std::cout << "Creating window" << std::endl;
	  HDC hDC = GetDC(hwnd);
      PIXELFORMATDESCRIPTOR pfd;
	  
	  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	  pfd.nVersion = 1;
	  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	  pfd.iPixelType = PFD_TYPE_RGBA;
	  pfd.cColorBits = 24;
	  pfd.cDepthBits = 32;
	  pfd.iLayerType = PFD_MAIN_PLANE;
	  
	  int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	  if (pixelFormat == 0) {
	    std::cerr << "Failed to choose pixel format" << std::endl;
	  }
	  
	  bool err = SetPixelFormat(hDC, pixelFormat, &pfd);
	  if (!err) {
	    std::cerr << "Failed to set pixel format" << std::endl;
	  }
	  
	  hRC = wglCreateContext(hDC);
	  if (!hRC) {
	    std::cerr << "Failed to create wgl context" << std::endl;
	  }
	  
	  err = wglMakeCurrent(hDC, hRC);
	  if (!err) {
	    std::cerr << "Failed to set wgl context current" << std::endl;
	  }
	  break;
	}
    case WM_PAINT: 
    { 
      PAINTSTRUCT ps; 
      HDC dc; 
      RECT r;
      GetClientRect(hwnd,&r); 
      dc=BeginPaint(hwnd,&ps); 
      DrawText(dc,"Hello World",-1,&r,DT_SINGLELINE|DT_CENTER|DT_VCENTER); 
      EndPaint(hwnd,&ps); 
      break; 
    } 

    case WM_DESTROY: 
      PostQuitMessage(0); 
      break; 

    default: 
      return DefWindowProc(hwnd, msg, wparam, lparam); 
  } 
  return 0; 
} 