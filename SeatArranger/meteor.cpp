#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cmath>
#include"randoms.h"

#define STUNUM 21
using namespace Gdiplus;

char *g_str;
// ==================== 配置参数 ====================
const int METEOR_COUNT = 30;          // 流星数量
const int TRAIL_LENGTH = 50;          // 拖尾长度（轨迹点数）
const int TIMER_ID = 1;               // 定时器ID
const int TIMER_INTERVAL = 30;        // 定时器间隔(ms)
const int TEXT_BOX_WIDTH = 1366;       // 文字区域宽度
const int TEXT_BOX_HEIGHT = 768;       // 文字区域高度

// ==================== 数据结构 ====================
struct Meteor {
    std::vector<PointF> trail;        // 轨迹点（最新在末尾）
    PointF velocity;                  // 速度向量
    Color headColor;                  // 头部颜色

    void init(const Rect& bounds) {
        trail.clear();
        // 随机起始位置（确保在窗口内）
        float x = bounds.X + (float)(mt19937_random() % bounds.Width);
        float y = bounds.Y + (float)(mt19937_random() % bounds.Height);
        trail.push_back(PointF(x, y));
        // 随机速度：范围 -2.5 ～ 2.5，避免水平或垂直方向过快
        float vx = ((mt19937_random() % 100) / 100.0f) * 8.0f - 2.5f;
        float vy = ((mt19937_random() % 100) / 100.0f) * 8.0f - 2.5f;
        velocity = PointF(vx, vy);
        // 暖色头部：橙黄到白
        int r = 200 + mt19937_random() % 56;
        int g = 180 + mt19937_random() % 76;
        int b = 100 + mt19937_random() % 100;
        headColor = Color(255, r, g, b);
    }

    void update(const Rect& bounds) {
        if (trail.empty()) return;
        PointF newPos(trail.back().X + velocity.X,
                      trail.back().Y + velocity.Y);
        bool crossed=false;

        // 边界环绕（无缝循环，保持运动方向）
        if (newPos.X < bounds.X){
            newPos.X = bounds.GetRight() - 1.0f;
            crossed=true;
        }
        else if (newPos.X >= bounds.GetRight()){
            newPos.X = bounds.X;
            crossed=true;
        }

        if (newPos.Y < bounds.Y){
            newPos.Y = bounds.GetBottom() - 1.0f;
            crossed=true;
        }
        else if (newPos.Y >= bounds.GetBottom()){
            newPos.Y = bounds.Y;
            crossed=true;
        }

        if(crossed){
            trail.clear();
            trail.push_back(newPos);
        }
        else
        {
            trail.push_back(newPos);
            if (trail.size() > TRAIL_LENGTH)
            trail.erase(trail.begin());
        }
    }
};

// ==================== 全局变量 ====================
std::vector<Meteor> g_meteors;
char g_textBuffer[400] = "座位分配器。\n点击2查看分配结果。1用于查看更多信息。3用于再次分配。";
ULONG_PTR g_gdiplusToken = 0;
HWND g_hWnd = nullptr;
HDC g_hMemDC = nullptr;          // 内存DC（双缓冲）
HBITMAP g_hMemBitmap = nullptr;
int g_clientWidth = 0, g_clientHeight = 0;

// ==================== 辅助函数 ====================
// 将窄字符转为宽字符（用于GDI+）
WCHAR* CharToWChar(const char* str) {
    int len = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
    WCHAR* wstr = new WCHAR[len];
    MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, len);
    return wstr;
}

// 重新创建内存DC（适应窗口大小变化）
void RecreateMemDC(HWND hWnd) {
    if (g_hMemDC) {
        SelectObject(g_hMemDC, g_hMemBitmap);
        DeleteObject(g_hMemBitmap);
        DeleteDC(g_hMemDC);
    }
    HDC hdc = GetDC(hWnd);
    g_hMemDC = CreateCompatibleDC(hdc);
    g_hMemBitmap = CreateCompatibleBitmap(hdc, g_clientWidth, g_clientHeight);
    SelectObject(g_hMemDC, g_hMemBitmap);
    ReleaseDC(hWnd, hdc);
}

// 初始化所有流星
void InitMeteors(const Rect& bounds) {
    g_meteors.clear();
    for (int i = 0; i < METEOR_COUNT; ++i) {
        Meteor m;
        m.init(bounds);
        g_meteors.push_back(m);
    }
}

// 更新所有流星位置
void UpdateMeteors(const Rect& bounds) {
    for (auto& m : g_meteors) {
        m.update(bounds);
    }
}

