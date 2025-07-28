#include "Example.h"
#include <cmath>

class SierpinskiFractal {
public:
    struct Point {
        float x, y;
        Point(float x = 0, float y = 0) : x(x), y(y) {}
    };

    void drawSierpinskiTriangle(tvg::Canvas* canvas, Point a, Point b, Point c, int depth, uint8_t r, uint8_t g, uint8_t blue, uint8_t alpha) {
        if (depth == 0) {
            auto triangle = tvg::Shape::gen();
            triangle->moveTo(a.x, a.y);
            triangle->lineTo(b.x, b.y);
            triangle->lineTo(c.x, c.y);
            triangle->close();
            triangle->fill(r, g, blue, alpha);
            canvas->push(triangle);
            return;
        }

        Point ab = Point((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);
        Point bc = Point((b.x + c.x) * 0.5f, (b.y + c.y) * 0.5f);
        Point ca = Point((c.x + a.x) * 0.5f, (c.y + a.y) * 0.5f);

        int colorShift = depth * 30;
        uint8_t newR = (uint8_t)((r + colorShift) % 255);
        uint8_t newG = (uint8_t)((g + (int)(colorShift * 1.3f)) % 255);
        uint8_t newB = (uint8_t)((blue + colorShift * 2) % 255);

        drawSierpinskiTriangle(canvas, a, ab, ca, depth - 1, newR, newG, newB, alpha);
        drawSierpinskiTriangle(canvas, ab, b, bc, depth - 1, newR, newG, newB, alpha);
        drawSierpinskiTriangle(canvas, ca, bc, c, depth - 1, newR, newG, newB, alpha);
    }
};

struct SierpinskiExample : tvgexam::Example
{
private:
    SierpinskiFractal fractal;
    
public:
    bool content(tvg::Canvas* canvas, uint32_t w, uint32_t h) override
    {
        auto bg = tvg::Shape::gen();
        bg->appendRect(0, 0, w, h);
        bg->fill(10, 10, 20);
        canvas->push(bg);

        float centerX = w * 0.5f;
        float centerY = h * 0.5f;
        float size = std::min(w, h) * 0.4f;

        fractal.drawSierpinskiTriangle(canvas, 
            SierpinskiFractal::Point(centerX, centerY - size),
            SierpinskiFractal::Point(centerX - size * 0.866f, centerY + size * 0.5f),
            SierpinskiFractal::Point(centerX + size * 0.866f, centerY + size * 0.5f),
            7, 255, 150, 100, 200);

        return true;
    }
};

int main(int argc, char **argv)
{
    return tvgexam::main(new SierpinskiExample, argc, argv);
}