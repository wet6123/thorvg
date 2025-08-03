/*
 * Advanced ThorVG Ray Casting - Interactive with Textures and Lighting
 */

#include "Example.h"
#include <vector>
#include <cmath>
#include <algorithm>

/************************************************************************/
/* Advanced Ray Casting with Interactive Controls                      */
/************************************************************************/

struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Wall {
    Point2D start, end;
    uint8_t r, g, b; // 벽 색상
    Wall(Point2D s, Point2D e, uint8_t red = 150, uint8_t green = 150, uint8_t blue = 150) 
        : start(s), end(e), r(red), g(green), b(blue) {}
};

struct LightSource {
    Point2D position;
    float intensity;
    uint8_t r, g, b;
    
    LightSource(Point2D pos, float i, uint8_t red, uint8_t green, uint8_t blue)
        : position(pos), intensity(i), r(red), g(green), b(blue) {}
};

class AdvancedRayCaster {
private:
    std::vector<Wall> walls;
    std::vector<LightSource> lights;
    Point2D player;
    float playerAngle;
    int numRays;
    float fov;
    float moveSpeed;
    float rotSpeed;
    
public:
    AdvancedRayCaster() : player(400, 300), playerAngle(0), numRays(120), fov(M_PI/2.5), 
                         moveSpeed(3.0f), rotSpeed(0.08f) {
        
        // 색깔이 있는 벽들로 미로 생성
        walls.push_back(Wall(Point2D(50, 50), Point2D(750, 50), 200, 100, 100));     // 빨간 벽
        walls.push_back(Wall(Point2D(750, 50), Point2D(750, 550), 100, 200, 100));   // 녹색 벽
        walls.push_back(Wall(Point2D(750, 550), Point2D(50, 550), 100, 100, 200));   // 파란 벽
        walls.push_back(Wall(Point2D(50, 550), Point2D(50, 50), 200, 200, 100));     // 노란 벽
        
        // 내부 미로 구조
        walls.push_back(Wall(Point2D(150, 150), Point2D(250, 150), 180, 120, 180));
        walls.push_back(Wall(Point2D(250, 150), Point2D(250, 250), 180, 120, 180));
        walls.push_back(Wall(Point2D(350, 100), Point2D(450, 100), 120, 180, 180));
        walls.push_back(Wall(Point2D(450, 100), Point2D(450, 200), 120, 180, 180));
        walls.push_back(Wall(Point2D(550, 150), Point2D(650, 150), 180, 180, 120));
        walls.push_back(Wall(Point2D(650, 150), Point2D(650, 300), 180, 180, 120));
        
        walls.push_back(Wall(Point2D(100, 350), Point2D(200, 350), 200, 150, 100));
        walls.push_back(Wall(Point2D(200, 350), Point2D(200, 450), 200, 150, 100));
        walls.push_back(Wall(Point2D(300, 400), Point2D(400, 300), 150, 200, 150));
        walls.push_back(Wall(Point2D(500, 350), Point2D(600, 450), 100, 150, 200));
        
        // 조명 추가
        lights.push_back(LightSource(Point2D(200, 200), 100, 255, 200, 200));  // 따뜻한 빛
        lights.push_back(LightSource(Point2D(600, 200), 80, 200, 200, 255));   // 차가운 빛
        lights.push_back(LightSource(Point2D(400, 450), 120, 200, 255, 200));  // 녹색 빛
    }
    
    Point2D* rayWallIntersection(const Point2D& rayStart, const Point2D& rayDir, const Wall& wall, float& distance) {
        Point2D wallDir(wall.end.x - wall.start.x, wall.end.y - wall.start.y);
        
        float denominator = rayDir.x * wallDir.y - rayDir.y * wallDir.x;
        if (abs(denominator) < 1e-10) return nullptr;
        
        float t = ((wall.start.x - rayStart.x) * wallDir.y - (wall.start.y - rayStart.y) * wallDir.x) / denominator;
        float u = ((wall.start.x - rayStart.x) * rayDir.y - (wall.start.y - rayStart.y) * rayDir.x) / denominator;
        
        if (t > 0 && u >= 0 && u <= 1) {
            distance = t;
            return new Point2D(rayStart.x + t * rayDir.x, rayStart.y + t * rayDir.y);
        }
        
        return nullptr;
    }
    
    struct RayHit {
        Point2D point;
        float distance;
        Wall* wall;
        bool hit;
        
        RayHit() : hit(false), wall(nullptr), distance(0) {}
    };
    
