#ifdef WIN32
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#endif

//#include <cstdlib>
//#include <ctime>

#include "SDL.h"

#include "math.h"
#include "CmdLine.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

int JOY_DEADZONE = 500;

//Screen size
int WIDTH;
int HEIGHT;

//Button sizes
int BUTTON_SIZE = 10;
int UPPER_ROW_Y;
int MIDDLE_ROW_Y;
int LOWER_ROW_Y;

const int DASHBOARD_HEIGHT = BUTTON_SIZE + 2;

int particleCount = 0;
int penSize = 2;

int mbx;
int mby;

int canMoveX = 0;
int canMoveY = 0;

int speedX = 0;
int speedY = 0;

//Init and declare ParticleSwapping
bool implementParticleSwaps = true;



/* 
Enumerating concentions
-----------------------
Stillborn: between STILLBORN_UPPER_BOUND and STILLBORN_LOWER_BOUND
Floating: between FLOATING_UPPER_BOUND and FLOATING_LOWER_BOUND
*/
const int STILLBORN_UPPER_BOUND = 14;
const int STILLBORN_LOWER_BOUND = 1;
const int FLOATING_UPPER_BOUND = 35;
const int FLOATING_LOWER_BOUND = 32;

const int PARTICLETYPE_ENUM_LENGTH = 38;


enum ParticleType
{
	// STILLBORN
	NOTHING = 0,
	WALL = 1,
	IRONWALL = 2,
	TORCH = 3,
	//x = 4,
	STOVE = 5,
	ICE = 6,
	RUST = 7,
	EMBER = 8,
	PLANT = 9,
	VOID = 10,

	//SPOUTS
	WATERSPOUT = 11,
	SANDSPOUT = 12,
	SALTSPOUT = 13,
	OILSPOUT = 14,
	//x = 15,


	//ELEMENTAL
	WATER = 16,
	MOVEDWATER = 17,
	DIRT = 18,
	MOVEDDIRT = 19,
	SALT = 20,
	MOVEDSALT = 21,
	OIL = 22,
	MOVEDOIL = 23,
	SAND = 24,
	MOVEDSAND = 25,

	//COMBINED
	SALTWATER = 26,
	MOVEDSALTWATER = 27,
	MUD = 28,
	MOVEDMUD = 29,
	ACID = 30,
	MOVEDACID = 31,

	//FLOATING
	STEAM = 32,
	MOVEDSTEAM = 33,
	FIRE = 34,
	MOVEDFIRE = 35,

	//Electricity
	ELEC = 36,
	MOVEDELEC = 37
};

//Instead of using a two dimensional array
// we'll use a simple array to improve speed
// vs = virtual screen
ParticleType *vs;

// The current brush type
ParticleType CurrentParticleType = WALL;


//The number of buttons
const int BUTTON_COUNT = 19;

// Button rectangle struct
typedef struct
{
	SDL_Rect rect;
	ParticleType particleType;
} ButtonRect;

//An array of the buttons
ButtonRect Button[BUTTON_COUNT];

//The screen
SDL_Surface *screen;

// The particle system play area
SDL_Rect scene;


//Checks wether a given particle type is a stillborn element
static inline bool IsStillborn(ParticleType t)
{
	return (t >= STILLBORN_LOWER_BOUND && t <= STILLBORN_UPPER_BOUND);
}

//Checks wether a given particle type is a floting type - like FIRE and STEAM
static inline bool IsFloating(ParticleType t)
{
	return (t >= FLOATING_LOWER_BOUND && t <= FLOATING_UPPER_BOUND);
}

//Checks wether a given particle type is burnable - like PLANT and OIL
static inline bool IsBurnable(ParticleType t)
{
	return (t == PLANT || t == OIL || t == MOVEDOIL);
}

//Checks wether a given particle type is burnable - like PLANT and OIL
static inline bool BurnsAsEmber(ParticleType t)
{
	return (t == PLANT); //Maybe we'll add a FUSE or WOOD
}

// Setting a given piel to Uint32 color - in 16 bbp
inline void SetPixel16Bit(Uint16 x, Uint16 y, Uint32 pixel)
{
	*((Uint16 *)screen->pixels + y * screen->pitch/2 + x) = pixel;
}


static Uint32 colors[PARTICLETYPE_ENUM_LENGTH];

// Initializing colors
void initColors()
{
	//STILLBORN

	colors[SAND]		= SDL_MapRGB(screen->format, 238, 204, 128);
	colors[WALL]		= SDL_MapRGB(screen->format, 100, 100, 100);
	colors[VOID]		= SDL_MapRGB(screen->format, 60, 60, 60);
	colors[IRONWALL]	= SDL_MapRGB(screen->format, 110, 110, 110);
	colors[TORCH]		= SDL_MapRGB(screen->format, 139, 69, 19);
	colors[STOVE]		= SDL_MapRGB(screen->format, 74, 74, 74);		
	colors[ICE]			= SDL_MapRGB(screen->format, 175, 238, 238);		
	colors[PLANT]		= SDL_MapRGB(screen->format, 0, 150, 0);			
	colors[EMBER]		= SDL_MapRGB(screen->format, 127, 25, 25);		
	colors[RUST]		= SDL_MapRGB(screen->format, 110, 40, 10);		

	//ELEMENTAL
	colors[WATER]		= SDL_MapRGB(screen->format, 32, 32, 255);		
	colors[DIRT]		= SDL_MapRGB(screen->format, 205, 175, 149);		
	colors[SALT]		= SDL_MapRGB(screen->format, 255, 255, 255);		
	colors[OIL]			= SDL_MapRGB(screen->format, 128, 64, 64);			

	//COMBINED
	colors[MUD]			= SDL_MapRGB(screen->format, 139, 69, 19);			
	colors[SALTWATER]	= SDL_MapRGB(screen->format, 65, 105, 225);	
	colors[STEAM]		= SDL_MapRGB(screen->format, 95, 158, 160);		

	//EXTRA
	colors[ACID]		= SDL_MapRGB(screen->format, 173, 255, 47);		
	colors[FIRE]		= SDL_MapRGB(screen->format, 255, 50, 50);		
	colors[ELEC]		= SDL_MapRGB(screen->format, 255, 255, 0);

	//SPOUTS
	colors[WATERSPOUT]	= SDL_MapRGB(screen->format, 0, 0, 128);
	colors[SANDSPOUT]	= SDL_MapRGB(screen->format, 240, 230, 140);
	colors[SALTSPOUT]	= SDL_MapRGB(screen->format, 238, 233, 233);
	colors[OILSPOUT]	= SDL_MapRGB(screen->format, 108, 44, 44);

}


