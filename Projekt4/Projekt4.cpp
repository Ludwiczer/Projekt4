#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <algorithm>
#include <map>

#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;
using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void update();
void drawElevator(Graphics&);
void spawnPassenger(int floor, int target);
void initFloorY();

constexpr int windowWidth = 750;
constexpr int windowHeight = 600;
constexpr int floorCount = 5;
constexpr int floorHeight = 120;
constexpr int elevatorWidth = 200;
constexpr int elevatorHeight = 100;
constexpr int shaftX = 250;
constexpr int elevatorSpeed = 4;
constexpr int verticalOffset = 50;

int elevatorY;
vector<int> elevatorTargets;

enum class Direction { Up, Down, Idle };
Direction elevatorDirection = Direction::Idle;

struct Passenger {
    int destinationFloor;
};

vector<Passenger> elevatorPassengers;
map<int, vector<Passenger>> waitingPassengers;
map<int, int> floorY;

HWND hwnd;
ULONG_PTR gdiplusToken;

bool isRunning = true;

void spawnPassenger(int floor, int target) {
    waitingPassengers[floor].push_back({ target });
}

void initFloorY() {
    for (int i = 1; i <= floorCount; ++i) {
        floorY[i] = windowHeight - i * floorHeight;
    }
    elevatorY = floorY[1];
}

void update() {
    static int idleCounter = 0;

    if (!elevatorTargets.empty()) {
        int targetFloor = elevatorTargets.front();
        int targetY = floorY[targetFloor];

        if (abs(elevatorY - targetY) <= elevatorSpeed) {
            elevatorY = targetY;

            elevatorPassengers.erase(
                remove_if(elevatorPassengers.begin(), elevatorPassengers.end(),
                    [&](Passenger& p) {
                        return p.destinationFloor == targetFloor;
                    }),
                elevatorPassengers.end());

            auto& queue = waitingPassengers[targetFloor];
            float totalWeight = elevatorPassengers.size() * 70.0f;
            vector<Passenger> entering;

            bool canTakeMore = false;

            for (auto& p : queue) {
                if (totalWeight + 70.0f <= 600.0f) {
                    totalWeight += 70.0f;
                    entering.push_back(p);

                    if (find(elevatorTargets.begin(), elevatorTargets.end(), p.destinationFloor) == elevatorTargets.end() &&
                        p.destinationFloor != targetFloor) {
                        elevatorTargets.push_back(p.destinationFloor);
                    }
                    canTakeMore = true;
                }
                else {
                    canTakeMore = false;
                    break;
                }
            }

            elevatorPassengers.insert(elevatorPassengers.end(), entering.begin(), entering.end());
            queue.erase(queue.begin(), queue.begin() + entering.size());

            if (queue.empty() || !canTakeMore) {
                elevatorTargets.erase(elevatorTargets.begin());
            }

            if (!elevatorTargets.empty()) {
                int nextFloor = elevatorTargets.front();
                elevatorDirection = (floorY[nextFloor] > elevatorY) ? Direction::Up : Direction::Down;
            }
            else {
                elevatorDirection = Direction::Idle;
            }

            idleCounter = 0;
        }

        else {
            if (elevatorY < targetY) elevatorY += elevatorSpeed;
            else elevatorY -= elevatorSpeed;
        }
    }
    else {
        vector<int> newTargets;
        for (int i = 1; i <= floorCount; ++i) {
            if (!waitingPassengers[i].empty()) newTargets.push_back(i);
        }

        if (!newTargets.empty()) {
            int closest = newTargets[0];
            int minDist = abs(floorY[closest] - elevatorY);
            for (int f : newTargets) {
                int dist = abs(floorY[f] - elevatorY);
                if (dist < minDist) {
                    closest = f;
                    minDist = dist;
                }
            }

            elevatorTargets.push_back(closest);
            elevatorDirection = (floorY[closest] > elevatorY) ? Direction::Up : Direction::Down;
        }
        else if (elevatorPassengers.empty()) {
            idleCounter++;
            if (idleCounter > 300) {
                int groundY = floorY[1];
                if (elevatorY < groundY) elevatorY += elevatorSpeed;
                else if (elevatorY > groundY) elevatorY -= elevatorSpeed;
            }
        }
    }
}

