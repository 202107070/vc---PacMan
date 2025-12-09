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
    if (!InitInstance(hInstance, nCmdShow))
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

    return (int)msg.wParam;
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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PACMAN));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PACMAN);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
        CW_USEDEFAULT, 0, 1260, 820, nullptr, nullptr, hInstance, nullptr);

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
//RECT g_p1, g_p2; // 포탈 객체(서로 연결)
//RECT g_food; // 음식 객체
//BOOL g_myFlag; // 내 상태 확인
//BOOL g_yourFlag; // 적 상태 확인
RECT tileRect; // 맵 타일 블록

int g_speed = 10;

int g_me_direction = 1; // WM_KEYDOWN에서 팩맨의 다음 이동 방향을 저장할 변수
int g_you_direction = 1; // 유령의 현재 방향 (0:좌, 1:우, 2:상, 3:하)
RECT g_initialMe, g_initialYou; // 나와 적 캐릭터 초기 위치
bool g_is_invincible = false; // 무적 상태를 저장할 변수

// 타이머 ID
#define GAME_LOOP_TIMER_ID    1
#define RESET_TIMER_ID        2   
#define READY_TIMER_ID        3
#define LEVEL_CLEAR_TIMER_ID  4

// 게임 상태 정의
enum GameState {
    STATE_MENU,
    STATE_READY,
    STATE_PLAYING,
    STATE_LEVEL_CLEAR,
    STATE_RESETTING,
    STATE_GAME_OVER
};

// 게임 변수
int g_game_state = STATE_MENU;  // 초기 상태: 메뉴 (또는 시작 대기)
int g_score = 0;                // 현재 점수 (좌측 상단)
int g_life = 3;                 // 남은 목숨 (좌측 하단)
int g_level = 1;                // 현재 레벨 (좌측 상단)
int g_food_count = 0;           // 남은 일반 먹이 수


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

// 먹이 종류 정의 (food_map 필요)
// 0: 빈 공간, 1: 일반 먹이(10점), 2: 슈퍼 먹이(100점)
int food_map[ROWS][COLS];

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
    {3, 0, 0, 0, 4, 4, 4, 4, 4, 4, 0, 2, 0, 2, 0, 4, 4, 4, 4, 4, 4, 0, 0, 3}, // <--- 포탈 주변 4

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
    {1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1},

    // i==12 : 외곽 하단 벽
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1} };

// 통로(obs[i][j] == 0) 중 랜덤한 위치에 캐릭터를 배치하고, 맵에 먹이를 채우는 함수
void PlaceCharactersAndFood(int level)
{
    // 1. 맵 초기화 및 먹이 채우기
    g_food_count = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // ⚠️ obs 배열의 값을 food_map으로 복사하며 카운트 증가
            if (obs[i][j] == 4 || obs[i][j] == 5) {
                food_map[i][j] = obs[i][j]; // 4 또는 5 복사
                g_food_count++;
            }
            else {
                food_map[i][j] = 0; // 통로나 벽은 0
            }
        }
    }
    g_me = g_initialMe;
    g_you = g_initialYou;
}