//Drawing our virtual screen to the real screen
static void DrawScene()
{
	particleCount = 0;

	//Locking the screen
	if ( SDL_MUSTLOCK(screen) )
	{
		if ( SDL_LockSurface(screen) < 0 )
		{
			return;
		}
	}

	//Clearing the screen with black
	SDL_FillRect(screen,&scene,0);

	//Iterating through each pixel height first
	for(int y=HEIGHT-DASHBOARD_HEIGHT;y--;)
	{
		//Width
		for(int x=WIDTH;x--;)
		{
			int index = x+(WIDTH*y);
			ParticleType same = vs[index];
			if(same != NOTHING)
			{
				if(IsStillborn(same))
					SetPixel16Bit(x,y,colors[same]);
				else
				{
					particleCount++;
					if(same % 2 == 1) //Moved
					{
						SetPixel16Bit(x,y,colors[(same-1)]);
						vs[index] = (ParticleType)(same-1); //Set it to not moved
					}
					else			//Not moved
						SetPixel16Bit(x,y,colors[same]);
				}
			}
		}
	}

	//Unlocking the screen
	if ( SDL_MUSTLOCK(screen) )
	{
		SDL_UnlockSurface(screen);
	}
}

// Emitting a given particletype at (x,o) width pixels wide and
// with a p density (probability that a given pixel will be drawn 
// at a given position withing the width)
void Emit(int x, int width, ParticleType type, float p)
{
	for (int i = x - width/2; i < x + width/2; i++)
	{
		if ( rand() < (int)(RAND_MAX * p) ) vs[i+WIDTH] = type;
	}
}

//Performs logic of stillborn particles
void StillbornParticleLogic(int x,int y,ParticleType type)
{
	int index, above, left, right, below, same, abovetwo;
	switch(type)
	{

	case VOID:
		above = x+((y-1)*WIDTH);
		left = (x+1)+(y*WIDTH);
		right = (x-1)+(y*WIDTH);
		below = x+((y+1)*WIDTH);
		if(vs[above] != NOTHING)
			vs[above] = NOTHING;
		if(vs[below] != NOTHING)
			vs[below] = NOTHING;
		if(vs[left] != NOTHING)
			vs[left] = NOTHING;
		if(vs[right] != NOTHING)
			vs[right] = NOTHING;
		break;
	case IRONWALL:
		above = x+((y-1)*WIDTH);
		left = (x+1)+(y*WIDTH);
		right = (x-1)+(y*WIDTH);
		if(rand()%200 == 0 && (vs[above] == RUST || vs[left] == RUST || vs[right] == RUST))
			vs[x+(y*WIDTH)] = RUST;
		break;
	case TORCH:
		above = x+((y-1)*WIDTH);
		left = (x+1)+(y*WIDTH);
		right = (x-1)+(y*WIDTH);
		if(rand()%2 == 0) // Spawns fire
		{
			if(vs[above] == NOTHING || vs[above] == MOVEDFIRE) //Fire above
				vs[above] = MOVEDFIRE;
			if(vs[right] == NOTHING || vs[right] == MOVEDFIRE) //Fire to the right
				vs[right] = MOVEDFIRE;
			if(vs[left] == NOTHING || vs[left] == MOVEDFIRE) //Fire to the left
				vs[left] = MOVEDFIRE;
		}
		if(vs[above] == MOVEDWATER || vs[above] == WATER) //Fire above
			vs[above] = MOVEDSTEAM;
		if(vs[right] == MOVEDWATER || vs[right] == WATER) //Fire to the right
			vs[right] = MOVEDSTEAM;
		if(vs[left] == MOVEDWATER || vs[left] == WATER) //Fire to the left
			vs[left] = MOVEDSTEAM;

		break;
	case PLANT:
		if(rand()%2 == 0) //Making the plant grow slowly
		{
			index = 0;
			switch(rand()%4)
			{
			case 0: index = (x-1)+(y*WIDTH); break;
			case 1: index = x+((y-1)*WIDTH); break;
			case 2: index = (x+1)+(y*WIDTH); break;
			case 3:	index = x+((y+1)*WIDTH); break;
			}
			if(vs[index] == WATER)
				vs[index] = PLANT;
		}
		break;
	case EMBER:
		below = x+((y+1)*WIDTH);
		if(vs[below] == NOTHING || IsBurnable(vs[below]))
			vs[below] = FIRE;

		index = 0;
		switch(rand()%4)
		{
		case 0: index = (x-1)+(y*WIDTH); break;
		case 1: index = x+((y-1)*WIDTH); break;
		case 2: index = (x+1)+(y*WIDTH); break;
		case 3:	index = x+((y+1)*WIDTH); break;
		}
		if(vs[index] == PLANT)
			vs[index] = FIRE;

		if(rand()%18 == 0) // Making ember burn out slowly
			vs[x+(y*WIDTH)] = NOTHING;
		break;
	case STOVE:
		above = x+((y-1)*WIDTH);
		abovetwo = x+((y-2)*WIDTH);
		if(rand()%4 == 0 && vs[above] == WATER) // Boil the water
			vs[above] = STEAM;
		if(rand()%4 == 0 && vs[above] == SALTWATER) // Saltwater separates
		{
		    vs[above] = SALT;
		    vs[abovetwo] = STEAM;
        }
		if(rand()%8 == 0 && vs[above] == OIL) // Set oil aflame
			vs[above] = EMBER;
		break;
	case RUST:
		if(rand()%7000 == 0)//Deteriate rust
			vs[x+(y*WIDTH)] = NOTHING;
		break;


		//####################### SPOUTS ####################### 
	case WATERSPOUT:
		if(rand()%6 == 0) // Take it easy on the spout
		{
			below = x+((y+1)*WIDTH);
			if (vs[below] == NOTHING)
				vs[below] = MOVEDWATER;
		}
		break;
	case SANDSPOUT:
		if(rand()%6 == 0) // Take it easy on the spout
		{
			below = x+((y+1)*WIDTH);
			if (vs[below] == NOTHING)
				vs[below] = MOVEDSAND;
		}
		break;
	case SALTSPOUT:
		if(rand()%6 == 0) // Take it easy on the spout
		{

			below = x+((y+1)*WIDTH);
			if (vs[below] == NOTHING)
				vs[below] = MOVEDSALT;
			if(vs[below] == WATER || vs[below] == MOVEDWATER)
			    vs[below] = MOVEDSALTWATER;
		}
		break;
	case OILSPOUT:
		if(rand()%6 == 0) // Take it easy on the spout
		{
			below = x+((y+1)*WIDTH);
			if (vs[below] == NOTHING)
				vs[below] = MOVEDOIL;
		}
		break;

	default:
		break;
	}

}



