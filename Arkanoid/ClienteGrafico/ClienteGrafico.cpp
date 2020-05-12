// ClienteGrafico.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ClienteGrafico.h"
#include "Windowsx.h"
#include <mmsystem.h>
#include <Commctrl.h>

#define MAX_LOADSTRING 100
#define WIDTH 1600
#define HEIGHT 900
#define SCALE 10

int spectate = 0;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Login(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	JoinGame(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	ConfigureKeys(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	Top10(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


void loadDefaultKeys(void);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	if (createMutexs() == FAILURE) {
		return FAILURE;
	}
	loadDefaultKeys();

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CLIENTEGRAFICO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTEGRAFICO));

	MSG msg;

	// Main message loop:
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENTEGRAFICO));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(18, 65, 120));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENTEGRAFICO);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		200, 100, WIDTH, HEIGHT, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	static HDC hdc = NULL;
	static HDC auxDC = NULL;
	static HBITMAP auxBM = NULL;
	static int nX = 0, nY = 0;

	static int xBefore = 0;

	// Ball
	static HBITMAP hBall; // Carregar o mapa
	//static HBITMAP hMaskBall; // Carregar o mapa
	static BITMAP bmBall; // associar as dimensoes
	//static BITMAP bmMaskBall; // associar as dimensoes
	static HDC hdcBall;
	//static HDC hdcMaskBall;

	// Background Normal
	static HBITMAP hBackground; // Carregar o mapa
	static BITMAP bmBackground; // associar as dimensoes

	// Background Aguardar
	static HBITMAP hBackground_Wait; // Carregar o mapa

	// Background Jogo
	static HBITMAP hBackgroundGame; // Carregar o mapa

	// Background Vitoria
	static HBITMAP hBackground_Win; // Carregar o mapa

	// Background Derrota
	static HBITMAP hBackground_Lose; // Carregar o mapa

	static HDC hdcBackground;
	static RECT gameLimits;
	RECT brickLimits;
	RECT paddleLimits;
	RECT paddleInfo;
	static HBRUSH rectFrame = (HBRUSH)CreateSolidBrush(RGB(255, 255, 255)); // TODO mudar cor
	HBRUSH brickFrame;
	static HBRUSH paddleFrame[GAME_MAX_PLAYERS];

	gameLimits.left = 0;
	gameLimits.top = 0;
	gameLimits.right = 0;
	gameLimits.bottom = 0;

	switch (message)
	{
	case WM_CREATE:
	{
		Answer answer = PingServer();
		if (answer.type == ANSWER_PING_SERVER_ONLINE)
			MessageBox(hWnd, TEXT("The server is online!"), TEXT("Server Status"), MB_OK);
		else {
			if (MessageBox(hWnd, TEXT("The server is disconnected or hasn't responded. Please try again later!"), TEXT("Login"), MB_OK) == IDOK)
				DestroyWindow(hWnd);
		}

		// OBTEM AS DIMENSOES DO DISPLAY... 
		nX = GetSystemMetrics(SM_CXSCREEN);
		nY = GetSystemMetrics(SM_CYSCREEN);

		hdc = GetDC(hWnd);
		auxDC = CreateCompatibleDC(hdc);
		auxBM = CreateCompatibleBitmap(hdc, nX, nY);
		SelectObject(auxDC, auxBM);
		PatBlt(auxDC, 0, 0, nX, nY, PATCOPY);

		// CARREGA 'BITMAP' BACKGROUND
		hBackgroundGame = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BKG_GAME), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		GetObject(hBackgroundGame, sizeof(bmBackground), &bmBackground);
		hBackground_Win = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BG_WIN), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		GetObject(hBackground_Win, sizeof(bmBackground), &bmBackground);
		hBackground_Lose = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BG_LOSE), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		GetObject(hBackground_Lose, sizeof(bmBackground), &bmBackground);
		hBackground_Wait = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BG_WAIT), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		GetObject(hBackground_Wait, sizeof(bmBackground), &bmBackground);
		hBackground = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BACKGROUND), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		GetObject(hBackground, sizeof(bmBackground), &bmBackground);

		hdcBackground = CreateCompatibleDC(hdc);
		SelectObject(hdcBackground, hBackground);

		hBall = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BALL), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		GetObject(hBall, sizeof(bmBall), &bmBall);
		hdcBall = CreateCompatibleDC(hdc);
		SelectObject(hdcBall, hBall);

		game.win = -1;

		paddleFrame[0] = (HBRUSH)CreateSolidBrush(RGB(255, 128, 0));
		paddleFrame[1] = (HBRUSH)CreateSolidBrush(RGB(255, 255, 0));
		paddleFrame[2] = (HBRUSH)CreateSolidBrush(RGB(255, 0, 0));
		paddleFrame[3] = (HBRUSH)CreateSolidBrush(RGB(128, 255, 0));
		paddleFrame[4] = (HBRUSH)CreateSolidBrush(RGB(0, 255, 128));
		paddleFrame[5] = (HBRUSH)CreateSolidBrush(RGB(0, 255, 255));
		paddleFrame[6] = (HBRUSH)CreateSolidBrush(RGB(0, 128, 255));
		paddleFrame[7] = (HBRUSH)CreateSolidBrush(RGB(0, 0, 255));
		paddleFrame[8] = (HBRUSH)CreateSolidBrush(RGB(127, 0, 255));
		paddleFrame[9] = (HBRUSH)CreateSolidBrush(RGB(255, 0, 255));
		paddleFrame[10] = (HBRUSH)CreateSolidBrush(RGB(255, 0, 127));
		paddleFrame[11] = (HBRUSH)CreateSolidBrush(RGB(128, 128, 128));
		paddleFrame[12] = (HBRUSH)CreateSolidBrush(RGB(0, 255, 0));

		SetTextColor(auxDC, RGB(255, 255, 255));

		//hMaskBall = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_MASKINV4), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		//GetObject(hMaskBall, sizeof(bmMaskBall), &bmMaskBall);
		//hdcMaskBall = CreateCompatibleDC(hdc);
		//SelectObject(hdcMaskBall, hMaskBall);

		ReleaseDC(hWnd, hdc);
		PlaySound(TEXT("sounds\\bkmusic_intro.wav"), NULL, SND_ASYNC | SND_LOOP);
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (game.gameActive) {
			int action = -1;
			int xNew = GET_X_LPARAM(lParam);

			if (!spectate)
				if (xBefore > xNew) {
					action = ACTION_MOVE_LEFT;
				}
				else if (xBefore < xNew) {
					action = ACTION_MOVE_RIGHT;
				}

			if (action != -1) {
				SendRequest(configs.username, action);
				xBefore = xNew;
			}
		}
	}
	break;
	case WM_CHAR:
	{
		if (game.gameActive) {
			int action = -1;

			if (!spectate)
				if (wParam == configs.move_left) {
					action = ACTION_MOVE_LEFT;
				}
				else if (wParam == configs.move_right) {
					action = ACTION_MOVE_RIGHT;
				}

			if (wParam == DEFAULT_KEY_QUIT) {
				action = ACTION_QUIT_GAME;
			}
			if (action != -1)
				SendRequest(configs.username, action);
		}
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_LOGIN:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGINFORM), hWnd, Login);
			break;
		case ID_FILE_JOINGAME:
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_JOINGAMEBOX), hWnd, JoinGame);
			Answer answer = JoinGame(configs.username);
			if (answer.type == ANSWER_JOIN_GAME_TO_SPECTATE)
				spectate = 1;
			//SelectObject(hdcBackground, hBackground_Wait);
			//InvalidateRect(hWnd, NULL, FALSE);
			WaitForGameToBegin();
			PlaySound(TEXT("sounds\\bkmusic_gameactive.wav"), NULL, SND_ASYNC | SND_LOOP);
			break;
		case ID_FILE_HIGHSCORE:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_TOP10), hWnd, Top10);
			break;
		case ID_SETTINGS_CONFIGUREKEYS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_CONFIGUREKEYS), hWnd, ConfigureKeys);
			break;
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
		// LIMPA FUNDO...
		PatBlt(auxDC, 0, 0, nX, nY, PATCOPY);
		WaitForSingleObject(hMutexData, INFINITE);
		if (game.gameActive) {
			SelectObject(hdcBackground, hBackgroundGame);
		}
		else {
			if (game.win == 1) {
				SelectObject(hdcBackground, hBackground_Win);
			}
			else if (game.win == 0) {
				SelectObject(hdcBackground, hBackground_Lose);
			}
			else {
				SelectObject(hdcBackground, hBackground);
			}
		}

		StretchBlt(auxDC, 0, 0, WIDTH, HEIGHT, hdcBackground, 0, 0, bmBackground.bmWidth, bmBackground.bmHeight, SRCCOPY);
		SetBkMode(auxDC, TRANSPARENT);

		if (game.gameActive) {
			SelectObject(hdcBackground, hBackgroundGame);
			SetBkMode(auxDC, TRANSPARENT);
			StretchBlt(auxDC, 0, 0, WIDTH, HEIGHT, hdcBackground, 0, 0, bmBackground.bmWidth, bmBackground.bmHeight, SRCCOPY);

			int x = 15;
			int y = 0;
			int space = 18;
			TCHAR temp[50];

			/*for (int j = 0; j < 13; j++)
			{*/
			for (int i = 0; i < game.configs.max_players; i++) {
				if (game.players[i].valid && game.players[i].playing && !game.players[i].spectating) {
					TextOut(auxDC, x, y + space, TEXT("Paddle: "), 8);
					paddleInfo.left = x + 55;
					paddleInfo.top = y + space;
					paddleInfo.right = x + 95;
					paddleInfo.bottom = y + 2 * space;
					FillRect(auxDC, &paddleInfo, paddleFrame[i]);
					_stprintf_s(temp, 50, TEXT("Lives: %d"), game.players[i].lives);
					TextOut(auxDC, x, y + 2 * space, temp, _tcslen(temp));
					_stprintf_s(temp, 50, TEXT("Score: %04d"), game.players[i].score);
					TextOut(auxDC, x, y + 3 * space, temp, _tcslen(temp));
					x += 120;
				}
			}
			//}


			// Desenha os limites do jogo
			gameLimits.left = WIDTH / 2 - (game.board.x_dimension * SCALE) / 2;
			gameLimits.top = HEIGHT / 2 - (game.board.y_dimension * SCALE) / 2;
			gameLimits.right = WIDTH / 2 + (game.board.x_dimension * SCALE) / 2;
			gameLimits.bottom = HEIGHT / 2 + (game.board.y_dimension * SCALE) / 2;
			FrameRect(auxDC, &gameLimits, rectFrame);

			// Desenhar a bola
			// TODO substituir por bitmap
			Ellipse(auxDC,
				gameLimits.left + (game.board.ball.coords.x * SCALE) - SCALE,
				gameLimits.top + (game.board.ball.coords.y * SCALE) - SCALE,
				gameLimits.left + (game.board.ball.coords.x * SCALE) + SCALE,
				gameLimits.top + (game.board.ball.coords.y * SCALE) + SCALE);
			/*StretchBlt(auxDC,
				gameLimits.left + game.board.ball.coords.x,
				gameLimits.top + game.board.ball.coords.y,
				20, 20, hdcBall, 0, 0, bmBall.bmWidth, bmBall.bmHeight, SRCPAINT);*/
				// Desenhar paddle(s)
				// TODO substituir por bitmaps
			for (int i = 0; i < game.configs.max_players; i++) {
				if (game.players[i].valid && game.players[i].playing && !game.players[i].spectating) {
					paddleLimits.left = gameLimits.left + (game.players[i].paddle.coords.x * SCALE);
					paddleLimits.top = gameLimits.top + (game.players[i].paddle.coords.y * SCALE) - SCALE;
					paddleLimits.right = gameLimits.left + ((game.players[i].paddle.coords.x + game.players[i].paddle.width) * SCALE);
					paddleLimits.bottom = gameLimits.top + (game.players[i].paddle.coords.y * SCALE);
					FillRect(auxDC, &paddleLimits, paddleFrame[i]);
				}
			}

			// Desenhar Bricks
			for (int i = 0; i < game.board.numBricks; i++) {
				if (game.board.bricks[i].lifes > 0) {
					brickLimits.left = gameLimits.left + (game.board.bricks[i].coords.x * SCALE);
					brickLimits.top = gameLimits.top + (game.board.bricks[i].coords.y * SCALE) - SCALE;
					brickLimits.right = gameLimits.left + ((game.board.bricks[i].coords.x + game.board.bricks[i].width) * SCALE);
					brickLimits.bottom = gameLimits.top + (game.board.bricks[i].coords.y * SCALE);

					switch (game.board.bricks[i].type) {
					case BRICK_TYPE_NORMAL:
						brickFrame = (HBRUSH)CreateSolidBrush(RGB(0, (int)(255 * (game.board.bricks[i].lifes / (float)game.configs.brick_normal_max_lives)), 0));
						break;
					case BRICK_TYPE_RESISTANT:
						brickFrame = (HBRUSH)CreateSolidBrush(RGB((int)(255 * (game.board.bricks[i].lifes / (float)game.configs.brick_resistant_max_lives)), 0, 0));
						break;
					case BRICK_TYPE_MAGIC:
						brickFrame = (HBRUSH)CreateSolidBrush(RGB(0, 0, (int)(255 * (game.board.bricks[i].lifes / (float)game.configs.brick_magic_max_lives))));
						break;
					default:
						brickFrame = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
						break;
					}

					FillRect(auxDC, &brickLimits, brickFrame);
				}
			}
		}

		ReleaseMutex(hMutexData);

		// COPIA INFORMACAO DO 'DC' EM MEMORIA PARA O DISPLAY... 
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, nX, nY, auxDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		if (configs.username != NULL && game.run)
			SendRequest(configs.username, ACTION_SHUTDOWN);
		PostQuitMessage(0);
		DeleteObject(auxBM);
		DeleteDC(auxDC);
		DeleteObject(hBackground);
		DeleteObject(hBackgroundGame);
		DeleteObject(hBackground_Lose);
		DeleteObject(hBackground_Wait);
		DeleteObject(hBackground_Win);
		DeleteDC(hdcBackground);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
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