// 绘制所有流星（拖尾渐变 + 头部光晕）
void DrawMeteors(Graphics& graphics, const Rect& bounds) {
    Pen pen(Color(255, 255, 255), 8.0f);
    pen.SetStartCap(LineCapRound);
    pen.SetEndCap(LineCapRound);
    for (const auto& m : g_meteors) {
        const auto& trail = m.trail;
        if (trail.size() < 2) continue;

        // 绘制拖尾线段（从尾部到头部颜色渐变）
        for (size_t i = 0; i < trail.size() - 1; ++i) {
            float t = (float)i / (trail.size() - 1);   // 0=尾部, 1=头部
            int r = (int)(80 + 175 * t);
            int g = (int)(40 + 215 * t);
            int b = (int)(20 + 235 * t);
            pen.SetColor(Color(255, r, g, b));
            graphics.DrawLine(&pen, trail[i], trail[i + 1]);
        }

        // 绘制头部光晕
        SolidBrush headBrush(m.headColor);
        graphics.FillEllipse(&headBrush,
                             trail.back().X - 6.0f, trail.back().Y - 6.0f,
                             12.0f, 12.0f);
        // 核心亮点
        SolidBrush coreBrush(Color(255, 255, 255, 220));
        graphics.FillEllipse(&coreBrush,
                             trail.back().X - 3.0f, trail.back().Y - 3.0f,
                             6.0f, 6.0f);
    }
}

// 绘制文字（半透明背景，避免覆盖流星）
void DrawTextOverlay(Graphics& graphics, const Rect& clientRect) {
    // 文字区域（左上角）
    RectF textArea(12.0f, 12.0f,
                   (float)TEXT_BOX_WIDTH, (float)TEXT_BOX_HEIGHT);
    // 半透明黑色背景
    SolidBrush backBrush(Color(0, 0, 0, 0));
    graphics.FillRectangle(&backBrush, textArea);
    // 边框
    Pen borderPen(Color(100, 255, 255, 255), 1.0f);
    graphics.DrawRectangle(&borderPen, textArea);

    // 创建字体（使用系统常见字体，避免加载失败）
    Font font(L"Arial", 30.0f, FontStyleRegular, UnitPoint);
    SolidBrush textBrush(Color(255, 255, 240, 200));

    WCHAR* wtext = CharToWChar(g_textBuffer);
    graphics.DrawString(wtext, -1, &font, textArea, nullptr, &textBrush);
    delete[] wtext;
}

int __cmp(const void *a,const void* b){
    return hwrand()%2?1:-1;
}

char * ArrangeSeat(bool need_to_reopen){
    static FILE *f;
    static char stuname[STUNUM][14], stulist[STUNUM];
    static char lns[6][50];
    static char total_char[301];
    bool _5187_found = false;
    
    if(need_to_reopen) {
        f = fopen("C:/users/namelist.nls", "rb");
        if(f == NULL) return NULL;
        for (int i = 0; i < STUNUM; i++) {
            fscanf(f, "%s", stuname[i]);
        }
        fclose(f);
    }
    
    for (int i = 0; i < STUNUM; i++) {
        stulist[i] = i;
    }
    qsort(stulist, STUNUM, 1, __cmp);
    
    for (int i = 0; i < 5; i++) {
        // 检查当前行是否有四个字的名字
        int four_char_pos = -1;
        for (int j = 0; j < 4; j++) {
            if (strcmp("王杨冰清", stuname[stulist[i*4+j]]) == 0) {
                four_char_pos = j;
                _5187_found = true;
                break;
            }
        }
        
        // 根据是否找到四个字的名字来格式化输出
        if (four_char_pos != -1) {
            switch(four_char_pos) {
                case 0:
                    sprintf(lns[i], "%s\t%s\t\t%s\t\t%s\n", 
                            stuname[stulist[i*4]], stuname[stulist[i*4+1]], 
                            stuname[stulist[i*4+2]], stuname[stulist[i*4+3]]);
                    break;
                case 1:
                    sprintf(lns[i], "%s\t\t%s\t%s\t\t%s\n", 
                            stuname[stulist[i*4]], stuname[stulist[i*4+1]], 
                            stuname[stulist[i*4+2]], stuname[stulist[i*4+3]]);
                    break;
                case 2:
                    sprintf(lns[i], "%s\t\t%s\t\t%s\t%s\n", 
                            stuname[stulist[i*4]], stuname[stulist[i*4+1]], 
                            stuname[stulist[i*4+2]], stuname[stulist[i*4+3]]);
                    break;
                case 3:
                    sprintf(lns[i], "%s\t\t%s\t\t%s\t\t%s\n", 
                            stuname[stulist[i*4]], stuname[stulist[i*4+1]], 
                            stuname[stulist[i*4+2]], stuname[stulist[i*4+3]]);
                    break;
            }
        } else {
            sprintf(lns[i], "%s\t\t%s\t\t%s\t\t%s\n", 
                    stuname[stulist[i*4]], stuname[stulist[i*4+1]], 
                    stuname[stulist[i*4+2]], stuname[stulist[i*4+3]]);
        }
    }
    
    sprintf(lns[5], "\t\t\t\t%s\t\t\n", stuname[stulist[20]]); 
    sprintf(total_char, "%s%s%s%s%s%s", lns[0], lns[1], lns[2], lns[3], lns[4], lns[5]);
    return total_char;
}