// 게임 시작/레벨 시작 시 호출
void RunGameReadySequence(HWND hWnd, int next_level)
{
    g_level = next_level;

    // 1. 맵 초기화 및 먹이 채우기 (food_map 업데이트)
    PlaceCharactersAndFood(g_level);

    // 2. g_hMemDC 초기화 및 맵 요소 다시 그리기 (WM_CREATE 로직 재활용)
    HDC hdc = GetDC(hWnd);

    // DC 상태 저장 및 브러시 정의
    HBRUSH hBlueBrush = CreateSolidBrush(RGB(0, 0, 255));
    HBRUSH hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH hFoodBrush = CreateSolidBrush(RGB(250, 224, 212));
    HBRUSH hSFoodBrush = CreateSolidBrush(RGB(250, 224, 212));
    HPEN hYellowpen = CreatePen(PS_SOLID, 5, RGB(255, 228, 0)); // 게이트용 펜

    RECT fullMemRect = { 0, 0, 1260, 820 };
    HBRUSH hBgBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(g_hMemDC, &fullMemRect, hBgBlackBrush); // 배경 전체 검은색으로 지우기
    DeleteObject(hBgBlackBrush);

    // 💥 DC 초기 펜/브러시 상태를 명확히 저장
    HPEN hOldPen = (HPEN)SelectObject(g_hMemDC, GetStockObject(NULL_PEN));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(g_hMemDC, GetStockObject(NULL_BRUSH));

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int x = groundBlock_left + (j * TILE_SIZE);
            int y = groundBlock_top + (i * TILE_SIZE);
            RECT tileRect = { x, y, x + TILE_SIZE, y + TILE_SIZE };
            int centerX = x + TILE_SIZE / 2;
            int centerY = y + TILE_SIZE / 2;

            if (obs[i][j] == 1) { // 벽
                FillRect(g_hMemDC, &tileRect, hBlueBrush);
            }
            else if (obs[i][j] == 2) { // 게이트 (노란색 테두리)
                FillRect(g_hMemDC, &tileRect, hBlackBrush);

                // 게이트 테두리 그리기
                SelectObject(g_hMemDC, hYellowpen);
                Rectangle(g_hMemDC, x, y, x + TILE_SIZE, y + TILE_SIZE);
                SelectObject(g_hMemDC, hOldPen); // 펜 복원
            }

            // 먹이 다시 그리기 (food_map 기반)
            if (food_map[i][j] == 4 || food_map[i][j] == 5) {
                // ... (중심 좌표 및 radius 계산 유지) ...
                int radius = (food_map[i][j] == 4) ? 4 : 12;
                HBRUSH currentBrush = (food_map[i][j] == 4) ? hFoodBrush : hSFoodBrush;

                // 먹이 그리기
                SelectObject(g_hMemDC, currentBrush);
                SelectObject(g_hMemDC, GetStockObject(NULL_PEN)); // 펜은 사용하지 않음
                Ellipse(g_hMemDC, centerX - radius, centerY - radius,
                    centerX + radius, centerY + radius);

                // 브러시와 펜 복원
                SelectObject(g_hMemDC, hOldBrush);
                SelectObject(g_hMemDC, hOldPen);
            }
        }
    }

    // 💥 최종 DC 상태 복원
    SelectObject(g_hMemDC, hOldBrush);
    SelectObject(g_hMemDC, hOldPen);

    // 객체 해제
    DeleteObject(hBlueBrush);
    DeleteObject(hBlackBrush);
    DeleteObject(hFoodBrush);
    DeleteObject(hSFoodBrush);
    DeleteObject(hYellowpen);
    ReleaseDC(hWnd, hdc);

    // 기존 타이머 제거
    KillTimer(hWnd, GAME_LOOP_TIMER_ID);

    // 상태 변경 및 Ready 타이머 시작 (2초)
    g_game_state = STATE_READY;
    SetTimer(hWnd, READY_TIMER_ID, 2000, NULL); // 2초 후 READY_TIMER_ID 발동

    InvalidateRect(hWnd, NULL, TRUE); // 화면 전체 갱신
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
    {
        if (g_game_state == STATE_MENU)
        {
            // g_game_state가 STATE_MENU일 때 마우스 클릭으로 시작
            RunGameReadySequence(hWnd, 1);
        }
        // ⚠️ 마우스 클릭으로 재시작을 방지합니다.
        else if (g_game_state == STATE_GAME_OVER) {
            // 게임 오버 상태를 유지합니다. (재시작 로직 없음)
        }
    }
    break;

    case WM_KEYDOWN:
    {
        // 게임 오버 치트키 처리 (VK_F1)
        if (wParam == VK_F1 && g_game_state == STATE_GAME_OVER)
        {
            // 목숨을 3으로 초기화하고 레벨 1로 바로 READY 상태 시작
            g_life = 3;
            g_score = 0; // 점수도 초기화 (일반적인 치트키 역할)
            RunGameReadySequence(hWnd, 1);
            return 0; // 즉시 반환
        }

        // F2 키를 눌러 무적 상태 토글
        if (wParam == VK_F2)
        {
            g_is_invincible = !g_is_invincible; // 현재 상태를 반전 (true <-> false)
            return 0;
        }

        // 게임 상태가 STATE_PLAYING일 때만 키 입력을 처리합니다.
        if (g_game_state != STATE_PLAYING) {
            return 0;
        }

        switch (wParam) {
        case VK_UP:
            g_me_direction = 2; // UP
            break;
        case VK_DOWN:
            g_me_direction = 3; // DOWN
            break;
        case VK_RIGHT:
            g_me_direction = 1; // RIGHT
            break;
        case VK_LEFT:
            g_me_direction = 0; // LEFT
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

        // 맵 초기화 및 먹이 배열 생성
        PlaceCharactersAndFood(g_level); // 레벨 1로 초기화

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
        // 1. 리셋 타이머 처리 (ID 2)
        if (wParam == RESET_TIMER_ID)
        {
            KillTimer(hWnd, RESET_TIMER_ID);
            // 목숨이 남아있으면 READY 상태로, 아니면 게임 오버 상태로 전환해야 함
            if (g_life > 0) {
                RunGameReadySequence(hWnd, g_level); // 현재 레벨을 그대로 유지하고 READY 상태로
            }
            else {
                g_game_state = STATE_GAME_OVER;
                // 메인 루프 타이머가 혹시라도 살아있다면 종료 (안전 조치)
                KillTimer(hWnd, GAME_LOOP_TIMER_ID);
                InvalidateRect(hWnd, NULL, TRUE);
            }
            return 0;
        }

        // 2. READY 타이머 처리 (2초 후 게임 시작)
        if (wParam == READY_TIMER_ID)
        {
            KillTimer(hWnd, READY_TIMER_ID);
            g_game_state = STATE_PLAYING;
            SetTimer(hWnd, GAME_LOOP_TIMER_ID, 50, NULL); // 메인 게임 루프 시작
            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }

        // 3. 레벨 클리어 타이머 처리 (2초 후 다음 레벨 시작)
        if (wParam == LEVEL_CLEAR_TIMER_ID)
        {
            KillTimer(hWnd, LEVEL_CLEAR_TIMER_ID);
            if (g_food_count <= 0) {                  // 먹이가 모두 사라지면
                RunGameReadySequence(hWnd, g_level + 1); // 다음 레벨로 전환
            }
            return 0;
        }

        // 2. 일반 게임 루프 (ID 1) 및 유령 이동
        if (wParam == GAME_LOOP_TIMER_ID && g_game_state == STATE_PLAYING)
        {
            RECT g_oldYou = g_you;
            RECT g_oldMe = g_me; // 팩맨의 이전 위치 저장


            // **********************************************
        // 2-1. 팩맨 이동 시도
        // **********************************************
            switch (g_me_direction) {
            case 0: g_me.left -= g_speed; g_me.right -= g_speed; break;
            case 1: g_me.left += g_speed; g_me.right += g_speed; break;
            case 2: g_me.top -= g_speed; g_me.bottom -= g_speed; break;
            case 3: g_me.top += g_speed; g_me.bottom += g_speed; break;
            }

            // 2-2. 팩맨 벽 충돌 검사
            bool is_me_colliding = false;
            for (int i = 0; i < ROWS; i++) {
                for (int j = 0; j < COLS; j++) {
                    if (obs[i][j] == 1 || obs[i][j] == 2) {
                        int x = groundBlock_left + (j * TILE_SIZE);
                        int y = groundBlock_top + (i * TILE_SIZE);
                        RECT wallRect = { x, y, x + TILE_SIZE, y + TILE_SIZE };
                        RECT overlap_result;
                        if (IntersectRect(&overlap_result, &g_me, &wallRect)) {
                            is_me_colliding = true;
                            break;
                        }
                    }
                }
            }
            if (is_me_colliding) {
                g_me = g_oldMe; // 충돌 시 되돌림
            }

            // **********************************************
            // 2-3. 먹이 섭취 및 점수 업데이트 (점수 활성화 로직)
            // **********************************************

            bool portal_warped = false; // 실제 기능은 없지만 명시적 선언

            for (int i = 0; i < ROWS; i++) {
                for (int j = 0; j < COLS; j++) {
                    if (obs[i][j] == 4 || obs[i][j] == 5 || obs[i][j] == 3) // 4, 5, 3 검사
                    {
                        // 현재 타일의 화면 RECT 좌표 계산
                        int x = groundBlock_left + (j * TILE_SIZE);
                        int y = groundBlock_top + (i * TILE_SIZE);

                        int radius = (obs[i][j] == 4) ? 4 : 12; // 일반 먹이 4, 슈퍼 먹이 12
                        int centerX = x + TILE_SIZE / 2;
                        int centerY = y + TILE_SIZE / 2;

                        // 먹이 전용의 작은 RECT 정의 (IntersectRect 판정 영역)
                        RECT foodRect_precise = {
                            centerX - radius,
                            centerY - radius,
                            centerX + radius,
                            centerY + radius
                        };

                        RECT overlap_result;

                        // 캐릭터와 먹이 충돌 검사
                        if (IntersectRect(&overlap_result, &g_me, &foodRect_precise))
                        {
                            // 1. 포탈 처리 (3)
                            if (obs[i][j] == 3)
                            {
                                // 포탈 워프 로직
                                int target_col = (j == 0) ? (COLS - 1) : 0;
                                if (target_col == 0) {
                                    target_col += 1; // 좌측 포탈(j=0) 도착 시, 다음 칸(j=1)으로 이동
                                }
                                else {
                                    target_col -= 1; // 우측 포탈(j=COLS-1) 도착 시, 이전 칸(j=COLS-2)으로 이동
                                }

                                int newX_center = groundBlock_left + target_col * TILE_SIZE + TILE_SIZE / 2;
                                int newY_center = groundBlock_top + i * TILE_SIZE + TILE_SIZE / 2;

                                int char_size = g_me.right - g_me.left;
                                g_me.left = newX_center - char_size / 2;
                                g_me.right = newX_center + char_size / 2;
                                g_me.top = newY_center - char_size / 2;
                                g_me.bottom = newY_center + char_size / 2;
                            }
                            // 2. 먹이 처리 (4, 5)
                            else if (obs[i][j] == 4 || obs[i][j] == 5)
                            {
                                if (food_map[i][j] != 0)
                                {
                                    // 점수 증가
                                    if (obs[i][j] == 4) {
                                        g_score += 10;  // 일반 먹이
                                    }
                                    else if (obs[i][j] == 5) {
                                        g_score += 100; // 슈퍼 먹이
                                    }

                                    // 먹이 제거
                                    food_map[i][j] = 0;   // food_map에서도 제거
                                    g_food_count--;       // 남은 먹이 수 감소

                                    // 화면에서 먹이 제거
                                    RECT tileRect = { x, y, x + TILE_SIZE, y + TILE_SIZE };
                                    HBRUSH hClearBrush = CreateSolidBrush(RGB(0, 0, 0));
                                    FillRect(g_hMemDC, &tileRect, hClearBrush);
                                    DeleteObject(hClearBrush);
                                }
                            }
                        }
                    }
                }
                if (portal_warped) break; // 명시적으로 포탈 워프 선언
            }

            // 2-4. 레벨 클리어 체크
            if (g_food_count <= 0) {
                // 레벨 클리어가 완료되었으면, 게임 상태를 STATE_LEVEL_CLEAR로 변경하고 타이머를 설정
                if (g_game_state != STATE_LEVEL_CLEAR) {  // 이미 레벨 클리어 상태인 경우 중복 실행 방지
                    KillTimer(hWnd, GAME_LOOP_TIMER_ID);  // 게임 루프 타이머 중지
                    g_game_state = STATE_LEVEL_CLEAR;    // 상태를 LEVEL_CLEAR로 설정
                    SetTimer(hWnd, LEVEL_CLEAR_TIMER_ID, 2000, NULL); // 2초 후 다음 레벨로 이동
                    InvalidateRect(hWnd, NULL, TRUE);  // 화면 갱신
                }
                return 0;
            }

            // **********************************************
            // 1. AI 방향 결정 및 탐색 로직
            // **********************************************

            // 현재 중심 좌표
            int currentX_center = g_you.left + (g_you.right - g_you.left) / 2; 
            int currentY_center = g_you.top + (g_you.bottom - g_you.top) / 2;

            int half_tile = TILE_SIZE / 2; // 타일 중심 값
            // 타일 내 오프 셋(유령의 중심이 현재 타일 내에서 어디에 위치한 지 알려줌)
            int tile_x_offset = (currentX_center - groundBlock_left) % TILE_SIZE;
            int tile_y_offset = (currentY_center - groundBlock_top) % TILE_SIZE;

            // 타일 중앙에 근접했을 때만 (오차 3픽셀) AI를 재계산합니다.
            if (abs(tile_x_offset - half_tile) < 3 && abs(tile_y_offset - half_tile) < 3)
            {
                //현재 타일 인덱스
                int current_col = (currentX_center - groundBlock_left) / TILE_SIZE;
                int current_row = (currentY_center - groundBlock_top) / TILE_SIZE;

                // 팩맨 과의 거리
                int dX = g_me.left - g_you.left;
                int dY = g_me.top - g_you.top;

                // 직선 목표 방향
                int target_dir;
                if (abs(dX) >= abs(dY)) {
                    target_dir = (dX > 0) ? 1 : 0; // X축 우선
                }
                else {
                    target_dir = (dY > 0) ? 3 : 2; // Y축 우선
                }

                int primary_dir = target_dir; // 1순위 목표
                // 수직 우회 방향(막혔을 때 코너를 돌 수 있도록)
                int secondary_dir_1 = (primary_dir == 0 || primary_dir == 1) ? 2 : 0; // 수직 1
                int secondary_dir_2 = (primary_dir == 0 || primary_dir == 1) ? 3 : 1; // 수직 2

                // 탐색 순서: {현재 방향, 목표 방향, 수직 1, 수직 2}
                int try_dirs[4] = { g_you_direction, primary_dir, secondary_dir_1, secondary_dir_2 };

                // 최적 경로 추적
                int best_dir = -1; // 가장 최적의 방향 저장
                float min_dist_sq = 1e9; // 가장 짧은 거리 저장
                int targetX = g_me.left + (g_me.right - g_me.left) / 2;
                int targetY = g_me.top + (g_me.bottom - g_me.top) / 2;

                for (int k = 0; k < 4; ++k)
                {
                    int dir = try_dirs[k];
                    int opposite_dir = (g_you_direction + 2) % 4;

                    // 180도 회전 및 중복 시도 방지 (현재 방향과 목표가 같을 때는 제외)
                    if (dir == opposite_dir && dir != g_you_direction) continue;

                    // 다음 타일 예측
                    int next_row = current_row;
                    int next_col = current_col;

                    if (dir == 0) next_col -= 1; else if (dir == 1) next_col += 1;
                    else if (dir == 2) next_row -= 1; else if (dir == 3) next_row += 1;

                    // 벽 충돌 검사 (obs[row][col] == 1)
                    if (next_row >= 0 && next_row < ROWS && next_col >= 0 && next_col < COLS && obs[next_row][next_col] != 1)
                    {
                        // 이동 가능! 팩맨과의 거리 계산
                        int next_X = groundBlock_left + next_col * TILE_SIZE + half_tile;
                        int next_Y = groundBlock_top + next_row * TILE_SIZE + half_tile;
                        float dist_sq = (float)(next_X - targetX) * (next_X - targetX) + (float)(next_Y - targetY) * (next_Y - targetY);

                        // 최적 방향 선택 및 갱신
                        if (dist_sq < min_dist_sq)
                        {
                            min_dist_sq = dist_sq;
                            best_dir = dir;
                        }
                    }
                }

                // 최적 방향으로 유령의 방향 업데이트
                if (best_dir != -1) {
                    g_you_direction = best_dir;
                }
            }

            // **********************************************
            // 3. 유령 이동 및 충돌 검사 (벽)
            // **********************************************

            // 3-1. 유령 좌표 업데이트
            switch (g_you_direction)
            {
            case 0: g_you.left -= g_speed; g_you.right -= g_speed; break;
            case 1: g_you.left += g_speed; g_you.right += g_speed; break;
            case 2: g_you.top -= g_speed; g_you.bottom -= g_speed; break;
            case 3: g_you.top += g_speed; g_you.bottom += g_speed; break;
            }

            // 3-2. 벽 충돌 검사 (IntersectRect 기반 순회는 생략하고, 중심점 타일 검사로 간소화)
            int checkX = g_you.left + (g_you.right - g_you.left) / 2;
            int checkY = g_you.top + (g_you.bottom - g_you.top) / 2;
            int col = (checkX - groundBlock_left) / TILE_SIZE;
            int row = (checkY - groundBlock_top) / TILE_SIZE;

            bool is_colliding_wall = false;

            // 맵 경계 밖이거나 obs[row][col] == 1 (벽)이면 충돌
            if (row < 0 || row >= ROWS || col < 0 || col >= COLS || obs[row][col] == 1) {
                is_colliding_wall = true;
            }

            if (is_colliding_wall)
            {
                g_you = g_oldYou; // 충돌 시, 이전 위치로 되돌림
            }

            // **********************************************
            // 4. 팩맨과의 충돌 검사 (리셋 로직)
            // **********************************************
            RECT ret;
            if (IntersectRect(&ret, &g_me, &g_you))
            {
                // 무적 상태 확인
                if (g_is_invincible)
                {
                    // 무적 상태이므로 아무것도 하지 않고 함수 종료 (충돌 무시)
                    InvalidateRect(hWnd, NULL, FALSE);
                    return 0;
                }

                g_life--; // 목숨 감소
                KillTimer(hWnd, GAME_LOOP_TIMER_ID);

                if (g_life <= 0) { // ⚠️ 목숨이 0 이하인 경우 게임 오버
                    // 🚨 목숨이 0이 되면 즉시 게임 오버로 최종 전환
                    g_game_state = STATE_GAME_OVER;
                }
                else {
                    g_game_state = STATE_RESETTING;
                    SetTimer(hWnd, RESET_TIMER_ID, 3000, NULL); // 3초 후 리셋
                }
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

        if (g_game_state == STATE_PLAYING || g_game_state == STATE_RESETTING || g_game_state == STATE_READY || g_game_state == STATE_LEVEL_CLEAR)
        {
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
        }
        else if (g_game_state == STATE_GAME_OVER)
        {
            HFONT hFont;
            SetTextColor(hdc, RGB(255, 0, 0));
            hFont = CreateFont(64, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS, L"Arial");
            SelectObject(hdc, hFont);

            TextOut(hdc, 500, 350, L"GAME OVER!", 10);
            DeleteObject(hFont);
        }
        // ========================= [점수, 레벨 출력 (좌측 상단)] =========================

        // 폰트 설정
        HFONT hFont, hOldFont;
        hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Arial");
        hOldFont = (HFONT)SelectObject(hdc, hFont);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255)); // 노란색 텍스트

        // 1. 레벨 표시 (점수 위에)
        wchar_t level_text[32];
        wsprintf(level_text, L"LEVEL %d", g_level);
        TextOut(hdc, 20, 10, level_text, lstrlen(level_text));

        // 2. 점수 표시
        wchar_t score_text[32];
        wsprintf(score_text, L"SCORE: %d", g_score);
        TextOut(hdc, 20, 35, score_text, lstrlen(score_text));

        // ========================= [목숨 표시 (좌측 하단)] =========================
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 0)); // 노란색 원
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        for (int i = 0; i < g_life; i++) {
            // 목숨을 작은 팩맨 모양(노란 동그라미)으로 표시 (좌측 하단)
            int x = 20 + i * 25;
            int y = 720; // 대략적인 좌측 하단 위치
            Ellipse(hdc, x, y, x + 20, y + 20);
        }

        DeleteObject(hBrush);
        DeleteObject(hPen);

        // ========================= [레벨 표시 (우측 하단)] =========================
        hPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
        hBrush = CreateSolidBrush(RGB(0, 255, 0)); // 초록색 원
        hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        hOldPen = (HPEN)SelectObject(hdc, hPen);

        // 현재 레벨을 초록색 동그라미로 표시 (우측 하단)
        for (int i = 0; i < g_level; i++) {
            int x = 1220 - (i + 1) * 25;
            int y = 720; // 대략적인 우측 하단 위치
            Ellipse(hdc, x, y, x + 20, y + 20);
        }

        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hBrush);
        DeleteObject(hPen);

        // ========================= [상태 텍스트 출력 (중앙)] =========================

        // Ready! 텍스트 및 Level Clear 텍스트
        if (g_game_state == STATE_READY)
        {
            SetTextColor(hdc, RGB(255, 255, 0));
            hFont = CreateFont(48, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS, L"Arial");
            SelectObject(hdc, hFont);

            TextOut(hdc, 570, 400, L"READY!", 6);
            DeleteObject(hFont);
        }
        else if (g_game_state == STATE_LEVEL_CLEAR)
        {
            SetTextColor(hdc, RGB(0, 255, 255));
            hFont = CreateFont(48, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS, L"Arial");
            SelectObject(hdc, hFont);

            TextOut(hdc, 500, 400, L"LEVEL CLEAR!", 12);
            DeleteObject(hFont);
        }

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
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