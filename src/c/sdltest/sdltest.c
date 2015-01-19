#include <stdio.h>
#include "SDL.h"

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240
#define SPRITE_FILEPATH "/usr/lib/python2.7/dist-packages/pygame/pygame_icon.bmp"

#define DEBUG(...)
//#define DEBUG printf

void drawSprite(SDL_Surface* imageSurface, 
                SDL_Surface* screenSurface,
                int srcX, int srcY, 
                int dstX, int dstY,
                int width, int height)
{
   SDL_Rect srcRect;
   srcRect.x = srcX;
   srcRect.y = srcY;
   srcRect.w = width;
   srcRect.h = height;

   SDL_Rect dstRect;
   dstRect.x = dstX;
   dstRect.y = dstY;
   dstRect.w = width;
   dstRect.h = height;

   SDL_BlitSurface(imageSurface, &srcRect, screenSurface, &dstRect);
}


int main( int argc, char* args[] )
{
    SDL_Surface* screen = NULL;
    //Start SDL
    SDL_Init( SDL_INIT_VIDEO );
   const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo ();

   int systemX = videoInfo->current_w ;
   int systemY = videoInfo->current_h ;
   Uint8 bpp = videoInfo->vfmt->BitsPerPixel ;

    //Set up screen
    screen = SDL_SetVideoMode( WINDOW_WIDTH, WINDOW_HEIGHT, 16, SDL_HWSURFACE | SDL_DOUBLEBUF );
    //screen = SDL_SetVideoMode( systemX, systemY, bpp, SDL_SWSURFACE );
    
    DEBUG("(x,y,bbp)=(%d,%d,%d)\n", systemX, systemY, bpp);
    if (!screen)
    {
        printf("SDL_SetVideoMode failed\n");
        return 0;
    }
    SDL_ShowCursor(SDL_DISABLE);

    SDL_Rect r = {0,0,320,240};
    SDL_FillRect(screen,&r, SDL_MapRGB(screen->format, 200,200,0) );

	SDL_Surface* bitmap = SDL_LoadBMP(SPRITE_FILEPATH);
	if (bitmap)
		SDL_SetColorKey(bitmap, SDL_SRCCOLORKEY, SDL_MapRGB(bitmap->format, 255, 0, 255));
    SDL_Flip( screen );

    
    SDL_Event event;
    SDLKey keyPressed;
    int gameRunning = 1;

	while (gameRunning) {
		
		while (SDL_PollEvent(&event)) { 
         DEBUG("EVENT : %d\n", event.type);
         switch(event.type)
         {
			case SDL_QUIT:
				gameRunning = 0;
				break;
								
			case SDL_KEYDOWN:			
				keyPressed = event.key.keysym.sym;      
				if (SDLK_ESCAPE == keyPressed)
						gameRunning = 0;
				break;
				
            case SDL_MOUSEMOTION:
               DEBUG("Mouse moved by %d,%d to %d,%d\n", event.motion.xrel , event.motion.yrel, event.motion.x, event.motion.y);
               
               SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));


                drawSprite(bitmap, 
                 screen, 
                 0, 0,
                 event.motion.x, event.motion.y-32,
                 32,32); // width , height
                 
                 SDL_Flip( screen );
               break;
               
            case SDL_MOUSEBUTTONUP:
				DEBUG("Mouse but up\n");
               break;               

            case SDL_MOUSEBUTTONDOWN:
				DEBUG("Mouse but down\n");
               break;               
            
            }
        }

	}
	if (bitmap)
		SDL_FreeSurface(bitmap);


    //Quit SDL
    SDL_Quit();

    return 0;
}