// Performing the movement logic of a given particle. The argument 'type'
// is passed so that we don't need a table lookup when determining the
// type to set the given particle to - i.e. if the particle is SAND then the
// passed type will be MOVEDSAND
inline void MoveParticle(int x, int y, ParticleType type)
{

	type = (ParticleType)(type+1);


	int above = x+((y-1)*WIDTH);
	int same = x+(WIDTH*y);
	int below = x+((y+1)*WIDTH);


	//If nothing below then just fall (gravity)
	if(!IsFloating(type))
	{
		if ( (vs[below] == NOTHING) && (rand() % 8)) //rand() % 8 makes it spread
		{
			vs[below] = type;
			vs[same] = NOTHING;
			return;
		}
	}
	else
	{
		if(rand()%3 == 0) //Slow down please
			return;

		//If nothing above then rise (floating - or reverse gravity? ;))
		if ((vs[above] == NOTHING || vs[above] == FIRE) && (rand() % 8) && (vs[same] != ELEC) && (vs[same] != MOVEDELEC)) //rand() % 8 makes it spread
		{
			if (type == MOVEDFIRE && rand()%20 == 0)
				vs[same] = NOTHING;
			else
			{
				vs[above] = vs[same];
				vs[same] = NOTHING;
			}
			return;
		}

	}

	//Randomly select right or left first
	int sign = rand() % 2 == 0 ? -1 : 1;

	// We'll only calculate these indicies once for optimization purpose
	int first = (x+sign)+(WIDTH*y);
	int second = (x-sign)+(WIDTH*y);

	int index = 0;
	//Particle type specific logic
	switch(type)
	{
	case MOVEDELEC:
		if(rand()%2 == 0)
			vs[same] = NOTHING;
		break;
	case MOVEDSTEAM:
		if(rand()%1000 == 0)
		{
			vs[same] = MOVEDWATER;
			return;
		}
		if(rand()%500 == 0)
		{
			vs[same] = NOTHING;
			return;
		}
		if(!IsStillborn(vs[above]) && !IsFloating(vs[above]))
		{
			if(rand()%15 == 0)
			{
				vs[same] = NOTHING;
				return;
			}
			else
			{
				vs[same] = vs[above];
				vs[above] = MOVEDSTEAM;
				return;
			}
		}
		break;
	case MOVEDFIRE:

		if(!IsBurnable(vs[above]) && rand()%10 == 0)
		{
			vs[same] = NOTHING;
			return;
		}

		// Let the snowman melt!
		if(rand()%4 == 0)
		{
			if (vs[above] == ICE)
			{
				vs[above] = WATER;
				vs[same] = NOTHING;
			}
			if (vs[below] == ICE)
			{
				vs[below] = WATER;
				vs[same] = NOTHING;
			}
			if (vs[first] == ICE)
			{
				vs[first] = WATER;
				vs[same] = NOTHING;
			}
			if (vs[second] == ICE)
			{
				vs[second] = WATER;
				vs[same] = NOTHING;
			}
		}

		//Let's burn whatever we can!
		index = 0;
		switch(rand()%4)
		{
		case 0: index = above; break;
		case 1: index = below; break;
		case 2: index = first; break;
		case 3:	index = second; break;
		}
		if(IsBurnable(vs[index]))
		{
			if(BurnsAsEmber(vs[index]))
				vs[index] = EMBER;
			else
				vs[index] = FIRE;
		}
		break;
	case MOVEDWATER:
		if(rand()%200 == 0 && vs[below] == IRONWALL)
			vs[below] = RUST;

		if(vs[below]  == FIRE || vs[above] == FIRE || vs[first] == FIRE || vs[second] == FIRE)
            vs[same] = MOVEDSTEAM;

		//Making water+dirt into dirt
		if(vs[below] == DIRT)
		{
			vs[below] = MOVEDMUD;
			vs[same] = NOTHING;
		}
		if(vs[above] == DIRT)
		{
			vs[above] = MOVEDMUD;
			vs[same] = NOTHING;
		}

		//Making water+salt into saltwater
		if(vs[above] == SALT || vs[above] == MOVEDSALT)
		{
			vs[above] = MOVEDSALTWATER;
			vs[same] = NOTHING;
		}
		if(vs[below] == SALT || vs[below] == MOVEDSALT)
		{
			vs[below] = MOVEDSALTWATER;
			vs[same] = NOTHING;
		}

		if(rand()%60 == 0) //Melting ice
		{
			switch(rand()%4)
			{
			case 0:	index = above; break;
			case 1:	index = below; break;
			case 2:	index = first; break;
			case 3:	index = second; break;
			}
			if(vs[index] == ICE)vs[index] = WATER; //--
		}
		break;
	case MOVEDACID:
		switch(rand()%4)
		{
		case 0:	index = above; break;
		case 1:	index = below; break;
		case 2:	index = first; break;
		case 3:	index = second; break;
		}
		if(vs[index] != WALL && vs[index] != IRONWALL && vs[index] != WATER && vs[index] != MOVEDWATER && vs[index] != ACID && vs[index] != MOVEDACID) vs[index] = NOTHING;	break;
		break;
	case MOVEDSALT:
		if(rand()%20 == 0)
		{
			switch(rand()%4)
			{
			case 0:	index = above; break;
			case 1:	index = below; break;
			case 2:	index = first; break;
			case 3:	index = second; break;
			}
			if(vs[index] == ICE)vs[index] = WATER; //--
		}
		break;
	case MOVEDSALTWATER:
		//Saltwater separated by heat
	//	if (vs[above] == FIRE || vs[below] == FIRE || vs[first] == FIRE || vs[second] == FIRE || vs[above] == STOVE || vs[below] == STOVE || vs[first] == STOVE || vs[second] == STOVE)
	//	{
	//		vs[same] = SALT;
	//		vs[above] = STEAM;
	//	}
		if(rand()%40 == 0) //Saltwater dissolves ice more slowly than pure salt
		{
			switch(rand()%4)
			{
			case 0:	index = above; break;
			case 1:	index = below; break;
			case 2:	index = first; break;
			case 3:	index = second; break;
			}
			if(vs[index] == ICE)vs[index] = WATER;
		}
		break;
	case MOVEDOIL:
		switch(rand()%4)
		{
		case 0:	index = above; break;
		case 1:	index = below; break;
		case 2:	index = first; break;
		case 3:	index = second; break;
		}
		if(vs[index] == FIRE)
			vs[same] = FIRE;
		break;

	default:
		break;
	}

	//Peform 'realism' logic?
	// When adding dynamics to this part please use the following structure:
	// If a particle A is ligther than particle B then add vs[above] == B to the condition in case A (case MOVED_A)
	if(implementParticleSwaps)
	{
		switch(type)
		{
		case MOVEDWATER:
			if(vs[above] == SAND || vs[above] == MUD || vs[above] == SALTWATER && rand()%3 == 0)
			{
				vs[same] = vs[above];
				vs[above] = type;
				return;
			}
			break;
		case MOVEDOIL:
			if(vs[above] == WATER && rand()%3 == 0)
			{
				vs[same] = vs[above];
				vs[above] = type;
				return;
			}
			break;
		case MOVEDSALTWATER:
			if(vs[above] == DIRT || vs[above] == MUD || vs[above] == SAND && rand()%3 == 0)
			{
				vs[same] = vs[above];
				vs[above] = type;
				return;
			}
			break;

		default:
			break;
		}
	}


	// The place below (x,y+1) is filled with something, then check (x+sign,y+1) and (x-sign,y+1) 
	// We chose sign randomly to randomly check eigther left or right
	// This is for elements that fall downward
	if (!IsFloating(type))
	{
		int firstdown = (x+sign)+((y+1)*WIDTH);
		int seconddown = (x-sign)+((y+1)*WIDTH);

		if ( vs[firstdown] == NOTHING)
		{
			vs[firstdown] = type;
			vs[same] = NOTHING;
		}
		else if ( vs[seconddown] == NOTHING)
		{
			vs[seconddown] = type;
			vs[same] = NOTHING;
		}
		//If (x+sign,y+1) is filled then try (x+sign,y) and (x-sign,y)
		else if (vs[first] == NOTHING)
		{
			vs[first] = type;
			vs[same] = NOTHING;
		}
		else if (vs[second] == NOTHING)
		{
			vs[second] = type;
			vs[same] = NOTHING;
		}
	}
	// Make steam move
	else if(type == MOVEDSTEAM)
	{
		int firstup = (x+sign)+((y-1)*WIDTH);
		int secondup = (x-sign)+((y-1)*WIDTH);

		if ( vs[firstup] == NOTHING)
		{
			vs[firstup] = type;
			vs[same] = NOTHING;
		}
		else if ( vs[secondup] == NOTHING)
		{
			vs[secondup] = type;
			vs[same] = NOTHING;
		}
		//If (x+sign,y+1) is filled then try (x+sign,y) and (x-sign,y)
		else if (vs[first] == NOTHING)
		{
			vs[first] = type;
			vs[same] = NOTHING;
		}
		else if (vs[second] == NOTHING)
		{
			vs[second] = type;
			vs[same] = NOTHING;
		}
	}
}



