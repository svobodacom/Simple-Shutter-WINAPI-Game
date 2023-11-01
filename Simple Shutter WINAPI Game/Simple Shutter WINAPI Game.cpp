#include <iostream>
#include <stdlib.h>
#include <windows.h>


// the point object
typedef struct SPoint
{
   float x, y;
} TPoint;

TPoint point(float x, float y)
{
   TPoint pt;
   pt.x = x;
   pt.y = y;
   return pt;
}
// data type for object
typedef struct SObject
{
   TPoint pos;
   TPoint size;
   COLORREF brush;
   TPoint speed;
   // type of object: 'e' - enemy, 'p' - player
   char oType; 
} TObject, *PObject;

void ObjectInit(TObject* obj, float xPos, float yPos, float width, float height, char objType)
{
   obj->pos = point(xPos, yPos);
   obj->size = point(width, height);
   obj->brush = RGB(0, 255, 0);
   obj->speed = point(0, 0);
   obj->oType = objType;
   // enemies are red color
   if (objType == 'e') obj->brush = RGB(255, 0, 0);
}

// Draw procedure
void ObjectShow(TObject obj, HDC dc)
{
   SelectObject(dc, GetStockObject(DC_PEN));
   SetDCPenColor(dc, RGB(0, 0, 0));
   SelectObject(dc, GetStockObject(DC_BRUSH));
   SetDCBrushColor(dc, obj.brush);
   // draw enemies in form of circle and player as a square
   // define shape as a pointer to function
   BOOL(*shape)(HDC, int, int, int, int);
   shape = obj.oType == 'e' ? Ellipse : Rectangle;
   shape(dc, (int)(obj.pos.x), (int)(obj.pos.y), (int)(obj.pos.x + obj.size.x), (int)(obj.pos.y + obj.size.y));
}

//procedure to define the speed
void ObjectSetDestPoint(TObject *obj, float xPos, float yPos, float vecSpeed)
{
   //calculate the distance between object and destination point
   TPoint xyLen = point(xPos - obj->pos.x, yPos - obj->pos.y);
   // calculate the length by direct line
   float dxy = sqrt(xyLen.x * xyLen.x + xyLen.y * xyLen.y);
   // set the speed on each axis 
   obj->speed.x = xyLen.x / dxy * vecSpeed;
   obj->speed.y = xyLen.y / dxy * vecSpeed;
}

RECT rct;
TObject player;
// array of enemies object
PObject mas = NULL;
int masCnt = 0;

PObject NewObject()
{
   masCnt++;
   mas = static_cast<PObject>(realloc(mas, sizeof(*mas) * masCnt));
   return mas + masCnt - 1;
}

//procedure to move the object
void ObjectMove(TObject* obj)
{
   // enemies correct their direction 1 per 40 time
   if (obj->oType == 'e')
      if (rand() % 40 == 1)
      {
         static float enemySpeed = 1.5;
         ObjectSetDestPoint(obj, player.pos.x, player.pos.y, enemySpeed);
      }

   obj->pos.x += obj->speed.x;
   obj->pos.y += obj->speed.y;
}

// control a speed of the player
void PlayerControl()
{
   static int playerSpeed = 4;
   player.speed.x = 0;
   player.speed.y = 0;
   if (GetKeyState('W') < 0) player.speed.y = -playerSpeed;
   if (GetKeyState('S') < 0) player.speed.y = playerSpeed;
   if (GetKeyState('A') < 0) player.speed.x = -playerSpeed;
   if (GetKeyState('D') < 0) player.speed.x = playerSpeed;
   // if the player moves diagonally - decrease 30% of speed
   if ((player.speed.x != 0) && (player.speed.y != 0))
      player.speed = point(player.speed.x * 0.7, player.speed.y * 0.7);
}

// to initialize the character and enemies
void WinInit()
   {
      ObjectInit(&player,100,100,40,40,'p');
      ObjectInit(NewObject(), 400, 100, 40, 40, 'e');
      ObjectInit(NewObject(), 400, 300, 40, 40, 'e');
   }

void WinMove()
{
   PlayerControl();
   ObjectMove(&player);
   // enemies movement
   for (int i = 0; i < masCnt; i++)
      ObjectMove(mas + i);
   
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
   HBITMAP memBM = CreateCompatibleBitmap(dc, rct.right - rct.left, rct.bottom - rct.top);
   SelectObject(memDC, memBM);

   SelectObject(memDC, GetStockObject(DC_BRUSH));
   SetDCBrushColor(memDC, RGB(255, 255, 255));
   Rectangle(memDC, 0, 0, 640, 480);

   ObjectShow(player, memDC);
   for (int i = 0; i < masCnt; i++)
   {
      ObjectShow(mas[i], memDC);
   }

   BitBlt(dc, 0, 0, rct.right - rct.left, rct.bottom - rct.top, memDC, 0, 0, SRCCOPY);

   
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

   WinInit();

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

