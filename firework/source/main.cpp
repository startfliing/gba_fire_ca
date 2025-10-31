#include "tonc.h"

#include "page_pic.h"

// Conway's Game of Life on a 120x80 grid using two 1D bool arrays.
// Grid width and height
constexpr int PIXEL_WIDTH = 2;

constexpr int WIDTH = 120;
constexpr int HEIGHT = 80;

u16* front_screen = ((u16*)MEM_VRAM);
u16* back_screen = ((u16*)MEM_VRAM_BACK);

struct cell_data{
	u8 curr_cell_intensity;
	u8 highest_neighbor_intesity;
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

// get the intensity of highest neighbor
// new cell is highest_intensity - 1
// TODO: Check only orthogonal squares (dx, dy)
//	- (-1,0), (1,0), (0,-1), (0,1)
cell_data highest_adjacent_cell(const u16* src, int x, int y) {
	cell_data val;
	val.curr_cell_intensity = get_cell(src, x, y);
	val.highest_neighbor_intesity = 0;
	for(int dz = -1; dz <= 1; dz += 2){
		//check (x+dz, y) and (x, y+dz)
		int temp_y_intensity = get_cell(src, x, y + dz);
		int temp_x_intensity = get_cell(src, x + dz, y);
		int temp_intensity = temp_y_intensity > temp_x_intensity ? temp_y_intensity : temp_x_intensity;
		val.highest_neighbor_intesity = temp_intensity > val.highest_neighbor_intesity ? temp_intensity : val.highest_neighbor_intesity;
	}
	return val;
}

//increase palette intensity to 16 colors
void set(){
	for(int i = 1; i < 32; i++){
		pal_bg_mem[i] = RGB15(i,i,i);
	}
}

// Step one generation from src -> dst (both size WIDTH x HEIGHT)
void step(const u16* src, u16* dst) {
	// iterate inner grid
	for (int y = 1; y < HEIGHT-1; y++) {
		for (int x = 1; x < WIDTH-1; x++) {
			cell_data n = highest_adjacent_cell(src, x, y);
			int new_color = 0;
			// TODO: use intesity between 7-15 for fireworks
			if(n.curr_cell_intensity == 0 && n.highest_neighbor_intesity == 0){
				if((qran()&4095) == 0){
					new_color = (qran()&15)+16;
				}
				set_cell(dst, x, y, new_color);
				continue;
			}

			s8 decay = (qran()&7)+1;
			new_color = n.curr_cell_intensity > n.highest_neighbor_intesity ? n.curr_cell_intensity : n.highest_neighbor_intesity;
			new_color -= decay;

			set_cell(dst, x, y, clamp(new_color, 0, 32));
		}
	}
}

int main() {
    // start with random

	REG_DISPCNT = DCNT_MODE4 | DCNT_BG2;

	load_image();

	while(!key_hit(KEY_A)){
		qran();
		key_poll();
	}

	clear_image();
	
    bool isDisplayingFront = true;

	set();

    while(1) {
        const u16* src = isDisplayingFront ? front_screen : back_screen;
        u16* dst = isDisplayingFront ? back_screen : front_screen;

        step(src, dst);

        isDisplayingFront = !isDisplayingFront; // alternate arrays per generation

        vid_flip();
    }

    return 0;
}
