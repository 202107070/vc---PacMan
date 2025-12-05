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
BOOL g_myFlag; // 내 상태 확인
BOOL g_yourFlag; // 적 상태 확인
RECT tileRect; // 맵 타일 블록

int g_speed = 10;

int g_you_direction = 1; // 유령의 현재 방향 (0:좌, 1:우, 2:상, 3:하)
RECT g_initialMe, g_initialYou; // 나와 적 캐릭터 초기 위치
bool g_isResetting = false; // 리셋 여부
UINT_PTR g_resetTimerID = 2; // 리셋타이머 아이디 : 2
//struct blocks
//{
//    RECT blocksize;
//};
//
//struct blocks myblockArray[20];

#define TILE_SIZE 50 // 타일 하나 당 크기
#define COLS 24 // 24행
#define ROWS 13 // 13열
#define groundBlock_left  20 // 맵 블록 출발지
#define groundBlock_top  50 // 맵 블록 출발지

HDC g_hMemDC = NULL;      // 메모리 DC (그림을 그릴 가상 캔버스)
HBITMAP g_hBitmap = NULL; // 메모리 DC에 선택할 비트맵 (실제 그림 데이터)
HBITMAP g_hOldBitmap = NULL; // 원래 비트맵을 저장할 변수 (해제 시 필요)

