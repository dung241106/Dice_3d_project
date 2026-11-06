#define NOMINMAX
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/freeglut.h>
#include "ui.hpp"
#include <algorithm>
#include <cmath>
#include <map>

namespace ui
{
std::vector<Button> buttons;

namespace
{
double fontScale = 1;
#ifdef _WIN32
struct Font
{
    HFONT handle = nullptr;
    std::map<wchar_t, GLuint> glyphs;
};

std::map<int, Font> fonts;

std::wstring wide(const std::string& s)
{
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring w(static_cast<std::size_t>(n), L' ');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), w.data(), n);
    return w;
}

Font& font(int size)
{
    auto& f = fonts[size];
    if (!f.handle)
    {
        f.handle = CreateFontW(-std::max(8, int(size * fontScale)), 0, 0, 0,
                               size >= 26 ? FW_SEMIBOLD : FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                               ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    }
    return f;
}
#endif
} // namespace

void releaseFonts()
{
#ifdef _WIN32
    for (auto& pair : fonts)
    {
        for (auto g : pair.second.glyphs)
        {
            glDeleteLists(g.second, 1);
        }
        DeleteObject(pair.second.handle);
    }
    fonts.clear();
#endif
}

void initializeFonts(double scale)
{
    if (std::abs(scale - fontScale) > 0.001)
    {
        releaseFonts();
        fontScale = scale;
    }
}

void color(Color c)
{
    glColor3d(c.r, c.g, c.b);
}

void rectangle(double x, double y, double w, double h, Color c)
{
    color(c);
    glBegin(GL_QUADS);
    glVertex2d(x, y);
    glVertex2d(x + w, y);
    glVertex2d(x + w, y + h);
    glVertex2d(x, y + h);
    glEnd();
}

void box(double x, double y, double w, double h, Color c, double r)
{
    color(c);
    glBegin(GL_POLYGON);
    for (int k = 0; k < 4; k++)
    {
        for (int j = 0; j <= 8; j++)
        {
            double a = (k * 90 + j * 90.0 / 8) * 3.141592653589793 / 180;
            double cx = k == 0 || k == 3 ? x + w - r : x + r, cy = k < 2 ? y + h - r : y + r;
            glVertex2d(cx + r * std::cos(a), cy + r * std::sin(a));
        }
    }
    glEnd();
}

void text(double x, double y, const std::string& value, Color c, int size)
{
    color(c);
    glRasterPos2d(x, y);
#ifdef _WIN32
    auto& f = font(size);
    HDC dc = wglGetCurrentDC();
    HGDIOBJ previous = SelectObject(dc, f.handle);
    for (wchar_t ch : wide(value))
    {
        auto it = f.glyphs.find(ch);
        if (it == f.glyphs.end())
        {
            GLuint list = glGenLists(1);
            wglUseFontBitmapsW(dc, static_cast<DWORD>(ch), 1, list);
            it = f.glyphs.emplace(ch, list).first;
        }
        glCallList(it->second);
    }
    SelectObject(dc, previous);
#else
    for (unsigned char ch : value)
    {
        glutBitmapCharacter(size >= 18 ? GLUT_BITMAP_HELVETICA_18 : GLUT_BITMAP_HELVETICA_12, ch);
    }
#endif
}

double textWidth(const std::string& value, int size)
{
#ifdef _WIN32
    auto& f = font(size);
    HDC dc = wglGetCurrentDC();
    HGDIOBJ previous = SelectObject(dc, f.handle);
    SIZE extent{};
    auto s = wide(value);
    GetTextExtentPoint32W(dc, s.data(), static_cast<int>(s.size()), &extent);
    SelectObject(dc, previous);
    return extent.cx / fontScale;
#else
    return glutBitmapLength(size >= 18 ? GLUT_BITMAP_HELVETICA_18 : GLUT_BITMAP_HELVETICA_12,
                            reinterpret_cast<const unsigned char*>(value.c_str())) /
           fontScale;
#endif
}

void button(double x, double y, double w, const std::string& label, int id, bool selected,
            bool enabled, double h)
{
    box(x, y, w, h, selected ? Color{0.08, 0.26, 0.28} : raised, 7);
    int fontSize = textWidth(label, 18) > w - 20 ? 15 : 18;
    text(x + (w - textWidth(label, fontSize)) / 2, y + h / 2 + fontSize * 0.34, label,
         enabled ? (selected ? accent : white) : muted, fontSize);
    buttons.push_back({x, y, w, h, id, enabled});
}

int hit(double x, double y)
{
    for (auto it = buttons.rbegin(); it != buttons.rend(); ++it)
    {
        if (it->enabled && x >= it->x && y >= it->y && x < it->x + it->w && y < it->y + it->h)
        {
            return it->id;
        }
    }
    return -1;
}
} // namespace ui