    RayHit castRay(float angle) {
        Point2D rayDir(cos(angle), sin(angle));
        RayHit result;
        float minDistance = std::numeric_limits<float>::max();
        
        for (auto& wall : walls) {
            float distance;
            Point2D* intersection = rayWallIntersection(player, rayDir, wall, distance);
            if (intersection && distance < minDistance) {
                minDistance = distance;
                result.point = *intersection;
                result.wall = &wall;
                result.hit = true;
                result.distance = distance;
            }
            delete intersection;
        }
        
        return result;
    }
    
    // 조명 계산
    float calculateLighting(const Point2D& point) {
        float totalLight = 0.1f; // 환경광
        
        for (const auto& light : lights) {
            float dx = light.position.x - point.x;
            float dy = light.position.y - point.y;
            float distance = sqrt(dx*dx + dy*dy);
            
            if (distance > 0) {
                float lightContribution = light.intensity / (distance * 0.01f + 1);
                totalLight += lightContribution * 0.01f;
            }
        }
        
        return std::min(1.0f, totalLight);
    }
    
    void updatePlayer(float time) {
        // 더 복잡한 이동 패턴
        float sinTime = sin(time * 0.005f);
        float cosTime = cos(time * 0.003f);
        
        float dx = cos(playerAngle) * moveSpeed * (1 + sinTime * 0.3f);
        float dy = sin(playerAngle) * moveSpeed * (1 + cosTime * 0.3f);
        
        // 충돌 감지
        Point2D newPos(player.x + dx, player.y + dy);
        if (!checkCollision(newPos)) {
            player = newPos;
        } else {
            // 벽에 부딪히면 방향 변경
            playerAngle += M_PI * 0.7f;
        }
        
        playerAngle += rotSpeed * (1 + cosTime * 0.5f);
        
        // 경계 체크
        player.x = std::max(60.0f, std::min(740.0f, player.x));
        player.y = std::max(60.0f, std::min(540.0f, player.y));
    }
    
    bool checkCollision(const Point2D& pos) {
        for (const auto& wall : walls) {
            float dx = wall.end.x - wall.start.x;
            float dy = wall.end.y - wall.start.y;
            float length = sqrt(dx*dx + dy*dy);
            
            if (length == 0) continue;
            
            // 점과 선분 사이의 거리 계산
            float t = std::max(0.0f, std::min(1.0f, ((pos.x - wall.start.x) * dx + (pos.y - wall.start.y) * dy) / (length * length)));
            float projX = wall.start.x + t * dx;
            float projY = wall.start.y + t * dy;
            
            float distSq = (pos.x - projX) * (pos.x - projX) + (pos.y - projY) * (pos.y - projY);
            if (distSq < 15*15) return true; // 플레이어 반지름 고려
        }
        return false;
    }
    
    const std::vector<Wall>& getWalls() const { return walls; }
    const std::vector<LightSource>& getLights() const { return lights; }
    const Point2D& getPlayer() const { return player; }
    float getPlayerAngle() const { return playerAngle; }
    int getNumRays() const { return numRays; }
    float getFov() const { return fov; }
};

/************************************************************************/
/* ThorVG Advanced Drawing                                              */
/************************************************************************/