int obs[ROWS][COLS] = {// i=0 : 외곽 상단 벽
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

    // i=1 : 상단 통로 (슈퍼 먹이 코너)
    {1, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 1}, // <--- i=1 양 끝에 5

    // i=2 : 벽
    {1, 4, 1, 4, 1, 1, 4, 1, 1, 4, 1, 1, 1, 1, 4, 1, 1, 4, 1, 1, 4, 1, 4, 1},

    // i=3 : 통로
    {1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 1},

    // i=4 : 중앙 구조 (유령 감옥 진입로)
    {1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 0, 2, 2, 2, 0, 1, 1, 1, 1, 1, 4, 1, 1, 1},

    // i=5 : 유령 감옥 및 포탈
    {3, 0, 0, 0, 4, 4, 4, 4, 4, 4, 0, 2, 0, 2, 0, 4, 4, 4, 4, 4, 0, 0, 0, 3}, // <--- 포탈 주변 4

    // i=6 : 유령 게이트가 있는 행
    {1, 1, 1, 1, 4, 1, 1, 4, 1, 1, 0, 2, 2, 2, 0, 1, 1, 4, 1, 1, 4, 1, 1, 1},

    // i=7 : 유령 감옥 하단
    {1, 4, 4, 4, 4, 1, 1, 4, 1, 1, 0, 0, 0, 0, 0, 1, 1, 4, 4, 4, 4, 4, 4, 1},

    // i=8 : 통로
    {1, 4, 1, 1, 1, 1, 1, 4, 4, 4, 4, 1, 1, 1, 4, 4, 4, 4, 1, 1, 1, 1, 4, 1},

    // i=9 : 통로 및 구조 (슈퍼 먹이 코너)
    {1, 5, 4, 4, 4, 4, 4, 4, 1, 1, 4, 4, 4, 4, 4, 1, 1, 4, 4, 4, 4, 4, 5, 1}, // <--- i=9 양 끝에 5

    // i=10 : 하단 코너 벽
    {1, 4, 1, 4, 1, 1, 4, 1, 1, 1, 4, 1, 1, 1, 4, 1, 1, 1, 4, 1, 1, 1, 4, 1},

    // i=11 : 통로
    {1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1},

    // i==12 : 외곽 하단 벽
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1} };

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
    {
        // 내 캐릭터 이전 위치 저장
        RECT g_oldMe = g_me;

        switch (wParam) {
        case VK_UP:
        {
            g_me.top -= g_speed;
            g_me.bottom -= g_speed;
        }
        break;

        case VK_DOWN:
        {
            g_me.top += g_speed;
            g_me.bottom += g_speed;
        }
        break;

        case VK_RIGHT:
        {
            g_me.left += g_speed;
            g_me.right += g_speed;
        }
        break;

        case VK_LEFT:
        {
            g_me.left -= g_speed;
            g_me.right -= g_speed;
        }
        break;
        }

        //for (int i = 0; i < 10; i++)
        //{
        //    RECT ret;
        //    //if (IntersectRect(&ret, &g_me, &myblockArray[i].blocksize))
        //    {
        //        g_myFlag = TRUE;
        //        break;
        //    }
        //}

        // 2. 맵 인덱스 계산 및 충돌 검사

        bool is_colliding = false;

        // 맵 전체를 순회하면서 벽 타일의 RECT와 캐릭터를 비교합니다.
        for (int i = 0; i < ROWS; i++) // 행 순회
        {
            for (int j = 0; j < COLS; j++) // 열 순회
            {
                // 2-1. 현재 타일이 벽(1) 또는 게이트(2)인지 확인
                if (obs[i][j] == 1 || obs[i][j] == 2)
                {
                    // 2-2. 현재 타일의 화면 RECT 좌표 계산
                    int x = groundBlock_left + (j * TILE_SIZE);
                    int y = groundBlock_top + (i * TILE_SIZE);

                    // 현재 벽 타일의 RECT 구조체 선언
                    RECT wallRect = { x, y, x + TILE_SIZE, y + TILE_SIZE };

                    RECT overlap_result;

                    // 2-3. 캐릭터와 벽 타일의 겹침을 IntersectRect로 검사
                    if (IntersectRect(&overlap_result, &g_me, &wallRect))
                    {
                        // 겹치는 부분이 발견되면 충돌!
                        is_colliding = true;
                        break; // 충돌했으므로 더 이상 타일을 검사할 필요 없음
                    }
                }
            }
            if (is_colliding) break;
        }

        // 3. 충돌 처리
        if (is_colliding)
        {
            g_me = g_oldMe; // 충돌 시 원래 위치로 되돌림
        }
        else
        {
            bool portal_warped = false; // 포탈 이동 여부를 추적할 플래그

            // 맵 전체를 순회하며 먹이(4, 5)와 포탈(3)을 검사하고 처리합니다.
            for (int i = 0; i < ROWS; i++)
            {
                for (int j = 0; j < COLS; j++)
                {
                    if (obs[i][j] == 4 || obs[i][j] == 5 || obs[i][j] == 3) // 4, 5, 3 검사
                    {
                        // 4-2. 현재 타일의 화면 RECT 좌표 계산
                        int x = groundBlock_left + (j * TILE_SIZE);
                        int y = groundBlock_top + (i * TILE_SIZE);
                        
                        // 3. 먹이의 중심 및 크기
                        int radius = (obs[i][j] == 4) ? 4 : 12; // 일반 먹이 4, 슈퍼 먹이 12
                        int centerX = x + TILE_SIZE / 2;
                        int centerY = y + TILE_SIZE / 2;

                        // 4. 먹이 전용의 작은 RECT 정의 (IntersectRect 판정 영역)
                        RECT foodRect_precise = {
                            centerX - radius,
                            centerY - radius,
                            centerX + radius,
                            centerY + radius
                        };

                        RECT overlap_result;

                        if (IntersectRect(&overlap_result, &g_me, &foodRect_precise))
                        {
                            // 1. 포탈 처리 (3)
                            if (obs[i][j] == 3)
                            {
                                // 포탈 워프 로직 (이전에 주석 처리했던 코드 복원)
                                int target_col = (j == 0) ? (COLS - 1) : 0;

                                // 1. 통로 타일의 인덱스 설정
                                if (target_col == 0) {
                                    // 좌측 포탈(j=0) 도착 시, 다음 칸(j=1)으로 이동
                                    target_col += 1;
                                }
                                else {
                                    // 우측 포탈(j=COLS-1) 도착 시, 이전 칸(j=COLS-2)으로 이동
                                    target_col -= 1;
                                }

                                int newX_center = groundBlock_left + target_col * TILE_SIZE + TILE_SIZE / 2;
                                int newY_center = groundBlock_top + i * TILE_SIZE + TILE_SIZE / 2;

                                int char_size = g_me.right - g_me.left;
                                g_me.left = newX_center - char_size / 2;
                                g_me.right = newX_center + char_size / 2;
                                g_me.top = newY_center - char_size / 2;
                                g_me.bottom = newY_center + char_size / 2;

                                portal_warped = true; // 플래그 설정
                                break; // 내부 j 루프 탈출
                            }

                            // 2. 먹이 처리 (4, 5) (포탈이 아닐 경우)
                            else if (obs[i][j] == 4 || obs[i][j] == 5)
                            {
                                obs[i][j] = 0;

                                RECT tileRect = { x, y, x + TILE_SIZE, y + TILE_SIZE };

                                HBRUSH hClearBrush = CreateSolidBrush(RGB(0, 0, 0));
                                FillRect(g_hMemDC, &tileRect, hClearBrush);
                                DeleteObject(hClearBrush);
                            }
                        }
                    }
                }
                if (portal_warped) break; // 플래그가 설정되면 외부 i 루프 탈출
            }
        }
        InvalidateRect(hWnd, NULL, FALSE);
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
        // 일반 먹이 그리기
        HBRUSH hFoodBrush = CreateSolidBrush(RGB(250, 224, 212));
        // 슈퍼 먹이 그리기
        HBRUSH hSFoodBrush = CreateSolidBrush(RGB(250, 224, 212));


        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++)
            {
                int x = groundBlock_left + (j * TILE_SIZE);
                int y = groundBlock_top + (i * TILE_SIZE);
                RECT tileRect = { x, y, x + TILE_SIZE, y + TILE_SIZE };

                int centerX = x + TILE_SIZE / 2;
                int centerY = y + TILE_SIZE / 2;


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
                else if (obs[i][j] == 4)
                {
                    int foodsize = 4;

                    SelectObject(g_hMemDC, hFoodBrush);
                    Ellipse(g_hMemDC, centerX - foodsize, centerY - foodsize,
                        centerX + foodsize, centerY + foodsize);
                }
                else if (obs[i][j] == 5)
                {
                    int Sfoodsize = 12;

                    SelectObject(g_hMemDC, hSFoodBrush);
                    Ellipse(g_hMemDC, centerX - Sfoodsize, centerY - Sfoodsize,
                        centerX + Sfoodsize, centerY + Sfoodsize);
                }
            }
        }
        DeleteObject(hBgBlackBrush);
        DeleteObject(hBlueBrush);
        DeleteObject(hBlackBrush);
        DeleteObject(hFoodBrush);
        DeleteObject(hSFoodBrush);


        SetTimer(hWnd, 1, 50, NULL);
        /*int rect_values[14][4] = { 
        };
       
        for (int i = 0; i < 14; i++) {
                myblockArray[i].blocksize.left = rect_values[i][0];
                myblockArray[i].blocksize.top = rect_values[i][1];
                myblockArray[i].blocksize.right = rect_values[i][2];
                myblockArray[i].blocksize.bottom = rect_values[i][3];
        }*/
        
        // 나와 적의 크기를 40으로 설정
        int char_size = 40;
        int tile_offset = (TILE_SIZE - char_size) / 2;

        // 내 캐릭터 초기 위치 설정
        g_me.left = groundBlock_left + 12 * TILE_SIZE + tile_offset;  
        g_me.top = groundBlock_top + 11 * TILE_SIZE + tile_offset;
        g_me.right = g_me.left + char_size;
        g_me.bottom = g_me.top + char_size;

        // 적 캐릭터 초기 위치 설정
        g_you.left = groundBlock_left + 12 * TILE_SIZE + tile_offset;
        g_you.top = groundBlock_top + 5 * TILE_SIZE + tile_offset;
        g_you.right = g_you.left + char_size;
        g_you.bottom = g_you.top + char_size;

        g_initialMe = g_me;
        g_initialYou = g_you;

        SetTimer(hWnd, 1, 50, NULL);
        /*g_p1.left = 20;
        g_p1.top = 280;
        g_p1.right = 50;
        g_p1.bottom = 350;

        g_p2.left = 1370;
        g_p2.top = 280;
        g_p2.right = 1400;
        g_p2.bottom = 350;*/
    }
    break;

   

    case WM_TIMER:
    {
        // 리셋 타이머 처리 (ID 2)는 그대로 유지

        if (wParam == 1 && !g_isResetting)
        {
            RECT g_oldYou = g_you; // 이전 위치 저장 (복원용)

            // **********************************************
            // 1. 유령의 AI 목표 방향 결정 및 이동 방향 설정
            // **********************************************
            int dX = g_me.left - g_you.left;
            int dY = g_me.top - g_you.top;

            int target_dir; // 팩맨을 향해 가야 할 방향

            // 팩맨이 유령보다 왼쪽에 있으면 왼쪽(0) 시도
            if (dX < 0) {
                target_dir = 0; // LEFT
            }
            // 팩맨이 유령보다 오른쪽에 있으면 오른쪽(1) 시도
            else if (dX > 0) {
                target_dir = 1; // RIGHT
            }
            // 팩맨이 유령보다 위쪽에 있으면 위(2) 시도
            else if (dY < 0) {
                target_dir = 2; // UP
            }
            // 팩맨이 유령보다 아래쪽에 있으면 아래(3) 시도
            else {
                target_dir = 3; // DOWN
            }

            // * 핵심: 팩맨을 향하는 방향(target_dir)으로 이동을 시도합니다. *
            g_you_direction = target_dir;

            // 2. 유령 좌표 업데이트 (현재 AI가 결정한 방향으로 이동)
            switch (g_you_direction)
            {
            case 0: g_you.left -= g_speed; g_you.right -= g_speed; break;
            case 1: g_you.left += g_speed; g_you.right += g_speed; break;
            case 2: g_you.top -= g_speed; g_you.bottom -= g_speed; break;
            case 3: g_you.top += g_speed; g_you.bottom += g_speed; break;
            }

        
            bool is_colliding_wall = false;

            for (int i = 0; i < ROWS; i++) // 행 순회
            {
                for (int j = 0; j < COLS; j++) // 열 순회
                {
                    // 1. 벽(1) 또는 게이트(2) 타일만 검사 (유령은 2를 통과해야 함)
                    if (obs[i][j] == 1) // ⚠️ 1(벽)만 검사하도록 수정
                    {
                        int x = groundBlock_left + (j * TILE_SIZE);
                        int y = groundBlock_top + (i * TILE_SIZE);
                        RECT wallRect = { x, y, x + TILE_SIZE, y + TILE_SIZE };

                        RECT overlap_result;

                        if (IntersectRect(&overlap_result, &g_you, &wallRect))
                        {
                            is_colliding_wall = true;
                            break;
                        }
                    }
                }
                if (is_colliding_wall) break;
            }

            // 2-4. 맵 경계 검사 추가 (유령이 맵 밖으로 완전히 나가는 것을 방지)
            if (g_you.left < groundBlock_left || g_you.right > groundBlock_left + (COLS * TILE_SIZE) ||
                g_you.top < groundBlock_top || g_you.bottom > groundBlock_top + (ROWS * TILE_SIZE))
            {
                // 맵 전체 영역을 벗어나는 경우
                is_colliding_wall = true;
            }

            // 3-1. 충돌 처리 및 AI 방향 재설정
            if (is_colliding_wall)
            {
                g_you = g_oldYou; // ⚠️ 충돌 시, 이전 위치로 되돌림

                // AI 방향 재설정: 벽에 갇히지 않도록 랜덤 방향을 찾습니다.
                int new_dir;
                do {
                    new_dir = rand() % 4;
                } while (new_dir == g_you_direction); // ⚠️ 막힌 방향은 일단 피함

                g_you_direction = new_dir; // 다음 턴에 이 랜덤 방향을 시도
            }

            // **********************************************
            // 4. 팩맨과의 충돌 검사 (리셋 로직)
            // **********************************************
            RECT ret;
            if (IntersectRect(&ret, &g_me, &g_you))
            {
                // 충돌 발생: 리셋 준비
                KillTimer(hWnd, 1);
                g_isResetting = true;
                SetTimer(hWnd, 2, 3000, NULL); // 3초 (3000ms) 후에 리셋
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

            // 내 캐릭터 그리기
            HBRUSH hYellowBrush = CreateSolidBrush(RGB(255, 255, 0));
            SelectObject(hdc, hYellowBrush);
            Ellipse(hdc, g_me.left, g_me.top, g_me.right, g_me.bottom);
            DeleteObject(hYellowBrush);

            // 적 캐릭터 그리기
            HBRUSH hRedBrush = CreateSolidBrush(RGB(255, 0, 0));
            SelectObject(hdc, hRedBrush);
            Ellipse(hdc, g_you.left, g_you.top, g_you.right, g_you.bottom);
            DeleteObject(hRedBrush);

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