//Drawing a filled circle at a given position with a given radius and a given partice type
void DrawParticles(int xpos, int ypos, int radius, ParticleType type)
{
	for (int x = ((xpos - radius - 1) < 0) ? 0 : (xpos - radius - 1); x <= xpos + radius && x < WIDTH; x++)
		for (int y = ((ypos - radius - 1) < 0) ? 0 : (ypos - radius - 1); y <= ypos + radius && y < HEIGHT; y++)
		{
			if ((x-xpos)*(x-xpos) + (y-ypos)*(y-ypos) <= radius*radius) vs[x+(WIDTH*y)] = type;
		};
}

// Drawing a line
void DrawLine(int newx, int newy, int oldx, int oldy)
{
	if(newx == oldx && newy == oldy)
	{
		DrawParticles(newx,newy,penSize,CurrentParticleType);
	}
	else
	{
		float step = 1.0f / ((abs(newx-oldx)>abs(newy-oldy)) ? abs(newx-oldx) : abs(newy-oldy));
		for (float a = 0; a < 1; a+=step)
			DrawParticles(a*newx+(1-a)*oldx,a*newy+(1-a)*oldy,penSize,CurrentParticleType); 
	}
}


// Drawing some random lines
void DoRandomLines(ParticleType type)
{
	ParticleType tmp = CurrentParticleType;
	CurrentParticleType = type;
	for(int i = 0; i < 20; i++)
	{
		int x1 = rand() % WIDTH;
		int x2 = rand() % WIDTH;

		DrawLine(x1,0,x2,HEIGHT);
	}

	for(int i = 0; i < 20; i++)
	{
		int y1 = rand() % HEIGHT;
		int y2 = rand() % HEIGHT;

		DrawLine(0,y1,WIDTH,y2);
	}
	CurrentParticleType = tmp;
}

// Updating a virtual pixel
inline void UpdateVirtualPixel(int x, int y)
{

	ParticleType same = vs[x+(WIDTH*y)];
	if(same != NOTHING)
	{
		if(IsStillborn(same))
			StillbornParticleLogic(x,y,same);
		else
			if ( rand() >= RAND_MAX / 13 && same % 2 == 0) MoveParticle(x,y,same); //THe rand condition makes the particles fall unevenly
	}

}

// Updating the particle system (virtual screen) pixel by pixel
inline void UpdateVirtualScreen()
{
	for(int y =0; y< HEIGHT-DASHBOARD_HEIGHT; y++)
	{
		// Due to biasing when iterating through the scanline from left to right,
		// we now chose our direction randomly per scanline.
		if (rand() % 2 == 0)
			for(int x = WIDTH-2; x--;) UpdateVirtualPixel(x,y);
		else
			for(int x = 1; x < WIDTH - 1; x++) UpdateVirtualPixel(x,y);
	}
}


//Cearing the particle system
void Clear()
{
	for(int w = 0; w < WIDTH ; w++)
	{
		for(int h = 0; h < HEIGHT; h++)
		{
			vs[w+(WIDTH*h)] = NOTHING;
		}
	}
}

