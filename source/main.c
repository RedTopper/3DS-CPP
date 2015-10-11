#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <setjmp.h>

#include <3ds.h>

#include "drawing.h"

static jmp_buf exitJmp;
u32 *irmemloc;
Result Startup;
Result SetIR;
int frame = 0; // if the framerate is 60, every time we hit 60, 1 second has passed. 

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

int main(void) {
	irmemloc = (u32*) malloc(0x1000);
	Startup = IRU_Initialize(irmemloc, 0x1000);
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
		drawString(10, 70, "IR Init  Error: %x", Startup);
		drawString(10, 80, "IR I/O   Error: %x", SetIR);
		frame++;
		if(frame == 60) {
			frame = 0;
		}
		if(frame == 58 || frame == 59) {
			SetIR = IRU_SetIRLEDState(0x1);
			drawString(10, 90, "IR state:*");
		} else {
			SetIR = IRU_SetIRLEDState(0x0);
			drawString(10, 90, "IR state:");
		}
		
		drawString(10, 220, "Start + Select to exit.");

		
		if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}
	
	exit:
	
	IRU_Shutdown();
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	
	return 0;
}