struct AdvancedRayCastingExample : tvgexam::Example
{
private:
    AdvancedRayCaster raycaster;
    float animationTime = 0;
    
public:
    bool content(tvg::Canvas* canvas, uint32_t w, uint32_t h) override
    {
        // 배경 그라디언트
        auto background = tvg::Shape::gen();
        background->appendRect(0, 0, w, h, 0, 0);
        
        auto bgGrad = tvg::LinearGradient::gen();
        bgGrad->linear(0, 0, 0, h);
        tvg::Fill::ColorStop bgStops[2];
        bgStops[0] = {0.0f, 5, 5, 15, 255};
        bgStops[1] = {1.0f, 15, 15, 30, 255};
        bgGrad->colorStops(bgStops, 2);
        background->fill(std::move(bgGrad));
        canvas->push(background);
        
        // 플레이어 업데이트
        raycaster.updatePlayer(animationTime);
        
        // 조명 효과 그리기
        drawLights(canvas);
        
        // 벽 그리기 (색상 포함)
        drawColoredWalls(canvas);
        
        // 고급 레이 캐스팅
        drawAdvancedRays(canvas);
        
        // 플레이어 그리기
        drawAdvancedPlayer(canvas);
        
        // 고품질 3D 뷰
        drawAdvanced3DView(canvas, w, h);
        
        // 미니맵
        drawMinimap(canvas, w, h);
        
        animationTime++;
        return true;
    }
    
private:
    void drawLights(tvg::Canvas* canvas) {
        for (const auto& light : raycaster.getLights()) {
            // 광원 주변 발광 효과
            auto lightGlow = tvg::Shape::gen();
            float glowRadius = light.intensity * 0.8f;
            lightGlow->appendCircle(light.position.x, light.position.y, glowRadius, glowRadius);
            
            auto glowGrad = tvg::RadialGradient::gen();
            glowGrad->radial(light.position.x, light.position.y, glowRadius, light.position.x, light.position.y, 0);
            
            tvg::Fill::ColorStop glowStops[3];
            glowStops[0] = {0.0f, light.r, light.g, light.b, 80};
            glowStops[1] = {0.6f, light.r, light.g, light.b, 20};
            glowStops[2] = {1.0f, light.r, light.g, light.b, 0};
            
            glowGrad->colorStops(glowStops, 3);
            lightGlow->fill(std::move(glowGrad));
            canvas->push(lightGlow);
            
            // 중심 광원
            auto lightCore = tvg::Shape::gen();
            lightCore->appendCircle(light.position.x, light.position.y, 4, 4);
            lightCore->fill(255, 255, 255);
            canvas->push(lightCore);
        }
    }
    
    void drawColoredWalls(tvg::Canvas* canvas) {
        for (const auto& wall : raycaster.getWalls()) {
            auto wallShape = tvg::Shape::gen();
            wallShape->moveTo(wall.start.x, wall.start.y);
            wallShape->lineTo(wall.end.x, wall.end.y);
            wallShape->strokeFill(wall.r, wall.g, wall.b, 255);
            wallShape->strokeWidth(4);
            canvas->push(wallShape);
        }
    }
    
    void drawAdvancedRays(tvg::Canvas* canvas) {
        Point2D player = raycaster.getPlayer();
        float playerAngle = raycaster.getPlayerAngle();
        float fov = raycaster.getFov();
        int numRays = raycaster.getNumRays();
        
        for (int i = 0; i < numRays; i += 3) { // 성능을 위해 간격을 둠
            float rayAngle = playerAngle - fov/2 + (fov * i) / (numRays - 1);
            auto hit = raycaster.castRay(rayAngle);
            
            if (hit.hit) {
                auto rayLine = tvg::Shape::gen();
                rayLine->moveTo(player.x, player.y);
                rayLine->lineTo(hit.point.x, hit.point.y);
                
                // 조명과 거리에 따른 색상
                float lighting = raycaster.calculateLighting(hit.point);
                float alpha = std::max(0.05f, lighting * (1.0f - hit.distance / 500.0f));
                
                rayLine->strokeFill(255, 255, 100, (uint8_t)(alpha * 150));
                rayLine->strokeWidth(1);
                canvas->push(rayLine);
            }
        }
    }
    