void InitButtons()
{

	//set up water emit button
	SDL_Rect wateroutput ;
	wateroutput.x = 1;
	wateroutput.y = UPPER_ROW_Y;
	wateroutput.w = BUTTON_SIZE;
	wateroutput.h = BUTTON_SIZE;
	SDL_FillRect(screen,&wateroutput,colors[WATER]);
	ButtonRect warect;
	warect.rect = wateroutput;
	warect.particleType = WATER;
	Button[0] = warect;


	//set up sand sand button
	SDL_Rect sandoutput ;
	sandoutput.x = BUTTON_SIZE + 1;
	sandoutput.y = UPPER_ROW_Y;
	sandoutput.w = BUTTON_SIZE;
	sandoutput.h = BUTTON_SIZE;
	SDL_FillRect(screen,&sandoutput,colors[SAND]);
	ButtonRect sanrect;
	sanrect.rect = sandoutput;
	sanrect.particleType = SAND;
	Button[1] = sanrect;

	//set up salt emit button
	SDL_Rect saltoutput ;
	saltoutput.x = BUTTON_SIZE*2 + 1;
	saltoutput.y = UPPER_ROW_Y;
	saltoutput.w = BUTTON_SIZE;
	saltoutput.h = BUTTON_SIZE;
	SDL_FillRect(screen,&saltoutput,colors[SALT]);
	ButtonRect sarect;
	sarect.rect = saltoutput;
	sarect.particleType = SALT;
	Button[2] = sarect;

	//set up oil button
	SDL_Rect oiloutput ;
	oiloutput.x = BUTTON_SIZE*3 + 1;
	oiloutput.y = UPPER_ROW_Y;
	oiloutput.w = BUTTON_SIZE;
	oiloutput.h = BUTTON_SIZE;
	SDL_FillRect(screen,&oiloutput,colors[OIL]);
	ButtonRect oirect;
	oirect.rect = oiloutput;
	oirect.particleType = OIL;
	Button[3] = oirect;

	//set up fire button
	SDL_Rect fire ;
	fire.x = BUTTON_SIZE*4 + 1;
	fire.y = UPPER_ROW_Y;
	fire.w = BUTTON_SIZE;
	fire.h = BUTTON_SIZE;
	SDL_FillRect(screen,&fire,colors[FIRE]);
	ButtonRect firect;
	firect.particleType = FIRE;
	firect.rect = fire;
	Button[4] = firect;

	//set up acid button
	SDL_Rect acid ;
	acid.x = BUTTON_SIZE*5 + 1;
	acid.y = UPPER_ROW_Y;
	acid.w = BUTTON_SIZE;
	acid.h = BUTTON_SIZE;
	SDL_FillRect(screen,&acid,colors[ACID]);
	ButtonRect acrect;
	acrect.particleType = ACID;
	acrect.rect = acid;
	Button[5] = acrect;

	//set up dirt emit button
	SDL_Rect dirtoutput ;
	dirtoutput.x = BUTTON_SIZE*6 + 1;
	dirtoutput.y = UPPER_ROW_Y;
	dirtoutput.w = BUTTON_SIZE;
	dirtoutput.h = BUTTON_SIZE;
	SDL_FillRect(screen,&dirtoutput,colors[DIRT]);
	ButtonRect direct;
	direct.rect = dirtoutput;
	direct.particleType = DIRT;
	Button[6] = direct;

	//set up spout water
	SDL_Rect spwater ;
	spwater.x = 75 + 1;
	spwater.y = MIDDLE_ROW_Y;
	spwater.w = BUTTON_SIZE;
	spwater.h = BUTTON_SIZE;
	SDL_FillRect(screen,&spwater,colors[WATERSPOUT]);
	ButtonRect spwrect;
	spwrect.particleType = WATERSPOUT;
	spwrect.rect = spwater;
	Button[7] = spwrect;

	//set up spout sand
	SDL_Rect spdirt ;
	spdirt.x = 75 + BUTTON_SIZE + 1;
	spdirt.y = MIDDLE_ROW_Y;
	spdirt.w = BUTTON_SIZE;
	spdirt.h = BUTTON_SIZE;
	SDL_FillRect(screen,&spdirt,colors[SANDSPOUT]);
	ButtonRect spdrect;
	spdrect.particleType = SANDSPOUT;
	spdrect.rect = spdirt;
	Button[8] = spdrect;

	//set up spout salt
	SDL_Rect spsalt ;
	spsalt.x = 75 + BUTTON_SIZE*2 + 1;
	spsalt.y = MIDDLE_ROW_Y;
	spsalt.w = BUTTON_SIZE;
	spsalt.h = BUTTON_SIZE;
	SDL_FillRect(screen,&spsalt,colors[SALTSPOUT]);
	ButtonRect spsrect;
	spsrect.particleType = SALTSPOUT;
	spsrect.rect = spsalt;
	Button[9] = spsrect;

	//set up spout oil
	SDL_Rect spoil ;
	spoil.x = 75 + BUTTON_SIZE*3 + 1;
	spoil.y = MIDDLE_ROW_Y;
	spoil.w = BUTTON_SIZE;
	spoil.h = BUTTON_SIZE;
	SDL_FillRect(screen,&spoil,colors[OILSPOUT]);
	ButtonRect sporect;
	sporect.particleType = OILSPOUT;
	sporect.rect = spoil;
	Button[10] = sporect;

	//set up wall button
	SDL_Rect wall ;
	wall.x = 120 + 1;
	wall.y = LOWER_ROW_Y;
	wall.w = BUTTON_SIZE;
	wall.h = BUTTON_SIZE;
	SDL_FillRect(screen,&wall,colors[WALL]);
	ButtonRect walrect;
	walrect.particleType = WALL;
	walrect.rect = wall;
	Button[11] = walrect;

	//set up torch button
	SDL_Rect torch ;
	torch.x = 120 + BUTTON_SIZE + 1;
	torch.y = LOWER_ROW_Y;
	torch.w = BUTTON_SIZE;
	torch.h = BUTTON_SIZE;
	SDL_FillRect(screen,&torch,colors[TORCH]);
	ButtonRect torect;
	torect.particleType = TORCH;
	torect.rect = torch;
	Button[12] = torect;

	//set up stove button
	SDL_Rect stove ;
	stove.x = 120 + BUTTON_SIZE*2 + 1;
	stove.y = LOWER_ROW_Y;
	stove.w = BUTTON_SIZE;
	stove.h = BUTTON_SIZE;
	SDL_FillRect(screen,&stove,colors[STOVE]);
	ButtonRect storect;
	storect.particleType = STOVE;
	storect.rect = stove;
	Button[13] = storect;

	//set up plant button
	SDL_Rect plant ;
	plant.x = 120 + BUTTON_SIZE*3 + 1;
	plant.y = LOWER_ROW_Y;
	plant.w = BUTTON_SIZE;
	plant.h = BUTTON_SIZE;
	SDL_FillRect(screen,&plant,colors[PLANT]);
	ButtonRect prect;
	prect.particleType = PLANT;
	prect.rect = plant;
	Button[14] = prect;

	//ICE
	SDL_Rect ice ;
	ice.x = 120 + BUTTON_SIZE*4 + 1;
	ice.y = LOWER_ROW_Y;
	ice.w = BUTTON_SIZE;
	ice.h = BUTTON_SIZE;
	SDL_FillRect(screen,&ice,colors[ICE]);
	ButtonRect irect;
	irect.particleType = ICE;
	irect.rect = ice;
	Button[15] = irect;

	//IRONWALL
	SDL_Rect iw ;
	iw.x = 120 + BUTTON_SIZE*5 + 1;
	iw.y = LOWER_ROW_Y;
	iw.w = BUTTON_SIZE;
	iw.h = BUTTON_SIZE;
	SDL_FillRect(screen,&iw,colors[IRONWALL]);
	ButtonRect iwrect;
	iwrect.particleType = IRONWALL;
	iwrect.rect = iw;
	Button[16] = iwrect;

	//VOID
	SDL_Rect voidele ;
	voidele.x = 120 + BUTTON_SIZE*6 + 1;
	voidele.y = LOWER_ROW_Y;
	voidele.w = BUTTON_SIZE;
	voidele.h = BUTTON_SIZE;
	SDL_FillRect(screen,&voidele,colors[VOID]);
	ButtonRect voidelerect;
	voidelerect.particleType = VOID;
	voidelerect.rect = voidele;
	Button[17] = voidelerect;

	//eraser
	SDL_Rect eraser ;
	eraser.x = 195 + 1;
	eraser.y = LOWER_ROW_Y;
	eraser.w = BUTTON_SIZE;
	eraser.h = BUTTON_SIZE;
	SDL_FillRect(screen,&eraser,0);
	ButtonRect eraserrect;
	eraserrect.particleType = NOTHING;
	eraserrect.rect = eraser;
	Button[18] = eraserrect;

	for(int i = 0; i< BUTTON_COUNT; i++)
		SDL_UpdateRect ( screen , Button[i].rect.x , Button[i].rect.y , Button[i].rect.w , Button[i].rect.h ) ;

}





