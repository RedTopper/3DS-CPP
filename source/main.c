#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <setjmp.h>

#include <3ds.h>

#include "drawing.h"

static jmp_buf exitJmp;
u32 *irmemloc;
//u8 *buf;
//u32 *transfercount;
Result Startup;
Result SetBit;
Result SetIR;
Result GetIR = 0xffffffff; //unknown until known...
unsigned long frame = 0;


void hang(char *message) {
	while(aptMainLoop()) {
		hidScanInput();
		
		clearScreen();
		drawString(10, 10, "%s", message);
		drawString(10, 20, "Start and Select to exit");
		
		u32 kHeld = hidKeysHeld();
		if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}
}

void printMemory(void *ptr, int size, int row) //This is probably very bad but whatever
{ 
    unsigned char *c = (unsigned char *)ptr;
	int i = 0; //goes through all memory
	int col = 0; //goes through the colums in desp
	int rowu = 0; //goes through the row in desp
	int memed = 0; //added to when memory is shown.
    while(i  != size){
		int mem = (int)c[i];
        if(mem != 0) {
			drawString(10 + (col*18), row + (rowu * 10), "%02x ", mem); 
			if(col>15) {
				col = -1;
				rowu++;
			}
			col++;
			memed++;
		}
		i++;
	}  
	if(memed == 0) {
		drawString(10, row, "Nothing to show in memory."); 
	}
}

int main(void) {
	irmemloc = (u32*) memalign(0x1000, 0x1000);
	Startup = IRU_Initialize(irmemloc, 0x1000);
	SetBit = IRU_SetBitRate(0x4);
	srvInit();        // services
	aptInit();        // applets
	hidInit(NULL);    // input
	gfxInitDefault(); // gfx
	
	gfxSetDoubleBuffering(GFX_TOP, true);
	gfxSetDoubleBuffering(GFX_BOTTOM, true);
	
	if(setjmp(exitJmp)) goto exit;
	
	clearScreen();
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	while(aptMainLoop()) {
		hidScanInput();
		irrstScanInput();
		
		u32 kHeld = hidKeysHeld();
		circlePosition circlePad;
		circlePosition cStick;
		//circlePosition circlePadPro;
		u8 vol8;
		u8* volp = &vol8;
		hidCstickRead(&cStick);
		hidCircleRead(&circlePad);
		HIDUSER_GetSoundVolume(volp);
		touchPosition touch;
		touchRead(&touch);
		
		clearScreen();
		
		char keys[30] = "ABXY URDL SEST LR ZLZR";
		
		if(kHeld & KEY_A) {
			keys[0] = '*';
		}
		if(kHeld & KEY_B) {
			keys[1] = '*';
		}
		if(kHeld & KEY_X) {
			keys[2] = '*';
		}
		if(kHeld & KEY_Y) {
			keys[3] = '*';
		}
		if(kHeld & KEY_DUP) {
			keys[5] = '*';
		}
		if(kHeld & KEY_DRIGHT) {
			keys[6] = '*';
		}
		if(kHeld & KEY_DDOWN) {
			keys[7] = '*';
		}
		if(kHeld & KEY_DLEFT) {
			keys[8] = '*';
		}
		if(kHeld & KEY_SELECT) {
			keys[10] = '*';
			keys[11] = '*';
		}
		if(kHeld & KEY_START) {
			keys[12] = '*';
			keys[13] = '*';
		}
		if(kHeld & KEY_L) {
			keys[15] = '*';
		}
		if(kHeld & KEY_R) {
			keys[16] = '*';
		}
		if(kHeld & KEY_ZL) {
			keys[18] = '*';
			keys[19] = '*';
		}
		if(kHeld & KEY_ZR) {
			keys[20] = '*';
			keys[21] = '*';
		}
		drawString(10, 10, keys);
		drawString(10, 20, "Volume: %d", vol8);
		drawString(10, 30, "Circle Pad   x: %04+d, y: %04+d", circlePad.dx, circlePad.dy);
		drawString(10, 40, "CStick (new) x: %04+d, y: %04+d", cStick.dx, cStick.dy );
		drawString(10, 50, "CPadPr (old) x: NaN , y: NaN", cStick.dx, cStick.dy );
		drawString(10, 60, "Touch        x: %04d, y: %04d", touch.px, touch.py );
		if(Startup == 0) {
			drawString(10, 70, "IR started!");
		} else {
			drawString(10, 70, "IR Init  Error: %x", Startup);
		}
		if(SetIR == 0) {
			drawString(10, 80, "IR on/off works!");
		} else {
			drawString(10, 80, "IR I/O   Error: %x", SetIR);
		}
		if(SetBit == 0) {
			drawString(10, 90, "IR bit rate works!");
		} else {
			drawString(10, 90, "IR Bit   Error: %x", SetBit);
		}
		if(GetIR == 0) {
			drawString(10, 100, "IR get works :P!");
		} else {
			drawString(10, 100, "IR get   Error: %x", GetIR);
		}
		
		
		if(frame % 36 == 0) {
			SetIR = IRU_SetIRLEDState(0x1);
			drawString(10, 110, "IR state:*");
		} else if(frame % 36 == 1) {
			drawString(10, 110, "IR state:*");
		} else if(frame % 36 == 2){
			SetIR = IRU_SetIRLEDState(0x0);
			drawString(10, 110, "IR state:");
		} else {
			drawString(10, 110, "IR state:");
		}
		
		if(frame > 363) {
			GetIR = irucmd_StartRecvTransfer(0x98, 0x1);
		}
		printMemory(irmemloc,0x1000,120); //might be dangerous?
		
		drawString(10, 220, "Start + Select to exit.");
		
		if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
		frame++;
	}
	
	exit:
	
	IRU_Shutdown();
	free(irmemloc);
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	
	return 0;
}
