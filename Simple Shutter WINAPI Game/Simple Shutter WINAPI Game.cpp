#include <iostream>
#include <stdlib.h>
#include <windows.h>

// macros for creating enemies
#define AddEnemy(a,b) ObjectInit(NewObject(), a, b, 40, 40, 'e')

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

// offset point
TPoint cam;

// data type for object
typedef struct SObject
{
   TPoint pos;
   TPoint size;
   COLORREF brush;
   TPoint speed;
   char oType;  // type of object: 'e' - enemy, 'p' - player, 'b' - bullet
   float range, vecSpeed; // distance of bullet shoot and the vector speed
   BOOL isDel; // need delete an object or not
} TObject, *PObject;

RECT rct;
TObject player;

// array of enemies object
PObject mas = NULL;
int masCnt = 0;
BOOL needNewGame = FALSE;


// function to check the crossing of two objects
BOOL ObjectCollision(TObject o1, TObject o2)
{
   return ((o1.pos.x + o1.size.x) > o2.pos.x) && (o1.pos.x < (o2.pos.x + o2.size.x)) &&
          ((o1.pos.y + o1.size.y) > o2.pos.y) && (o1.pos.y < (o2.pos.y + o2.size.y));
}

void ObjectInit(TObject* obj, float xPos, float yPos, float width, float height, char objType)
{
   obj->pos = point(xPos, yPos);
   obj->size = point(width, height);
   obj->brush = RGB(0, 255, 0);
   obj->speed = point(0, 0);
   obj->oType = objType;
   // enemies are red color
   if (objType == 'e') obj->brush = RGB(255, 0, 0);

   obj->isDel = FALSE;
}


PObject NewObject()
{
   masCnt++;
   mas = static_cast<PObject>(realloc(mas, sizeof(*mas) * masCnt));
   return mas + masCnt - 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
   shape(dc, (int)(obj.pos.x - cam.x), (int)(obj.pos.y - cam.y), (int)(obj.pos.x + obj.size.x - cam.x), (int)(obj.pos.y + obj.size.y - cam.y));
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
   // bullet speed
   obj->vecSpeed = vecSpeed;
}

// procedure for generating enemies
void GenNewEnemy()
{
   static int rad = 300;
   int pos1 = (rand() % 2 == 0 ? -rad : rad);
   int pos2 = (rand() % (rad * 2)) - rad;
   int k = rand() % 100;
   if (k == 1)
      AddEnemy(player.pos.x + pos1, player.pos.y + pos2);
   if (k == 2)
      AddEnemy(player.pos.x + pos2, player.pos.y + pos1);

}

//procedure to delete the object from the array
void DelObjects()
{
   int i = 0;
   while (i < masCnt)
   {
      if (mas[i].isDel)
      {
         masCnt--;
         mas[i] = mas[masCnt];
         mas = static_cast<PObject>(realloc(mas, sizeof(*mas) * masCnt));
      }
      else
         i++;
   }
}

void AddBullet(float xPos, float yPos, float x, float y)
{
   PObject obj = NewObject();
   ObjectInit(obj, xPos, yPos, 10, 10, 'b');
   ObjectSetDestPoint(obj, x, y, 4);
   obj->range = 300;
}

// procedure to set the focus on object
void SetCameraFocus(TObject obj)
{
   cam.x = obj.pos.x - rct.right / 2;
   cam.y = obj.pos.y - rct.bottom / 2;
}

//procedure to move an object
void ObjectMove(TObject* obj)
{
   // enemies correct their direction 1 per 40 time
   if (obj->oType == 'e')
   {
      if (rand() % 40 == 1)
      {
         static float enemySpeed = 1.3;
         ObjectSetDestPoint(obj, player.pos.x, player.pos.y, enemySpeed);
      }
      if (ObjectCollision(*obj, player))
         needNewGame = TRUE;
   }
      
   // player movement
   obj->pos.x += obj->speed.x;
   obj->pos.y += obj->speed.y;

   // bullet movement
   if (obj->oType == 'b')
   {
      obj->range -= obj->vecSpeed;
      // when bullet distance is max - delete the bullet
      if (obj->range < 0) 
         obj->isDel = TRUE;

      // if bullet cross the enemy == bullet and enemy are deleting
      for (int i = 0; i < masCnt; i++)
      {
         if ((mas[i].oType == 'e') && (ObjectCollision(*obj, mas[i])))
         {
            mas[i].isDel = TRUE;
            obj->isDel = TRUE;
         }
      }
   }
}

// control the speed of the player
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

////////////////////////////////////////////////////////////////////////////////////////////////////

// to initialize the character and enemies
void WinInit()
   {
   needNewGame = FALSE;
   masCnt = 0;
   //when the new level starts the memory will be cleaned
   mas = static_cast<PObject>(realloc(mas, 0));

   ObjectInit(&player,100,100,40,40,'p');
   }

void WinMove()
{
   if (needNewGame)
      WinInit();

   PlayerControl();
   ObjectMove(&player);
   SetCameraFocus(player);

   // enemies movement
   for (int i = 0; i < masCnt; i++)
      ObjectMove(mas + i);

   GenNewEnemy();
   DelObjects();
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

   else if (message == WM_LBUTTONDOWN)
   {
      int xPos = LOWORD(lparam);
      int yPos = HIWORD(lparam);
      AddBullet(player.pos.x + player.size.x / 2, player.pos.y + +player.size.y / 2, xPos + cam.x, yPos + cam.y);
   }

   else return DefWindowProcA(hwnd, message, wparam, lparam);
}

void WinShow(HDC dc)
{
   HDC memDC = CreateCompatibleDC(dc);
   HBITMAP memBM = CreateCompatibleBitmap(dc, rct.right - rct.left, rct.bottom - rct.top);
   SelectObject(memDC, memBM);

   SelectObject(memDC, GetStockObject(DC_PEN));
   SetDCPenColor(memDC, RGB(255, 255, 255));
   SelectObject(memDC, GetStockObject(DC_BRUSH));
   SetDCBrushColor(memDC, RGB(200, 200, 200));

   // create the background
   static int rectSize = 200; // the size of square
   int dx = (int)(cam.x) % rectSize; // offset of beginning to draw the square
   int dy = (int)(cam.y) % rectSize;
   // fill the background by gray squares
   for (int i = -1; i < (rct.right / rectSize) + 2; i++)
   {
      for (int j = -1; j < (rct.bottom / rectSize) + 2; j++)
      {
         Rectangle(memDC, -dx + (i * rectSize), -dy + (j * rectSize), -dx + ((i + 1) * rectSize), -dy + ((j + 1) * rectSize));
      }
   }


   ObjectShow(player, memDC);
   for (int i = 0; i < masCnt; i++)
   {
      ObjectShow(mas[i], memDC);
   }

   BitBlt(dc, 0, 0, rct.right - rct.left, rct.bottom - rct.top, memDC, 0, 0, SRCCOPY);

   
   DeleteDC(memDC);
   DeleteObject(memBM);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
   WNDCLASSA wcl;
   memset(&wcl, 0, sizeof(WNDCLASSA));
   wcl.lpszClassName = "MyWindow";
   wcl.lpfnWndProc = WndProc;
   wcl.hCursor = LoadCursor(NULL,IDC_CROSS);
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

