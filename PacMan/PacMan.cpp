// PacMan.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "PacMan.h"
#include <time.h>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PACMAN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PACMAN));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PACMAN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PACMAN);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0,1260, 820, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//

RECT g_background; // 배경 객체
RECT g_me; // 내 캐릭터 객체
RECT g_you; // 적 캐릭터 객체
RECT g_p1, g_p2; // 포탈 객체(서로 연결)
RECT g_food; // 음식 객체
BOOL g_myFlag; 
BOOL g_yourFlag;

//struct blocks
//{
//    RECT blocksize;
//};
//
//struct blocks myblockArray[20];

#define TILE_SIZE 50 // 타일 하나 당 크기
#define COLS 24 // 24행
#define ROWS 13 // 13열
#define yourground_left  20 // 맵 블록 출발지
#define yourground_top  50 // 맵 블록 출발지

HDC g_hMemDC = NULL;      // 메모리 DC (그림을 그릴 가상 캔버스)
HBITMAP g_hBitmap = NULL; // 메모리 DC에 선택할 비트맵 (실제 그림 데이터)
HBITMAP g_hOldBitmap = NULL; // 원래 비트맵을 저장할 변수 (해제 시 필요)

int obs[ROWS][COLS] = { // i=0 : 외곽 상단 벽
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

    // i=1 : 상단 통로 및 코너
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},

    // i=2 : 벽
    {1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1},

    // i=3 : 통로
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},

    // i=4 : 중앙 구조 (유령 감옥 진입로)
    {1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 2, 2, 2, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1},

    // i=5 : 유령 감옥 및 포탈
    {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}, // <--- 포탈 (3)

    // i=6 : 유령 게이트가 있는 행
    {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 2, 2, 2, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1}, // <--- 게이트 (2)

    // i=7 : 유령 감옥 하단
    {1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},

    // i=8 : 통로
    {1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1},

    // i=9 : 통로 및 구조
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},

    // i=10 : 하단 코너 벽
    {1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1},

    // i=11 : 통로
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},

    // i==12 : 외곽 하단 벽
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1} };

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
    {
        g_myFlag = FALSE;

        RECT g_oldMe = g_me;

        switch (wParam) {
        case VK_UP:
        {
            g_me.top -= 10;
            g_me.bottom -= 10;
            if (g_me.top < 50)
            {
                g_me.top = 50;
                g_me.bottom = g_me.top + 50;
            }
        }
        break;

        case VK_DOWN:
        {
            g_me.top += 10;
            g_me.bottom += 10;
            if (g_me.bottom > 650)
            {
                g_me.top = 600;
                g_me.bottom = 650;
            }
        }
        break;

        case VK_RIGHT:
        {
            RECT ret;

            g_me.left += 10;
            g_me.right += 10;
            if (g_me.right > 1220)
            {
                g_me.left = 1350;
                g_me.right = 1400;
                if (IntersectRect(&ret, &g_me, &g_p2))
                {
                    g_p1.left = 20;
                    g_p1.top = 280;
                    g_p1.right = 50;
                    g_p1.bottom = 350;
                    g_me.left = 20;
                    g_me.top = 290;
                    g_me.right = 70;
                    g_me.bottom = 340;
                }
            }
        }
        break;

        case VK_LEFT:
        {
            RECT ret;

            g_me.left -= 10;
            g_me.right -= 10;
            if (g_me.left < 20)
            {
                g_me.left = 20;
                g_me.right = 70;
                if (IntersectRect(&ret, &g_me, &g_p1))
                {
                    g_p2.left = 1370;
                    g_p2.top = 280;
                    g_p2.right = 1400;
                    g_p2.bottom = 350;
                    g_me.left = 1350;
                    g_me.top = 290;
                    g_me.right = 1400;
                    g_me.bottom = 340;
                }
            }
        }
        break;
        }

        for (int i = 0; i < 10; i++)
        {
            RECT ret;
            //if (IntersectRect(&ret, &g_me, &myblockArray[i].blocksize))
            {
                g_myFlag = TRUE;
                break;
            }
        }

        if (TRUE == g_myFlag)
        {
            g_me = g_oldMe;
        }

        InvalidateRect(hWnd, NULL, TRUE);
    }
    break;

    case WM_CREATE:
    {
        g_background.left = 20;
        g_background.top = 50;
        g_background.right = 1220;
        g_background.bottom = 650;

        HDC hdc = GetDC(hWnd);
        g_hMemDC = CreateCompatibleDC(hdc);

        g_hBitmap = CreateCompatibleBitmap(hdc, 1260, 820);
        g_hOldBitmap = (HBITMAP)SelectObject(g_hMemDC, g_hBitmap);
        ReleaseDC(hWnd, hdc);
       
        RECT fullMemRect = { 0, 0, 1260, 820 };
        HBRUSH hBgBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(g_hMemDC, &fullMemRect, hBgBlackBrush);
        

        // 게임 블록(장애물) 색 설정 및 화면에 그리기
        HBRUSH hBlueBrush = CreateSolidBrush(RGB(0, 0, 255));
        // 적 생성 블록 그리기
        HBRUSH hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));

        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++)
            {
                int x = yourground_left + (j * TILE_SIZE);
                int y = yourground_top + (i * TILE_SIZE);
                RECT tileRect = { x, y, x + TILE_SIZE, y + TILE_SIZE };

                if (obs[i][j] == 1)
                {
                    FillRect(g_hMemDC, &tileRect, hBlueBrush);
                }
                else if (obs[i][j] == 2)
                {
                    FillRect(g_hMemDC, &tileRect, hBlackBrush);
                    HPEN hpen = CreatePen(PS_SOLID, 5, RGB(255, 228, 0));

                    HPEN hOldPen = (HPEN)SelectObject(g_hMemDC, hpen);
                    HBRUSH hOldBrush = (HBRUSH)SelectObject(g_hMemDC, GetStockObject(NULL_BRUSH));

                    Rectangle(g_hMemDC, x, y, x + TILE_SIZE, y + TILE_SIZE);

                    SelectObject(g_hMemDC, hOldPen);
                    SelectObject(g_hMemDC, hOldBrush);

                    DeleteObject(hpen);
                }
            }
        }
        DeleteObject(hBgBlackBrush);
        DeleteObject(hBlueBrush);
        DeleteObject(hBlackBrush);

        SetTimer(hWnd, 1, 50, NULL);
        /*int rect_values[14][4] = { 
        };
       
        for (int i = 0; i < 14; i++) {
                myblockArray[i].blocksize.left = rect_values[i][0];
                myblockArray[i].blocksize.top = rect_values[i][1];
                myblockArray[i].blocksize.right = rect_values[i][2];
                myblockArray[i].blocksize.bottom = rect_values[i][3];
        }*/
        
        g_p1.left = 20;
        g_p1.top = 280;
        g_p1.right = 50;
        g_p1.bottom = 350;

        g_p2.left = 1370;
        g_p2.top = 280;
        g_p2.right = 1400;
        g_p2.bottom = 350;


        g_me.left = 675;
        g_me.top = 590;
        g_me.right = 725;
        g_me.bottom = 640;

        g_you.left = 675;
        g_you.top = 280;
        g_you.right = 725;
        g_you.bottom = 330;

        SetTimer(hWnd, 1, NULL, NULL);
    }
    break;

    case WM_TIMER:
    {
        g_yourFlag = FALSE;
        int RanMove = 0;
        int RanMoveTime = 0;


        RECT g_oldYou = g_you;

        if (1 == wParam)
        {
            if (RanMoveTime <= 0)
            {
                RanMove = rand() % 4;

                RanMoveTime = (rand() % 800) + 500;
            }

            switch (RanMove)
            {
            case 0:
                {
                    g_you.left -= 10;
                    g_you.right -= 10;
                }
                break;
            case 1:
                {
                    g_you.left += 10;
                    g_you.right += 10;
                }
                break;
            case 2:
                {
                    g_you.top -= 10;
                    g_you.bottom -= 10;
                }
                break;
            case 3:
                {
                    g_you.top += 10;
                    g_you.bottom += 10;
                }
                break;
            }

            RanMoveTime--;

            for (int i = 0; i < 10; i++)
            {
                RECT ret;
                //if (IntersectRect(&ret, &g_you, &myblockArray[i].blocksize))
                {
                    g_yourFlag = TRUE;
                    break;
                }
            }

            if (TRUE == g_yourFlag)
            {
                g_you = g_oldYou;

                RanMoveTime = 0;
            }

            if (g_you.left < g_background.left)
            {
                g_you.left = g_background.left;
                g_you.right = g_you.left + 50;
            }
            if (g_you.right > g_background.right)
            {
                g_you.right = g_background.right;
                g_you.left = g_you.right - 50;
            }
            if (g_you.top < g_background.top)
            {
                g_you.top = g_background.top;
                g_you.bottom = g_you.top + 50;
            }
            if (g_you.bottom > g_background.bottom)
            {
                g_you.bottom = g_background.bottom;
                g_you.top = g_you.bottom - 50;
            }

            RECT ret;
            if (IntersectRect(&ret, &g_me, &g_you))
            {
                g_me.left = 675;
                g_me.top = 590;
                g_me.right = 725;
                g_me.bottom = 640;

                g_you.left = 675;
                g_you.top = 280;
                g_you.right = 725;
                g_you.bottom = 330;
                
                KillTimer(hWnd, 1);
            }

            InvalidateRect(hWnd, NULL, FALSE);
        }
    }
    break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            // 게임 배경색상 설정 및 배경 그리기
           
            BitBlt(hdc, 0, 0, 1260, 820, g_hMemDC, 0, 0, SRCCOPY);

            /*Rectangle(hdc, g_youground.left, g_youground.top,
                g_youground.right, g_youground.bottom);*/
            /*for (int i = 0; i < 14; i++) {
                Rectangle(hdc, myblockArray[i].blocksize.left, myblockArray[i].blocksize.top,
                    myblockArray[i].blocksize.right, myblockArray[i].blocksize.bottom);
            }*/
            //Rectangle(hdc, g_p1.left, g_p1.top, g_p1.right, g_p1.bottom);
            //Rectangle(hdc, g_p2.left, g_p2.top, g_p2.right, g_p2.bottom);
            //Ellipse(hdc, g_me.left, g_me.top, g_me.right, g_me.bottom);
            //Ellipse(hdc, g_you.left, g_you.top, g_you.right, g_you.bottom);
            
            

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:

        if (g_hMemDC) {
            SelectObject(g_hMemDC, g_hOldBitmap);
            DeleteDC(g_hMemDC);
            DeleteObject(g_hBitmap);
        }

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
