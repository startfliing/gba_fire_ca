#include "tonc.h"

#include "page_pic.h"

// Conway's Game of Life on a 120x80 grid using two 1D bool arrays.
// Grid width and height
constexpr int PIXEL_WIDTH = 2;

constexpr int WIDTH = 120;
constexpr int HEIGHT = 80;

u16* front_screen = ((u16*)MEM_VRAM);
u16* back_screen = ((u16*)MEM_VRAM_BACK);


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
		memcpy16(&vid_mem_front[ii*120], &page_picBitmap[ii*240/4], 240/2);
	}
}

int hottest_neighbor(const u16* src, int x, int y) {
	int hottest_neighbor = 0;
	int ny = y + 1;
	for (int dx = -1; dx <= 1; dx++) {
		int nx = x + dx;
		int temp = get_cell(src, nx, ny);
		if(temp > hottest_neighbor) hottest_neighbor = temp;
	}
	int val = hottest_neighbor - (qran()&3);
	return clamp(val, 0, 8);
}

int avg_neighbors(const u16* src, int x, int y) {
	int count = 0;
	int ny = y + 1;
	for (int dx = -1; dx <= 1; dx++) {
		int nx = x + dx;
		count += get_cell(src, nx, ny);
	}
	count += qran()&1;
	return count/3;
}

void drawBottomRow(u16* dst){
	int currPixel = 0;
	for(int i = 0; i < 15; i++){
		//color 8 pixels each
		u16 colorInd = i <= 7 ? i : 14 - i;
		u16 pixelNum = 8 + ((s8)qran())%3;
		for(int j = 0; j < pixelNum; j++){
			set_cell(dst, currPixel, HEIGHT-1, colorInd);
			if(currPixel++ == WIDTH) return;
		}
	}
	
};

void set(){
	pal_bg_mem[1] = RGB15(31,0,0);
	pal_bg_mem[2] = RGB15(31,10,0);
	pal_bg_mem[3] = RGB15(31,15,0);
	pal_bg_mem[4] = RGB15(31,23,0);
	pal_bg_mem[5] = RGB15(31,31,0);
	pal_bg_mem[6] = RGB15(31,31,15);
	pal_bg_mem[7] = RGB15(31,31,31);

	drawBottomRow(front_screen);
}

// Step one generation from src -> dst (both size WIDTH x HEIGHT)
void step_highest(const u16* src, u16* dst) {
	// iterate inner grid
	for (int y = 1; y < HEIGHT-1; y++) {
		for (int x = 1; x < WIDTH-1; x++) {
			int n = hottest_neighbor(src, x, y);
			set_cell(dst, x, y, n);
		}
	}
	drawBottomRow(dst);
}

void step_average(const u16* src, u16* dst) {
	// iterate inner grid
	for (int y = 1; y < HEIGHT-1; y++) {
		for (int x = 1; x < WIDTH-1; x++) {
			int n = avg_neighbors(src, x, y);
			set_cell(dst, x, y, n);
		}
	}
	drawBottomRow(dst);
}

int main() {
    // start with random

	REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

	while(1){
		load_image();

		u16 buttonSelected = 0;

		while(buttonSelected == 0 || buttonSelected == (KEY_A | KEY_B)){
			qran();
			key_poll();
			buttonSelected = key_hit(KEY_A | KEY_B);
		}

		clear_image();

		
		bool isDisplayingFront = true;

		set();

		while(1) {
			const u16* src = isDisplayingFront ? front_screen : back_screen;
			u16* dst = isDisplayingFront ? back_screen : front_screen;

			switch(buttonSelected){
				case KEY_A:
					step_average(src, dst);
					break;
				case KEY_B:
					step_highest(src,dst);
					break;
			}

			isDisplayingFront = !isDisplayingFront; // alternate arrays per generation

			vid_flip();

			key_poll();
			if(key_hit(KEY_START) || key_is_down(KEY_START)){break;}
		}

		if(!isDisplayingFront)vid_flip();
		clear_image();
	}
}