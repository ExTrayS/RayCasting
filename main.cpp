#include <stdio.h> 
#include "include/SDL/SDL.h"
#include <stdint.h>
#include <math.h>
#undef main


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define MINIMAP_SCALE 0.3f

struct Vec2f
{
    float x;
    float y;
};


struct CoordinateSystem
{
    Vec2f origin;
    Vec2f U;
    Vec2f V;
};

void DrawCircle(SDL_Renderer * renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    const int32_t diameter = (radius * 2);
    
    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);
    
    while (x >= y)
    {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);
        
        if (error <= 0)
        {
            ++y;
            error += ty;
            ty += 2;
        }
        
        if (error > 0)
        {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

struct RayInfo
{
    float wallX;
    float wallY;
    float distance;
}rays[32];

#define RAYS_COUNT WINDOW_WIDTH
#define PI 3.14159f

int main(int argc, char *argv[])
{
    
    SDL_Init(SDL_INIT_EVERYTHING);
    
    SDL_Window *window = SDL_CreateWindow("RaycastING",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    SDL_Texture *texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBX8888,SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    const char *error = SDL_GetError();
    
    SDL_Rect rect{};
    rect.x = 0;
    rect.y = 0;
    rect.w = WINDOW_WIDTH;
    rect.h = WINDOW_HEIGHT;
    
    int32_t pitch = 0;
    void *pixels;
    
    CoordinateSystem playerCoordinateSystem{};
    playerCoordinateSystem.origin = {150.0f,150.0f};
    playerCoordinateSystem.U = {1.0f,0.0f};
    playerCoordinateSystem.V = {0.0f,1.0f};
    
    Vec2f lineOrigin = {0.0f,0.0f};
    Vec2f lineEnd = {5.0f,0.0f};
    
    float playerX = 130.0f;
    float playerY = 130.0f;
    
    float lastPlayerX = playerX;
    float lastPlayerY = playerY;
    
    float cx = 1.0f;
    float cy = 0.0f;
    
    float x = 1.0f;
    float y = 0.0f;
    static float angle = 1.0f;
    Vec2f dir{};
    dir.x = 1.0f;
    dir.y = 0.0f;
    
    uint32_t map[10][10] = 
    {
        {1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,1,1},
        {1,0,0,1,1,0,0,0,1,1},
        {1,0,0,0,1,0,0,0,0,1},
        {1,0,0,1,1,0,0,0,1,1},
        {1,0,0,0,1,0,0,0,1,1},
        {1,0,0,1,1,0,0,0,1,0},
        {1,0,0,0,1,0,0,0,1,1},
        {1,0,0,1,1,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1},
    };
    const float FOV = 90.0f*(PI/180.0f);
    
    float distanceToProjectPlane = ((float)WINDOW_WIDTH / 2.0f) / tan(FOV/2.0f);
    
    uint32_t first = SDL_GetTicks();
    for(;;)
    {
        SDL_Event ev;
        float speed = 20.0f;
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT)
            {
                return 0;
            }
            if(ev.type == SDL_KEYDOWN)
            {
                switch(ev.key.keysym.sym )
                {
                    case SDLK_LEFT:
                    angle -= 0.01f;
                    break;
                    case SDLK_RIGHT:
                    {
                        angle += 0.01f;
                    }break;
                    case SDLK_UP:
                    {
                        playerX = playerX + cx*speed*0.16f;
                        playerY = playerY + cy*speed*0.16f;
                        angle+= 0.01f;
                    }break;
                    case SDLK_DOWN:
                    {
                        
                    }break;
                    default:
                    break;
                }
            }
        }
#define TILE_WIDTH 80
#define TILE_HEIGHT 80
        
        SDL_LockTexture(texture,0,&pixels, &pitch);
#if 0 
        uint8_t *row = (uint8_t*)pixels;
        for(uint32_t y = 0; y < WINDOW_HEIGHT; y++)
        {
            uint32_t *pixel = (uint32_t*)row;
            for(uint32_t x = 0; x < WINDOW_WIDTH; x++)
            {
                uint32_t mapX = x / TILE_WIDTH;
                uint32_t mapY = y / TILE_HEIGHT;
#if 0
                if(wallX == hittedX && wallY  == hittedY)
                {
                    *pixel++ = 0xffffffff;
                    continue;
                }
#endif
                if(map[mapY][mapX] == 1)
                {
                    *pixel++ = 0xff0000ff;
                }
                else
                {
                    *pixel++ = 0xffff00ff;
                }
                
            }
            row += pitch;
        }
#endif
        angle = remainder(angle, 2*PI);
        if (angle < 0)
        {
            angle = 2*PI + angle;
        }
        
        float rayAngle = angle - (FOV / 2.0f); // NOTE 60 degress
        for(uint32_t rayIndex = 0; rayIndex < RAYS_COUNT; rayIndex++)
        {
            RayInfo *rayInfo = &rays[rayIndex];
            
            float hittedX = -1;
            float hittedY = -1;
            bool horizHit = false;
            
            float hittedXVert = -1;
            float hittedYVert = -1;
            bool vertHit = false;
            
            float wallX = -1;
            float wallY = -1;
            
            bool isRayFacingDown = rayAngle > 0 && rayAngle < PI;
            bool isRayFacingUp = !isRayFacingDown;
            
            bool isRayFacingRight = rayAngle <= (PI*0.5f) ||  rayAngle > 1.5f * PI;
            bool isRayFacingLeft = !isRayFacingRight;
            
            float Ay = int(playerY / (float)TILE_HEIGHT)*(float)TILE_HEIGHT;
            Ay += isRayFacingDown ? TILE_HEIGHT : 0;
            
            float Ax = playerX + (Ay - playerY)/(tan(rayAngle));
            
            float xstep = (float)TILE_HEIGHT / tan(rayAngle);
            xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
            xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;
            float ystep = TILE_HEIGHT;
            ystep *= isRayFacingUp ? -1 : 1;
            
            float nextHorizTouchX = Ax;
            float nextHorizTouchY = Ay;
            
            while(nextHorizTouchX >= 0 && nextHorizTouchX <= WINDOW_WIDTH && nextHorizTouchY >= 0 && nextHorizTouchY <= WINDOW_HEIGHT)
            { 
                uint32_t mapX = nextHorizTouchX / (float)TILE_WIDTH;
                uint32_t mapY = isRayFacingUp ? (nextHorizTouchY-1) / (float)TILE_HEIGHT : nextHorizTouchY / (float)TILE_HEIGHT;
                if(map[mapY][mapX] == 1)
                {
                    horizHit = true;
                    hittedX = nextHorizTouchX;
                    hittedY = nextHorizTouchY;
                    break;
                }
                else
                {
                    nextHorizTouchX += xstep;
                    nextHorizTouchY += ystep;
                }
            }
            
            // NOTE Vertical lines
            float Bx = int(playerX / (float)TILE_WIDTH)*(float)TILE_WIDTH;
            Bx += isRayFacingRight ? (float)TILE_WIDTH : 0;
            
            float By = playerY + (Bx - playerX)*(tan(rayAngle));
            
            float xstepVert = TILE_WIDTH;
            xstepVert *= isRayFacingLeft ? -1 : 1;
            
            float ystepVert = tan(rayAngle)*(float)TILE_WIDTH;
            ystepVert *= (isRayFacingUp && ystepVert > 0) ? -1 : 1;
            ystepVert *= (isRayFacingDown && ystepVert < 0) ? -1 : 1;
            
            float nextVertTouchX = Bx;
            float nextVertTouchY = By;
            
            
            while(nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT)
            { 
                uint32_t mapX = isRayFacingLeft ? (nextVertTouchX-1) / (float)TILE_WIDTH: nextVertTouchX / (float)TILE_WIDTH;
                uint32_t mapY = nextVertTouchY / (float)TILE_HEIGHT;
                //DrawCircle(renderer, nextHorizTouchX, nextHorizTouchY, 5);
                if(map[mapY][mapX] == 1)
                {
                    vertHit = true;
                    hittedXVert = nextVertTouchX;
                    hittedYVert = nextVertTouchY;
                    break;
                }
                else
                {
                    nextVertTouchX += xstepVert;
                    nextVertTouchY += ystepVert;
                }
            }
            
            // NOTE find closest
            float horizWallDist = (horizHit) ? sqrtf(pow((hittedX - playerX),2) + pow((hittedY - playerY),2)) : 1e30;
            float vertWallDist = (vertHit) ? sqrtf( pow((hittedXVert - playerX),2) + pow((hittedYVert - playerY),2) ) : 1e30;
            
            float eps = 0.0005f;;
            if((horizWallDist - vertWallDist) < eps)
            {
                rayInfo->wallX = hittedX;
                rayInfo->distance = horizWallDist;
            }
            else
            {
                rayInfo->wallX = hittedXVert;
                rayInfo->distance = vertWallDist;
            }
            
            if((horizWallDist - vertWallDist) < eps)
            {
                rayInfo->wallY = hittedY;
            }
            else
            {
                rayInfo->wallY = hittedYVert;
            }
            rayAngle += FOV / (float)RAYS_COUNT;
            rayAngle = remainder(rayAngle, 2*PI);
            if (rayAngle < 0)
            {
                rayAngle = 2*PI + rayAngle;
            }
        }
        
        
        SDL_UnlockTexture(texture);
        SDL_SetRenderDrawColor(renderer, 0xF0, 0xF0, 0xF0, 0xFF);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, 0, 0);
        
        //DrawCircle(renderer,playerX,playerY, 30);
        //DrawCircle(renderer,Ax,Ay, 3);
        //DrawCircle(renderer,Bx,By, 3);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
        
        if(angle)
        {
            cx = x*cos(angle) - y*sin(angle);
            cy = x*sin(angle) + y*cos(angle);
        }
        
        // NOTE player direction vector rendering
        //SDL_RenderDrawLine(renderer, playerX, playerY, playerX + cx*50, playerY +cy*50);
        for(uint32_t rayIndex = 0; rayIndex < RAYS_COUNT; rayIndex++)
        {
            RayInfo rayInfo = rays[rayIndex];
            float wallStripHeight = (TILE_WIDTH / rayInfo.distance)*distanceToProjectPlane;
            
            SDL_Rect wall{};
            wall.x = rayIndex * 1.0f;
            wall.y = (WINDOW_HEIGHT / 2.0f) - (wallStripHeight / 2.0f);
            wall.w = 1.0f;
            wall.h = wallStripHeight;
            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            SDL_RenderFillRect(renderer, &wall);
            
            //SDL_RenderDrawLine(renderer, playerX, playerY, rayInfo.wallX, rayInfo.wallY);
            //DrawCircle(renderer, rayInfo.wallX, rayInfo.wallY, 5);
        }
        SDL_RenderPresent(renderer);
    }
    
    
    printf("Hello world\n");
    system("pause");
    return 0;
}