// Initializing the screen
void init()
{

	// Initializing SDL
	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 )
	{
		fprintf( stderr, "Video initialization failed: %s\n",
			SDL_GetError( ) );
		SDL_Quit( );
	}

	if(SDL_NumJoysticks() > 0)
		SDL_JoystickOpen(0);

	SDL_ShowCursor(0);

	//Creating the screen using 16 bit colors
	screen = SDL_SetVideoMode(WIDTH, HEIGHT, 16, SDL_HWSURFACE|SDL_TRIPLEBUF);
	if ( screen == NULL ) {
		fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());

	}

	//Setting caption
	SDL_WM_SetCaption("SDL Sand - http://sourceforge.net/projects/sdlsand",NULL);
	//Enabeling key repeats
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	initColors();

	scene.x = 0;
	scene.y = 0;
	scene.w = WIDTH;
	scene.h = HEIGHT-DASHBOARD_HEIGHT;

	//set up dashboard
	SDL_Rect dashboard ;
	dashboard.x = 0;
	dashboard.y = HEIGHT-DASHBOARD_HEIGHT;
	dashboard.w = WIDTH;
	dashboard.h = DASHBOARD_HEIGHT;
	SDL_FillRect(screen,&dashboard, SDL_MapRGB(screen->format, 155, 155, 155));
	SDL_UpdateRect ( screen , dashboard.x , dashboard.y , dashboard.w , dashboard.h ) ;

	//Initializing the buttons
	InitButtons();


}


inline void CheckGuiInteraction(int mbx, int mby)
{
	for(int i = BUTTON_COUNT; i--;)
	{
		ButtonRect r = Button[i];
		if(mbx > r.rect.x && mbx <= r.rect.x+r.rect.w && mby > r.rect.y && mby <= r.rect.y+r.rect.h)
			CurrentParticleType = r.particleType;
	}
}

void drawSelection()
{
	for(int i = BUTTON_COUNT; i--;)
	{
		ButtonRect r = Button[i];

		SDL_Rect top = {r.rect.x, r.rect.y, r.rect.w + 1, 1};
		SDL_Rect bottom = {r.rect.x, r.rect.y + r.rect.h, r.rect.w + 1, 1};
		SDL_Rect left = {r.rect.x, r.rect.y, 1, r.rect.h + 1};
		SDL_Rect right = {r.rect.x + r.rect.w, r.rect.y, 1, r.rect.h + 1};

		if (CurrentParticleType == r.particleType)
		{
			SDL_FillRect(screen, &top, SDL_MapRGB(screen->format, 255, 0, 255));
			SDL_FillRect(screen, &bottom, SDL_MapRGB(screen->format, 255, 0, 255));
			SDL_FillRect(screen, &left, SDL_MapRGB(screen->format, 255, 0, 255));
			SDL_FillRect(screen, &right, SDL_MapRGB(screen->format, 255, 0, 255));
		}
		else
		{
			SDL_FillRect(screen, &top, SDL_MapRGB(screen->format, 0, 0, 0));
			SDL_FillRect(screen, &bottom, SDL_MapRGB(screen->format, 0, 0, 0));
			SDL_FillRect(screen, &left, SDL_MapRGB(screen->format, 0, 0, 0));
			SDL_FillRect(screen, &right, SDL_MapRGB(screen->format, 0, 0, 0));
		}
	}
}

void drawCursor(int x, int y)
{
	SDL_Rect partHorizontal = { x+1, y+1, 4, 1 };
	SDL_Rect partVertical = { x+1, y+1, 1, 4 };

	SDL_FillRect(screen, &partHorizontal, SDL_MapRGB(screen->format, 255, 0, 255));
	SDL_FillRect(screen, &partVertical, SDL_MapRGB(screen->format, 255, 0, 255));
}

void drawPenSize()
{
	SDL_Rect size = { WIDTH - BUTTON_SIZE, HEIGHT - BUTTON_SIZE - 1, 0, 0 };

	switch(penSize)
	{
		case 1:
			size.w = 1;
			size.h = 1;
		break;
		case 2:
			size.w = 2;
			size.h = 2;
		break;
		case 4:
			size.w = 3;
			size.h = 3;
		break;
		case 8:
			size.w = 5;
			size.h = 5;
		break;
		case 16:
			size.w = 7;
			size.h = 7;
		break;
		case 32:
			size.w = 9;
			size.h = 9;
		break;

		default:
		break;
	}

	SDL_FillRect(screen, &size, SDL_MapRGB(screen->format, 0, 0, 0));
}


