#include "tonc.h"

#include "page_pic.h"

// Conway's Game of Life on a 120x80 grid using two 1D bool arrays.
// Grid width and height
constexpr int PIXEL_WIDTH = 2;

constexpr int WIDTH = 120;
constexpr int HEIGHT = 80;

u16* front_screen = ((u16*)MEM_VRAM);
u16* back_screen = ((u16*)MEM_VRAM_BACK);

struct vector{
	s8 dx;
	s8 dy;
};

//checks those three cells
const vector int_to_directions[8][3] = {
	{{-1,-1},	{0,-1},	{1,-1}},	//S
	{{0,-1},	{1,-1},	{1,0}}, 	//SE
	{{1,-1},	{1,0},	{1,1}},		//W
	{{1,0},		{0,1},	{1,1}},		//SW
	{{-1,1},	{0,1},	{1,1}},		//S
	{{-1,0},	{-1,1},	{0,1}},		//SE
	{{-1,-1},	{-1,0},	{-1,1}},	//E
	{{-1,-1},	{0,-1},	{-1,0}}		//NE
};

// Get cell
u8 get_cell(const u16* src, int gx, int gy) {
	// wrap coordinates to implement toroidal world
	return src[(gy * PIXEL_WIDTH * 120) + (gx)] & 0x00FF;
}

// draw a 2x2 pixel block.
void set_cell(u16* dst, int gx, int gy, u16 color) {
	int ind = (gy*240)+(gx);
	const u16 doubleColor = (color&0xFF)<<8 | (color&0xFF);
	for(int i = 0; i < PIXEL_WIDTH; i++){
		dst[ind+(i*120)] = doubleColor;
	}
}

void clear_image(){
	for(int x = 0; x < 120; x++){
		for(int y = 0; y < 80; y++){
			set_cell(front_screen,x,y,0);
			set_cell(back_screen,x,y,0);
		}
	}
}

void load_image(){
	pal_bg_mem[1] = RGB15(31,31,31);
	for(int ii=0; ii<160; ii++){
		memcpy16(&vid_mem_front[ii*120], &page_picBitmap[ii*240/4], 240/4);
	}
}

// check the fire direction
u16 check_fire_direction(const u16* src, int x, int y, u16 dir) {
	u16 curr_cell_intensity = get_cell(src, x, y);
	if(curr_cell_intensity == 0) return 0;	//if cell is black, stay black
	if(curr_cell_intensity == 2){ //if cell is red
		int new_color = (qran()&3) == 0 ? 0 : 2; //chance to turn black
		return new_color;
	}

	//cell is green
	for(int i = 0; i < 3; i++){
		//check direction[dir][i] for if there is fire
		vector direction_vector = int_to_directions[dir][i];
		int temp_intensity = get_cell(src, x + direction_vector.dx, y + direction_vector.dy);
		if(temp_intensity == 2 && (qran()&3) == 0) return 2; //if a cell in the direction is on fire, chance for it to spread
	}
	return 1;
}

//increase palette intensity to 16 colors
void set(){
	pal_bg_mem[1] = RGB15(0,31,0);
	pal_bg_mem[2] = RGB15(31,0,0);


	//initialize screen with one fire pixel
	bool fire_started = false;

	for(int x = 1; x < 119; x++){
		for(int y = 1; y < 79; y++){
			int color = 1;
			if((qran()&0x0FFF) == 0 && !fire_started){
				color = 2;
				fire_started = true;
			};
			set_cell(front_screen, x, y, color);
		}
	}

	if(!fire_started) set_cell(front_screen, 60, 40, 2);
}

// Step one generation from src -> dst (both size WIDTH x HEIGHT)
void step(const u16* src, u16* dst, u16 dir) {
	// iterate inner grid
	for (int y = 1; y < HEIGHT-1; y++) {
		for (int x = 1; x < WIDTH-1; x++) {
			u16 n = check_fire_direction(src, x, y, dir);
			set_cell(dst, x, y, n);
		}
	}
}

int main() {
    // start with random

	REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

	while(1){
		
		load_image();

		while(!key_hit(KEY_A)){
			qran();
			key_poll();
		}
		

		u16 dir = qran()&7;
		
		bool isDisplayingFront = true;

		set();

		while(1) {
			key_poll();
			const u16* src = isDisplayingFront ? front_screen : back_screen;
			u16* dst = isDisplayingFront ? back_screen : front_screen;

			step(src, dst, dir);

			if(key_hit(KEY_A)){
				key_poll();
				break;
			}
			isDisplayingFront = !isDisplayingFront; // alternate arrays per generation
			vid_flip();
		}
		if(!isDisplayingFront) vid_flip();
		clear_image();
	}
}