void drawElevator(Graphics& g) {
    static FontFamily fontFamily(L"Arial");
    static Font font(&fontFamily, 12);

    SolidBrush bg(Color(255, 255, 255));
    g.FillRectangle(&bg, 0, 0, windowWidth, windowHeight);

    Pen shaftPen(Color(0, 0, 0));
    g.DrawRectangle(&shaftPen, shaftX, 0, elevatorWidth, windowHeight);

    Pen cabinPen(Color(0, 0, 255));
    g.DrawRectangle(&cabinPen, shaftX, elevatorY, elevatorWidth, elevatorHeight);

    SolidBrush brush(Color(0, 0, 0));

    for (int i = 1; i <= floorCount; ++i) {
        int y = floorY[i];

        g.DrawLine(&shaftPen, 0, y + elevatorHeight, windowWidth, y + elevatorHeight);

        wstring floorLabel = to_wstring(i);
        g.DrawString(floorLabel.c_str(), -1, &font, PointF(10, y + 5), &brush);

        for (int t = 1; t <= floorCount; ++t) {
            if (t == i) continue;
            int bx = (i % 2 == 1) ? 60 + (t * 20) : shaftX + elevatorWidth + 10 + (t * 20);
            Rect button(bx, y + 20, 15, 15);
            SolidBrush grayBrush(Color::LightGray);
            g.FillRectangle(&grayBrush, button);
            g.DrawRectangle(&shaftPen, button);
            wstring lbl = to_wstring(t);
            g.DrawString(lbl.c_str(), -1, &font, PointF(bx + 2, y + 20), &brush);
        }

        for (size_t k = 0; k < waitingPassengers[i].size(); ++k) {
            float px = (i % 2 == 1) ? 70 + k * 20 : shaftX + elevatorWidth + 10 + k * 20;
            SolidBrush pBrush(Color::Green);
            g.FillEllipse(&pBrush, (INT)px, (INT)y, 15, 15);
            wstring num = to_wstring(waitingPassengers[i][k].destinationFloor);
            g.DrawString(num.c_str(), -1, &font, PointF(px + 3, y - 18), &brush);
        }
    }

    for (size_t i = 0; i < elevatorPassengers.size(); ++i) {
        float px = shaftX + 5 + i * 20;
        float py = elevatorY + 10;
        SolidBrush pBrush(Color::Blue);
        g.FillEllipse(&pBrush, (INT)px, (INT)py, 15, 15);
        wstring num = to_wstring(elevatorPassengers[i].destinationFloor);
        g.DrawString(num.c_str(), -1, &font, PointF(px + 3, py - 18), &brush);
    }

    const int barWidth = 100;
    const int barHeight = 15;
    int barX = shaftX;
    int barY = elevatorY + elevatorHeight + 5;

    float maxWeight = 600.0f;
    float currentWeight = elevatorPassengers.size() * 70.0f;
    float fillWidth = (currentWeight / maxWeight) * barWidth;
    if (fillWidth > barWidth) fillWidth = barWidth;

    SolidBrush bgBrush(Color::LightGray);
    SolidBrush fillBrush(Color::Green);
    Pen borderPen(Color::Black);

    g.FillRectangle(&bgBrush, barX, barY, barWidth, barHeight);
    g.FillRectangle(&fillBrush, barX, barY, (INT)fillWidth, barHeight);
    g.DrawRectangle(&borderPen, barX, barY, barWidth, barHeight);

    WCHAR loadText[50];
    swprintf(loadText, 50, L"Obciążenie: %.0f / %.0f kg", currentWeight, maxWeight);

    SolidBrush textBrush(Color::Black);
    g.DrawString(loadText, -1, &font, PointF((REAL)barX, (REAL)(barY + barHeight + 2)), &textBrush);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ElevatorSim";

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.hbrBackground = NULL;

    RegisterClass(&wc);

    hwnd = CreateWindow(wc.lpszClassName, L"Symulator Windy", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    initFloorY();

    MSG msg;
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            if (!IsWindow(hwnd)) break;

            update();
            InvalidateRect(hwnd, NULL, FALSE);
            Sleep(16);
        }
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        for (int i = 1; i <= floorCount; ++i) {
            int fy = floorY[i];
            for (int t = 1; t <= floorCount; ++t) {
                if (t == i) continue;
                int bx = (i % 2 == 1) ? 60 + (t * 20) : shaftX + elevatorWidth + 10 + (t * 20);
                Rect button(bx, fy + 20, 15, 15);
                if (x >= button.X && x <= button.X + button.Width && y >= button.Y && y <= button.Y + button.Height) {
                    spawnPassenger(i, t);
                }
            }
        }
        break;
    }
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, windowWidth, windowHeight);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        Graphics g(memDC);

        drawElevator(g);

        BitBlt(hdc, 0, 0, windowWidth, windowHeight, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        isRunning = false;
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
