#include <iostream>
#include <stdlib.h>
#include <windows.h>

RECT rct;

void WinMove()
{
   
}


LRESULT WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
   if (message == WM_DESTROY)
      PostQuitMessage(0);
  
   else if (message == WM_KEYDOWN)
      printf("code = %d\n", wparam);
 
   else if (message == WM_CHAR)
      printf("%c\n", wparam);

   else if (message == WM_SIZE)
      GetClientRect(hwnd, &rct);

   else return DefWindowProcA(hwnd, message, wparam, lparam);
}


void WinShow(HDC dc)
{
  
   HDC memDC = CreateCompatibleDC(dc);
  
   HBITMAP memBM = CreateCompatibleBitmap(dc,
      rct.right - rct.left, rct.bottom - rct.top);

   SelectObject(memDC, memBM);

   SelectObject(memDC, GetStockObject(DC_BRUSH));
   SetDCBrushColor(memDC, RGB(255, 255, 255));
   Rectangle(memDC, 0, 0, 640, 480);

 
   BitBlt(dc, 0, 0, rct.right - rct.left, rct.bottom - rct.top,
      memDC, 0, 0, SRCCOPY);

   
   DeleteDC(memDC);
   DeleteObject(memBM);
}

int main()
{
   WNDCLASSA wcl;
   memset(&wcl, 0, sizeof(WNDCLASSA));
   wcl.lpszClassName = "MyWindow";
   wcl.lpfnWndProc = WndProc;
   RegisterClassA(&wcl);

   HWND hwnd;
   hwnd = CreateWindow(L"MyWindow", L"SHUTTER GAME", WS_OVERLAPPEDWINDOW, 10, 10, 640, 480, NULL, NULL, NULL, NULL);

   HDC dc = GetDC(hwnd);

   ShowWindow(hwnd, SW_SHOWNORMAL);


   MSG msg;
   while (true)
   {
      if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT) break;
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      else
      {
         WinMove();
         WinShow(dc);
         Sleep(5);
      }
   }








   return 0;
}