    void drawAdvancedPlayer(tvg::Canvas* canvas) {
        Point2D player = raycaster.getPlayer();
        float playerAngle = raycaster.getPlayerAngle();
        
        // 플레이어 발광 효과
        auto playerGlow = tvg::Shape::gen();
        playerGlow->appendCircle(player.x, player.y, 15, 15);
        
        auto playerGlowGrad = tvg::RadialGradient::gen();
        playerGlowGrad->radial(player.x, player.y, 15, player.x, player.y, 0);
        
        tvg::Fill::ColorStop playerStops[2];
        playerStops[0] = {0.0f, 100, 255, 100, 100};
        playerStops[1] = {1.0f, 100, 255, 100, 0};
        
        playerGlowGrad->colorStops(playerStops, 2);
        playerGlow->fill(std::move(playerGlowGrad));
        canvas->push(playerGlow);
        
        // 플레이어 본체
        auto playerCircle = tvg::Shape::gen();
        playerCircle->appendCircle(player.x, player.y, 8, 8);
        playerCircle->fill(100, 255, 100);
        playerCircle->strokeFill(255, 255, 255, 255);
        playerCircle->strokeWidth(2);
        canvas->push(playerCircle);
        
        // 시야 범위 표시
        float fov = raycaster.getFov();
        auto fovArc = tvg::Shape::gen();
        fovArc->moveTo(player.x, player.y);
        
        float arcRadius = 50;
        int arcSteps = 20;
        for (int i = 0; i <= arcSteps; i++) {
            float angle = playerAngle - fov/2 + (fov * i) / arcSteps;
            float x = player.x + cos(angle) * arcRadius;
            float y = player.y + sin(angle) * arcRadius;
            if (i == 0) fovArc->moveTo(x, y);
            else fovArc->lineTo(x, y);
        }
        fovArc->lineTo(player.x, player.y);
        fovArc->fill(255, 255, 255, 30);  // 반투명 시야 범위
        canvas->push(fovArc);
        
        // 방향 화살표
        auto directionArrow = tvg::Shape::gen();
        float arrowLength = 25;
        float arrowWidth = 8;
        
        Point2D tip(player.x + cos(playerAngle) * arrowLength, 
                   player.y + sin(playerAngle) * arrowLength);
        Point2D left(player.x + cos(playerAngle - 2.5) * (arrowLength * 0.7), 
                    player.y + sin(playerAngle - 2.5) * (arrowLength * 0.7));
        Point2D right(player.x + cos(playerAngle + 2.5) * (arrowLength * 0.7), 
                     player.y + sin(playerAngle + 2.5) * (arrowLength * 0.7));
        
        directionArrow->moveTo(tip.x, tip.y);
        directionArrow->lineTo(left.x, left.y);
        directionArrow->lineTo(player.x, player.y);
        directionArrow->lineTo(right.x, right.y);
        directionArrow->lineTo(tip.x, tip.y);
        
        directionArrow->fill(255, 255, 255);
        directionArrow->strokeFill(0, 0, 0, 255);
        directionArrow->strokeWidth(1);
        canvas->push(directionArrow);
    }
    
    void drawAdvanced3DView(tvg::Canvas* canvas, uint32_t w, uint32_t h) {
        float viewWidth = w * 0.45f;
        float viewHeight = h * 0.7f;
        float viewX = w - viewWidth - 10;
        float viewY = 10;
        
        // 3D 뷰 배경 그라디언트
        auto view3D = tvg::Shape::gen();
        view3D->appendRect(viewX, viewY, viewWidth, viewHeight, 8, 8);
        
        auto view3DGrad = tvg::LinearGradient::gen();
        view3DGrad->linear(viewX, viewY, viewX, viewY + viewHeight);
        
        tvg::Fill::ColorStop viewStops[3];
        viewStops[0] = {0.0f, 50, 50, 80, 255};   // 천장
        viewStops[1] = {0.5f, 20, 20, 35, 255};   // 중간
        viewStops[2] = {1.0f, 30, 30, 50, 255};   // 바닥
        
        view3DGrad->colorStops(viewStops, 3);
        view3D->fill(std::move(view3DGrad));
        view3D->strokeFill(150, 150, 150, 255);
        view3D->strokeWidth(2);
        canvas->push(view3D);
        
        // 레이 캐스팅으로 3D 벽 생성
        Point2D player = raycaster.getPlayer();
        float playerAngle = raycaster.getPlayerAngle();
        float fov = raycaster.getFov();
        int numRays = raycaster.getNumRays();
        
        for (int i = 0; i < numRays; i++) {
            float rayAngle = playerAngle - fov/2 + (fov * i) / (numRays - 1);
            auto hit = raycaster.castRay(rayAngle);
            
            if (hit.hit) {
                // 어안렌즈 효과 보정
                float correctedDistance = hit.distance * cos(rayAngle - playerAngle);
                
                // 벽 높이 계산
                float wallHeight = std::min(viewHeight, viewHeight * 150.0f / (correctedDistance + 1));
                
                // 조명 계산
                float lighting = raycaster.calculateLighting(hit.point);
                
                // 벽 스트립 그리기
                float stripWidth = viewWidth / numRays;
                float stripX = viewX + i * stripWidth;
                float stripY = viewY + (viewHeight - wallHeight) / 2;
                
                auto wallStrip = tvg::Shape::gen();
                wallStrip->appendRect(stripX, stripY, stripWidth + 1, wallHeight, 0, 0);
                
                // 벽 색상 적용 (조명과 거리 고려)
                float distanceFactor = std::max(0.2f, 1.0f - correctedDistance / 400.0f);
                float finalBrightness = lighting * distanceFactor;
                
                uint8_t r = (uint8_t)(hit.wall->r * finalBrightness);
                uint8_t g = (uint8_t)(hit.wall->g * finalBrightness);
                uint8_t b = (uint8_t)(hit.wall->b * finalBrightness);
                
                wallStrip->fill(r, g, b);
                canvas->push(wallStrip);
                
                // 텍스처 효과 (세로 선)
                if (i % 3 == 0) {
                    auto texture = tvg::Shape::gen();
                    texture->moveTo(stripX + stripWidth/2, stripY);
                    texture->lineTo(stripX + stripWidth/2, stripY + wallHeight);
                    texture->strokeFill(r + 20, g + 20, b + 20, 100);
                    texture->strokeWidth(1);
                    canvas->push(texture);
                }
            }
        }
        
        // 크로스헤어
        auto crosshair = tvg::Shape::gen();
        float centerX = viewX + viewWidth/2;
        float centerY = viewY + viewHeight/2;
        
        crosshair->moveTo(centerX - 10, centerY);
        crosshair->lineTo(centerX + 10, centerY);
        crosshair->moveTo(centerX, centerY - 10);
        crosshair->lineTo(centerX, centerY + 10);
        
        crosshair->strokeFill(255, 255, 255, 150);
        crosshair->strokeWidth(2);
        canvas->push(crosshair);
        
        // FPS 스타일 UI
        auto uiPanel = tvg::Shape::gen();
        uiPanel->appendRect(viewX + 10, viewY + viewHeight - 40, 100, 30, 5, 5);
        uiPanel->fill(0, 0, 0, 180);
        canvas->push(uiPanel);
    }
    