INT_PTR CALLBACK Login(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	TCHAR username[MAX_NAME];

	switch (message)
	{
	case WM_INITDIALOG:
		Answer answer = PingServer();
		if (answer.type == ANSWER_PING_SERVER_ONLINE) {
			SetDlgItemTextW(hDlg, IDC_SERVERSTATUS_LABEL, TEXT("The server is online!"));
		}
		else {
			SetDlgItemTextW(hDlg, IDC_SERVERSTATUS_LABEL, TEXT("The server is offline"));
			HWND input = GetDlgItem(hDlg, IDC_LOGIN_EDIT);
			Edit_Enable(input, false);
		}
		ShowWindow(GetDlgItem(hDlg, IDC_IPADDRESS1), FALSE);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		TCHAR message[50];
		TCHAR Buff[500];
		if (LOWORD(wParam) == IDC_LOGIN_OK)
		{
			if (GetDlgItemText(hDlg, IDC_LOGIN_EDIT, username, 25)) {
				if (_tcslen(username) > 0) {
					if (SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, 0, 0) == BST_CHECKED) {
						remote = 1;
						DWORD IPAddress;
						SendMessage(GetDlgItem(hDlg, IDC_IPADDRESS1), IPM_GETADDRESS, 0, (LPARAM)& IPAddress);
						_stprintf_s(Buff, 500, TEXT("%d.%d.%d.%d"), (int)FIRST_IPADDRESS(IPAddress), (int)SECOND_IPADDRESS(IPAddress), (int)THIRD_IPADDRESS(IPAddress), (int)FOURTH_IPADDRESS(IPAddress));
						MessageBox(hDlg, Buff, TEXT("Something"), MB_OK);
					}
					else
						remote = 0;
					_tcscpy_s(configs.username, MAX_NAME, username);
					switch (makeLogin(username, Buff)) {
					case ANSWER_LOGIN_SUCCESS:
						_tcscpy_s(message, 50, TEXT("You have logged in successfully"));
						break;
					case ANSWER_LOGIN_FAILURE:
						_tcscpy_s(message, 50, TEXT("Login failed. Server said NO!"));
						break;
					case ANSWER_REMOTE_COMMUNICATION_FAILURE:
						_tcscpy_s(message, 50, TEXT("Error communicating with remote server."));
						break;
					}
					createThreads();
					if (MessageBox(hDlg, message, TEXT("Login"), MB_OK) == IDOK)
						EndDialog(hDlg, LOWORD(wParam));
				}
				else {
					if (MessageBox(hDlg, TEXT("Login failed. Please try again."), TEXT("Login"), MB_OK) == IDOK)
						EndDialog(hDlg, LOWORD(wParam));
				}
				return (INT_PTR)TRUE;
			}

		}
		else if (LOWORD(wParam) == IDC_LOGIN_CANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_CHECK1) {
			if (HIWORD(wParam) == BN_CLICKED) {
				LRESULT result;
				result = SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, 0, 0);
				if (result == BST_CHECKED) {
					ShowWindow(GetDlgItem(hDlg, IDC_IPADDRESS1), TRUE);
				}
				else {
					ShowWindow(GetDlgItem(hDlg, IDC_IPADDRESS1), FALSE);
				}
				return (INT_PTR)TRUE;
			}
		}

		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK JoinGame(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	Answer answer;

	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_JOINGAME_OK)
		{
			answer = JoinGame(configs.username);
			createThreads();
			WaitForGameToBegin();
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ConfigureKeys(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	TCHAR keyLeft = TEXT('\0');
	TCHAR keyRight = TEXT('\0');
	TCHAR debug[50];

	switch (message)
	{
	case WM_INITDIALOG:
		_sntprintf_s(debug, 50, TEXT("Configs left: %c , configs right: %c"), configs.move_left, configs.move_right);
		MessageBox(hDlg, debug, TEXT("Debug"), MB_OK);
		SetDlgItemText(hDlg, IDC_MOVE_LEFT, &(configs.move_left));
		SetDlgItemText(hDlg, IDC_MOVE_RIGHT, &(configs.move_right));
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			if (GetDlgItemText(hDlg, IDC_MOVE_LEFT, &keyLeft, 1))
				configs.move_left = keyLeft;
			if (GetDlgItemText(hDlg, IDC_MOVE_RIGHT, &keyRight, 1))
				configs.move_right = keyRight;

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Top10(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR info[4096];

	switch (message)
	{
	case WM_INITDIALOG:
		showTopRanking(info);
		SetDlgItemText(hDlg, IDC_TOP10_VIEW, info);
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

void loadDefaultKeys(void) {
	configs.move_right = DEFAULT_KEY_MOVE_RIGHT;
	configs.move_left = DEFAULT_KEY_MOVE_LEFT;
}