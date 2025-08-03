#include "Example.h"
#include <complex>
#include <vector>
#include <cmath>

/************************************************************************/
/* Mandelbrot Set Implementation                                        */
/************************************************************************/

class MandelbrotRenderer {
private:
    double centerX, centerY;
    double zoom;
    int maxIterations;
    int width, height;
    
public:
    MandelbrotRenderer(int w, int h) 
        : centerX(-0.5), centerY(0.0), zoom(1.0), maxIterations(100), width(w), height(h) {}
    
    // 망델브로 셋 계산 (한 점에 대해)
    int mandelbrotIterations(double x, double y) {
        std::complex<double> c(x, y);
        std::complex<double> z(0, 0);
        
        int iterations = 0;
        while (iterations < maxIterations && std::abs(z) < 2.0) {
            z = z * z + c;
            iterations++;
        }
        
        return iterations;
    }
    
    // 부드러운 색상 계산 (smooth coloring)
    double smoothMandelbrot(double x, double y) {
        std::complex<double> c(x, y);
        std::complex<double> z(0, 0);
        
        int iterations = 0;
        while (iterations < maxIterations && std::abs(z) < 2.0) {
            z = z * z + c;
            iterations++;
        }
        
        if (iterations == maxIterations) {
            return iterations;
        }
        
        // 부드러운 색상을 위한 추가 계산
        double smoothValue = iterations + 1 - log2(log2(std::abs(z)));
        return smoothValue;
    }
    
    // 화면 좌표를 복소수 평면으로 변환
    void screenToComplex(int screenX, int screenY, double& complexX, double& complexY) {
        double aspectRatio = (double)width / height;
        
        complexX = centerX + (screenX - width/2.0) * (4.0 / zoom) / width;
        complexY = centerY + (screenY - height/2.0) * (4.0 / zoom) / height;
        
        // 종횡비 보정
        if (aspectRatio > 1.0) {
            complexX *= aspectRatio;
        } else {
            complexY /= aspectRatio;
        }
    }
    
    // HSV to RGB 색상 변환
    void hsvToRgb(double h, double s, double v, uint8_t& r, uint8_t& g, uint8_t& b) {
        double c = v * s;
        double x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
        double m = v - c;
        
        double r1, g1, b1;
        
        if (h >= 0 && h < 60) {
            r1 = c; g1 = x; b1 = 0;
        } else if (h >= 60 && h < 120) {
            r1 = x; g1 = c; b1 = 0;
        } else if (h >= 120 && h < 180) {
            r1 = 0; g1 = c; b1 = x;
        } else if (h >= 180 && h < 240) {
            r1 = 0; g1 = x; b1 = c;
        } else if (h >= 240 && h < 300) {
            r1 = x; g1 = 0; b1 = c;
        } else {
            r1 = c; g1 = 0; b1 = x;
        }
        
        r = (uint8_t)((r1 + m) * 255);
        g = (uint8_t)((g1 + m) * 255);
        b = (uint8_t)((b1 + m) * 255);
    }
    
    // 반복 횟수를 색상으로 변환
    void iterationsToColor(double iterations, uint8_t& r, uint8_t& g, uint8_t& b) {
        if (iterations >= maxIterations) {
            // 망델브로 셋 내부 (검은색)
            r = g = b = 0;
            return;
        }
        
        // 무지개 색상 스펙트럼
        double hue = fmod(iterations * 360.0 / 25.0, 360.0);
        double saturation = 1.0;
        double value = iterations < maxIterations ? 1.0 : 0.0;
        
        // 거리에 따른 색상 강도 조절
        double intensity = 1.0 - (iterations / maxIterations);
        value *= (0.5 + 0.5 * intensity);
        
        hsvToRgb(hue, saturation, value, r, g, b);
    }
    
    void setZoom(double z) { zoom = z; }
    void setCenter(double x, double y) { centerX = x; centerY = y; }
    void setMaxIterations(int iter) { maxIterations = iter; }
    
    double getZoom() const { return zoom; }
    double getCenterX() const { return centerX; }
    double getCenterY() const { return centerY; }
    int getMaxIterations() const { return maxIterations; }
};

/************************************************************************/
/* ThorVG Drawing Contents                                              */
/************************************************************************/

struct MandelbrotExample : tvgexam::Example
{
private:
    MandelbrotRenderer renderer;
    uint32_t width, height;
    
public:
    MandelbrotExample() : renderer(800, 600), width(800), height(600) {}
    
    bool content(tvg::Canvas* canvas, uint32_t w, uint32_t h) override
    {
        // 초기 설정
        width = w;
        height = h;
        renderer = MandelbrotRenderer(w, h);
        
        // 초기 프레임 렌더링
        renderMandelbrotSet(canvas, w, h);
        
        return true;
    }
    
    bool update(tvg::Canvas* canvas, uint32_t elapsed) override
    {
        // 경과 시간을 초 단위로 변환
        float animationTime = elapsed * 0.001f;
        
        // 애니메이션 업데이트
        updateAnimation(animationTime);
        
        // Canvas 내용 지우고 새로 그리기
        canvas->remove(); // 모든 Paint 객체 제거
        renderMandelbrotSet(canvas, width, height);
        
        // 항상 업데이트 (연속 애니메이션)
        canvas->update();
        return true;
    }
    
private:
    void updateAnimation(float animationTime) {
        // 메인 카디오이드에서 미니 망델브로로 자연스럽게 이동
        
        // 시작점: 메인 카디오이드
        double startX = -0.5;
        double startY = 0.0;
        double startZoom = 1.0;
        
        // 목표점: 미니 망델브로
        double endX = -0.8;
        double endY = 0.156;
        double endZoom = 10.0;
        
        // 부드러운 전환을 위한 진행률 계산 (30초 주기)
        float progress = fmod(animationTime * 0.033f, 1.0f);
        
        // 스무스스텝 함수로 자연스러운 가속/감속
        float t = progress * progress * (3.0f - 2.0f * progress);
        
        // 좌표와 줌을 부드럽게 보간
        double centerX = startX + t * (endX - startX);
        double centerY = startY + t * (endY - startY);
        double zoom = startZoom + t * (endZoom - startZoom);
        
        renderer.setCenter(centerX, centerY);
        renderer.setZoom(zoom);
        renderer.setMaxIterations(80 + (int)(20 * (1.0 + t))); // 이동하면서 세밀도도 증가
    }
    
    void renderMandelbrotSet(tvg::Canvas* canvas, uint32_t w, uint32_t h) {
        // 성능을 위해 픽셀 블록 단위로 렌더링
        int blockSize = 2; // 2x2 픽셀 블록
        
        for (int y = 0; y < (int)h; y += blockSize) {
            for (int x = 0; x < (int)w; x += blockSize) {
                // 복소수 좌표 계산
                double complexX, complexY;
                renderer.screenToComplex(x, y, complexX, complexY);
                
                // 망델브로 셋 계산
                double iterations = renderer.smoothMandelbrot(complexX, complexY);
                
                // 색상 계산
                uint8_t r, g, b;
                renderer.iterationsToColor(iterations, r, g, b);
                
                // 픽셀 블록 그리기
                auto pixelBlock = tvg::Shape::gen();
                pixelBlock->appendRect(x, y, blockSize, blockSize, 0, 0);
                pixelBlock->fill(r, g, b);
                canvas->push(pixelBlock);
            }
        }
    }
};

/************************************************************************/
/* Entry Point                                                          */
/************************************************************************/

int main(int argc, char **argv)
{
    return tvgexam::main(new MandelbrotExample, argc, argv);
}