int main(int argc, char **argv)
{

	CCmdLine cmdLine;

	// parse the command line.
	if (cmdLine.SplitLine(argc, argv) < 1)
	{
		// no switches were given on the command line
		//Set default size
		HEIGHT = 240;
		WIDTH = 320;
	}
	else
	{
		// StringType is defined in CmdLine.h.
		// it is CString when using MFC, else STL's 'string'
		StringType height, width;

		// get the required arguments
		try
		{
			// if any of these GetArgument calls fail,
			// we'll end up in the catch() block

			height = cmdLine.GetArgument("-height", 0);

			width = cmdLine.GetArgument("-width", 0);
			HEIGHT = atoi(height.c_str());
			WIDTH = atoi(width.c_str());
		}
		catch (...)
		{
			// one of the required arguments was missing, abort
			exit(-1);
		}
	}

	UPPER_ROW_Y = HEIGHT - BUTTON_SIZE - 1;
	MIDDLE_ROW_Y = HEIGHT - BUTTON_SIZE - 1;
	LOWER_ROW_Y = HEIGHT - BUTTON_SIZE - 1;

	vs = new ParticleType[HEIGHT*WIDTH];


	init();
	Clear();

	int tick = 0;
	int done=0;

	//To emit or not to emit
	bool emitWater = true;
	bool emitSand = true;
	bool emitSalt = true;
	bool emitOil = true;

	//Initial density of emitters
	float waterDens = 0.3f;
	float sandDens = 0.3f;
	float saltDens = 0.3f;
	float oilDens = 0.3f;

	// Set initial seed
	srand( (unsigned)time( NULL ) );

	int oldx = WIDTH/2, oldy = HEIGHT/2;

	//Mouse button pressed down?
	bool down = false;

	//Used for calculating the average FPS from NumFrames
	const int NumFrames = 20;
	int AvgFrameTimes[NumFrames];
	for( int i = 0; i < NumFrames; i++)
		AvgFrameTimes[i] = 0;
	int FrameTime = 0;
	int PrevFrameTime = 0;
	int Index = 0;

	int slow = false;

	//The game loop
	while(done == 0)
	{
		tick++;

		SDL_Event event;
		//Polling events
		while ( SDL_PollEvent(&event) )
		{

			if ( event.type == SDL_QUIT )  {  done = 1;  }
			//Key strokes
			if ( event.type == SDL_KEYDOWN )
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE: //Exit
					done = 1;
					break;
				case SDLK_RETURN:
					Clear();
					break;
				case SDLK_LEFT:
					for(int i = BUTTON_COUNT; i--;)
					{
						if(CurrentParticleType == Button[i].particleType)
						{
							if(i > 0)
							{
								CurrentParticleType = Button[i-1].particleType;
							}
							else
							{
								CurrentParticleType = Button[BUTTON_COUNT-1].particleType;
							}

							break;
						}
					}
					break;
				case SDLK_RIGHT:
					for(int i = BUTTON_COUNT; i--;)
					{
						if(CurrentParticleType == Button[i].particleType)
						{
							if(i < BUTTON_COUNT - 1)
							{
								CurrentParticleType = Button[i+1].particleType;
							}
							else
							{
								CurrentParticleType = Button[0].particleType;
							}

							break;
						}
					}
					break;
				case SDLK_UP: // Increase pen size
					penSize *= 2;
					if (penSize > 32)
						penSize = 32;
					break;
				case SDLK_DOWN: // Decrease pen size
					penSize /= 2;
					if(penSize < 1)
						penSize = 1;
					break;
				case SDLK_LCTRL:	// A
					mbx = oldx;
					mby = oldy;

					if(oldy < (HEIGHT-DASHBOARD_HEIGHT))
						DrawLine(oldx+speedX,oldy+speedY,oldx,oldy);

					down = true;
					break;
				case SDLK_LALT:
					slow = true;
					break;
				case SDLK_LSHIFT:
					emitOil ^= true;
					emitSalt ^= true;
					emitWater ^= true;
					emitSand ^= true;
					break;
				case SDLK_SPACE:
					CurrentParticleType = NOTHING;
					break;
				case SDLK_TAB:
					oilDens -= 0.05f;
					if(oilDens < 0.05f)
						oilDens = 0.05f;
					saltDens -= 0.05f;
					if(saltDens < 0.05f)
						saltDens = 0.05f;
					waterDens -= 0.05f;
					if(waterDens < 0.05f)
						waterDens = 0.05f;
					sandDens -= 0.05f;
					if(sandDens < 0.05f)
						sandDens = 0.05f;
					break;
				case SDLK_BACKSPACE:
					oilDens += 0.05f;
					if(oilDens > 1.0f)
						oilDens = 1.0f;
					saltDens += 0.05f;
					if(saltDens > 1.0f)
						saltDens = 1.0f;
					waterDens += 0.05f;
					if(waterDens > 1.0f)
						waterDens = 1.0f;
					sandDens += 0.05f;
					if(sandDens > 1.0f)
						sandDens = 1.0f;
					break;

				default:
					break;
				}
//				{
//				case SDLK_ESCAPE: //Exit
//					done = 1;
//					break;
//				case SDLK_0: // Eraser
//					CurrentParticleType = NOTHING;
//					break;
//				case SDLK_2: // Draw water
//					CurrentParticleType = WATER;
//					break;
//				case SDLK_3: // Draw dirt		
//					CurrentParticleType = SAND;
//					break;
//				case SDLK_4: // Draw salt
//					CurrentParticleType = SALT;
//					break;
//				case SDLK_5: // Draw oil
//					CurrentParticleType = OIL;
//					break;
//				case SDLK_6: // Draw acid
//					CurrentParticleType = ACID;
//					break;
//				case SDLK_7: // Draw fire
//					CurrentParticleType = FIRE;
//					break;
//				case SDLK_8: // Draw electricity
//					CurrentParticleType = ELEC;
//					break;
//				case SDLK_9: // Draw torch
//					CurrentParticleType = TORCH;
//					break;
//				case SDLK_1: // Draw wall		
//					CurrentParticleType = WALL;
//					break;
//				case SDLK_F1: // Draw mud
//					CurrentParticleType = MUD;
//					break;
//				case SDLK_F2: // Draw saltwater
//					CurrentParticleType = SALTWATER;
//					break;
//				case SDLK_F3: // Draw steam
//					CurrentParticleType = STEAM;
//					break;
//				case SDLK_F4: // Draw ice
//					CurrentParticleType = ICE;
//					break;
//				case SDLK_UP: // Increase pen size
//					penSize *= 2;
//					if (penSize > 40)
//						penSize = 40;
//					break;
//				case SDLK_DOWN: // Decrease pen size
//					penSize /= 2;
//					if(penSize < 1)
//						penSize = 1;
//					break;
//				case SDLK_DELETE: // Clear screen
//					Clear();
//					break;
//				case SDLK_v: //Enable or disable oil emitter
//					emitOil ^= true;
//					break;
//				case SDLK_r: // Increase oil emitter density
//					oilDens += 0.05f;
//					if(oilDens > 1.0f)
//						oilDens = 1.0f;
//					break;
//				case SDLK_f: // Decrease oil emitter density
//					oilDens -= 0.05f;
//					if(oilDens < 0.05f)
//						oilDens = 0.05f;
//					break;
//				case SDLK_c: //Enable or disable salt emitter
//					emitSalt ^= true;
//					break;
//				case SDLK_e: // Increase salt emitter density
//					saltDens += 0.05f;
//					if(saltDens > 1.0f)
//						saltDens = 1.0f;
//					break;
//				case SDLK_d: // Decrease salt emitter density
//					saltDens -= 0.05f;
//					if(saltDens < 0.05f)
//						saltDens = 0.05f;
//					break;
//				case SDLK_z: //Enable or disable water emitter
//					emitWater ^= true;
//					break;
//				case SDLK_q: // Increase water emitter density
//					waterDens += 0.05f;
//					if(waterDens > 1.0f)
//						waterDens = 1.0f;
//					break;
//				case SDLK_a: // Decrease water emitter density
//					waterDens -= 0.05f;
//					if(waterDens < 0.05f)
//						waterDens = 0.05f;
//					break;
//				case SDLK_w: // Increase dirt emitter density
//					sandDens += 0.05f;
//					if(sandDens > 1.0f)
//						sandDens = 1.0f;
//					break;
//				case SDLK_s: // Decrease dirt emitter density
//					sandDens -= 0.05f;
//					if(sandDens < 0.05f)
//						sandDens = 0.05f;
//					break;
//				case SDLK_x: //Enable or disable dirt emitter
//					emitSand ^= true;
//					break;
//				case SDLK_t: // Draw a bunch of random lines
//					DoRandomLines(WALL);
//					break;
//				case SDLK_y: // Erase a bunch of random lines
//					DoRandomLines(NOTHING);
//					break;
//				case SDLK_o: // Enable or disable particle swaps
//					implementParticleSwaps ^= true;
//					break;
//				}
			}
			if ( event.type == SDL_KEYUP )
			{
				switch (event.key.keysym.sym)
				{
					case SDLK_LCTRL:
						mbx = 0;
						mby = 0;
						down = false;
						break;

					case SDLK_LALT:
						slow = false;
						break;

					default:
						break;
				}
			}
			// Analog joystick movement
			if(event.type == SDL_JOYAXISMOTION)
			{
				switch(event.jaxis.axis)
				{
					case 0:		// axis 0 (left-right)
						if(event.jaxis.value < -JOY_DEADZONE && canMoveX)
						{
							// left movement
							speedX = (6 * event.jaxis.value / 65535);
						}
						else if(event.jaxis.value > JOY_DEADZONE && canMoveX)
						{
							// right movement
							speedX = (6 * event.jaxis.value / 65535);
						}
						else if(event.jaxis.value > -JOY_DEADZONE && event.jaxis.value < JOY_DEADZONE)
						{
							speedX = 0;
							canMoveX = 1;
						}
					break;
					case 1:		// axis 1 (up-down)
						if(event.jaxis.value < -JOY_DEADZONE && canMoveY)
						{
							// up movement
							speedY = (6 * event.jaxis.value / 65535);
						}
						else if(event.jaxis.value > JOY_DEADZONE && canMoveY)
						{
							// down movement
							speedY = (6 * event.jaxis.value / 65535);
						}
						else if(event.jaxis.value > -JOY_DEADZONE && event.jaxis.value < JOY_DEADZONE)
						{
							speedY = 0;
							canMoveY = 1;
						}
					break;

					default:
					break;
				}
			}
			// If mouse button pressed then save position of cursor
			if( event.type == SDL_MOUSEBUTTONDOWN)
			{
				SDL_MouseButtonEvent mbe = (SDL_MouseButtonEvent) event.button;
				oldx = mbe.x; oldy=mbe.y;
				mbx = mbe.x;
				mby = mbe.y;
				if(mbe.x < (HEIGHT-DASHBOARD_HEIGHT))
					DrawLine(mbe.x,mbe.y,oldx,oldy);
				down = true;
			}
			// Button released
			if(event.type == SDL_MOUSEBUTTONUP)
			{
				SDL_MouseButtonEvent mbe = (SDL_MouseButtonEvent) event.button;
				if(oldy < (HEIGHT-DASHBOARD_HEIGHT))
					DrawLine(mbe.x,mbe.y,oldx,oldy);
				mbx = 0;
				mby = 0;
				down = false;

			}
			// Mouse has moved
			if(event.type == SDL_MOUSEMOTION)
			{
				SDL_MouseMotionEvent mme = (SDL_MouseMotionEvent) event.motion;
				if(mme.state & SDL_BUTTON(1))
					DrawLine(mme.x,mme.y,oldx,oldy);
				oldx = mme.x; oldy=mme.y;
			}
			if(mby > HEIGHT-DASHBOARD_HEIGHT)
				CheckGuiInteraction(mbx,mby);
		}

		if(slow)
		{
			if(speedX > 0)
				oldx += 1;
			else if(speedX < 0)
				oldx += -1;
			if(speedY > 0)
				oldy += 1;
			else if(speedY < 0)
				oldy += -1;
		}
		else
		{
			oldx += speedX;
			oldy += speedY;
		}

		if(oldx < 0)
			oldx = 0;
		else if(oldx > WIDTH)
			oldx = WIDTH;

		if(oldy < 0)
			oldy = 0;
		else if(oldy > HEIGHT)
			oldy = HEIGHT;

		//To emit or not to emit
		if(emitWater)
			Emit((WIDTH/2 - ((WIDTH/6)*2)), 20, WATER, waterDens);
		if(emitSand)
			Emit((WIDTH/2 - (WIDTH/6)), 20, SAND, sandDens);
		if(emitSalt)
			Emit((WIDTH/2 + (WIDTH/6)), 20, SALT, saltDens);
		if(emitOil)
			Emit((WIDTH/2 + ((WIDTH/6)*2)), 20, OIL, oilDens);

		//If the button is pressed (and no event has occured since last frame due
		// to the polling procedure, then draw at the position (enabeling 'dynamic emitters')
		if(down)
			DrawLine(oldx,oldy,oldx,oldy);

		//Clear bottom line
		for (int i=0; i< WIDTH; i++) vs[i+((HEIGHT-DASHBOARD_HEIGHT-1)*WIDTH)] = NOTHING;
		//Clear top line
		for (int i=0; i< WIDTH; i++) vs[i+((0)*WIDTH)] = NOTHING;

		// Update the virtual screen (performing particle logic)
		UpdateVirtualScreen();
		// Map the virtual screen to the real screen
		DrawScene();
		// Update dashboard
		SDL_Rect dashboard ;
		dashboard.x = 0;
		dashboard.y = HEIGHT-DASHBOARD_HEIGHT;
		dashboard.w = WIDTH;
		dashboard.h = DASHBOARD_HEIGHT;
		SDL_FillRect(screen,&dashboard, SDL_MapRGB(screen->format, 155, 155, 155));
		SDL_UpdateRect ( screen , dashboard.x , dashboard.y , dashboard.w , dashboard.h ) ;
		InitButtons();
		drawSelection();
		drawPenSize();
		drawCursor(oldx, oldy);
		//Fip the vs
		SDL_Flip(screen);

//		//Printing out the framerate and particlecount
//		FrameTime = SDL_GetTicks();
//		AvgFrameTimes[Index] = FrameTime - PrevFrameTime;
//		Index = (Index + 1) % NumFrames;
//		PrevFrameTime = FrameTime;
//		//We'll print for each 50 frames
//		if(tick % 50 == 0)
//		{
//			int avg = 0;
//			//Calculating the average over NumFrames frames
//			for( int i = 0; i < NumFrames; i++)
//				avg += AvgFrameTimes[i];

//			avg = 1000/((int)avg/NumFrames);

//			printf("FPS: %i\n",avg);
//			printf("Particle count: %i\n\n",particleCount);
//		}
	}

	//Loop ended - quit SDL
	SDL_Quit( );
	if(SDL_NumJoysticks() > 0)
		SDL_JoystickClose(0);
	return 0;
}