// ==================== 窗口过程 ====================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // 获取初始客户区大小
            RECT rect;
            GetClientRect(hWnd, &rect);
            g_clientWidth = rect.right - rect.left;
            g_clientHeight = rect.bottom - rect.top;
            RecreateMemDC(hWnd);

            Rect bounds(0, 0, g_clientWidth, g_clientHeight);
            InitMeteors(bounds);
            SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, nullptr);
            break;
        }

        case WM_SIZE: {
            // 窗口大小改变时重新调整内存DC并重置流星
            g_clientWidth = LOWORD(lParam);
            g_clientHeight = HIWORD(lParam);
            RecreateMemDC(hWnd);

            Rect bounds(0, 0, g_clientWidth, g_clientHeight);
            InitMeteors(bounds);
            InvalidateRect(hWnd, nullptr, FALSE);
            break;
        }

        case WM_ERASEBKGND: {
            // 告诉系统我们会在 WM_PAINT 中完全绘制背景，避免闪烁
            return TRUE;
        }

        case WM_TIMER: {
            Rect bounds(0, 0, g_clientWidth, g_clientHeight);
            UpdateMeteors(bounds);
            InvalidateRect(hWnd, nullptr, FALSE);
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // 在内存DC中绘制所有内容
            Rect bounds(0, 0, g_clientWidth, g_clientHeight);

            // 创建 Graphics 对象（绑定到内存DC）
            Graphics graphics(g_hMemDC);
            graphics.SetSmoothingMode(SmoothingModeAntiAlias);
            graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
            // 1. 黑色背景
            SolidBrush blackBrush(Color(0, 0, 0));
            graphics.FillRectangle(&blackBrush, bounds);

            // 2. 绘制流星
            DrawMeteors(graphics, bounds);

            // 3. 绘制文字
            DrawTextOverlay(graphics, bounds);

            // 将内存DC内容一次性拷贝到窗口DC
            BitBlt(hdc, 0, 0, g_clientWidth, g_clientHeight,
                   g_hMemDC, 0, 0, SRCCOPY);

            EndPaint(hWnd, &ps);
            break;
        }

        case WM_KEYDOWN: {
            // 按键修改文字内容（程序可控）
            switch (wParam) {
                case '1':
                    strcpy(g_textBuffer, "分配算法：mt19937。\n种子源：RDRAND指令。");
                    InvalidateRect(hWnd, nullptr, FALSE);
                    break;
                case '2':
                    strcpy(g_textBuffer, g_str);
                    InvalidateRect(hWnd, nullptr, FALSE);
                    break;
                case '3':
                    g_str = ArrangeSeat(false);
                    strcpy(g_textBuffer,g_str);
                    InvalidateRect(hWnd, nullptr, FALSE);
                    break;
            }
            break;
        }

        case WM_DESTROY: {
            KillTimer(hWnd, TIMER_ID);
            if (g_hMemDC) {
                SelectObject(g_hMemDC, g_hMemBitmap);
                DeleteObject(g_hMemBitmap);
                DeleteDC(g_hMemDC);
            }
            GdiplusShutdown(g_gdiplusToken);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ==================== 程序入口 ====================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    init_mt19937(hwrand());
    init_xorshift64(hwrand());
    g_str=ArrangeSeat(true);
    // 初始化 GDI+
    GdiplusStartupInput startupInput;
    if (GdiplusStartup(&g_gdiplusToken, &startupInput, nullptr) != Ok) {
        MessageBoxA(nullptr, "GDI+ initialization failed!", "Error", MB_ICONERROR);
        return 1;
    }

    // 注册窗口类（窄字符）
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;           // 不使用系统背景刷，完全自己绘制
    wc.lpszClassName = "MeteorEffectClass";
    if (!RegisterClassExA(&wc)) {
        MessageBoxA(nullptr, "Window class registration failed!", "Error", MB_ICONERROR);
        GdiplusShutdown(g_gdiplusToken);
        return 1;
    }

    // 创建窗口
    g_hWnd = CreateWindowExA(0, "MeteorEffectClass", "宏志软件集团座位分配器",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             CW_USEDEFAULT, CW_USEDEFAULT, 1366, 768,
                             nullptr, nullptr, hInstance, nullptr);
    if (!g_hWnd) {
        MessageBoxA(nullptr, "Window creation failed!", "Error", MB_ICONERROR);
        GdiplusShutdown(g_gdiplusToken);
        return 1;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // 消息循环
    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}