    void drawMinimap(tvg::Canvas* canvas, uint32_t w, uint32_t h) {
        float mapSize = 150;
        float mapX = 10;
        float mapY = h - mapSize - 10;
        
        // 미니맵 배경
        auto minimapBg = tvg::Shape::gen();
        minimapBg->appendRect(mapX, mapY, mapSize, mapSize, 5, 5);
        minimapBg->fill(0, 0, 0, 200);
        minimapBg->strokeFill(100, 100, 100, 255);
        minimapBg->strokeWidth(2);
        canvas->push(minimapBg);
        
        // 맵 스케일 계산
        float scaleX = mapSize / 700.0f;  // 맵 전체 너비
        float scaleY = mapSize / 500.0f;  // 맵 전체 높이
        
        // 미니맵 벽들
        for (const auto& wall : raycaster.getWalls()) {
            auto miniWall = tvg::Shape::gen();
            float startX = mapX + (wall.start.x - 50) * scaleX;
            float startY = mapY + (wall.start.y - 50) * scaleY;
            float endX = mapX + (wall.end.x - 50) * scaleX;
            float endY = mapY + (wall.end.y - 50) * scaleY;
            
            miniWall->moveTo(startX, startY);
            miniWall->lineTo(endX, endY);
            miniWall->strokeFill(wall.r, wall.g, wall.b, 200);
            miniWall->strokeWidth(2);
            canvas->push(miniWall);
        }
        
        // 미니맵 플레이어
        Point2D player = raycaster.getPlayer();
        float playerX = mapX + (player.x - 50) * scaleX;
        float playerY = mapY + (player.y - 50) * scaleY;
        
        auto miniPlayer = tvg::Shape::gen();
        miniPlayer->appendCircle(playerX, playerY, 3, 3);
        miniPlayer->fill(100, 255, 100);
        canvas->push(miniPlayer);
        
        // 미니맵 시야 방향
        float playerAngle = raycaster.getPlayerAngle();
        auto miniDirection = tvg::Shape::gen();
        float dirLength = 15;
        miniDirection->moveTo(playerX, playerY);
        miniDirection->lineTo(
            playerX + cos(playerAngle) * dirLength,
            playerY + sin(playerAngle) * dirLength
        );
        miniDirection->strokeFill(255, 255, 255, 255);
        miniDirection->strokeWidth(2);
        canvas->push(miniDirection);
        
        // 미니맵 조명
        for (const auto& light : raycaster.getLights()) {
            float lightX = mapX + (light.position.x - 50) * scaleX;
            float lightY = mapY + (light.position.y - 50) * scaleY;
            
            auto miniLight = tvg::Shape::gen();
            miniLight->appendCircle(lightX, lightY, 2, 2);
            miniLight->fill(light.r, light.g, light.b);
            canvas->push(miniLight);
        }
    }
};

/************************************************************************/
/* Entry Point                                                          */
/************************************************************************/

int main(int argc, char **argv)
{
    return tvgexam::main(new AdvancedRayCastingExample, argc, argv);
}