/*
	DispJPFont.h

	Reference
	M5Stackで日本語表示
	https://qiita.com/taront/items/7900c88b9e9782c33b08
*/

#include <M5Stack.h>
#include <sdfonts.h>

void fontDisp(uint16_t x, uint16_t y, uint8_t* buf, uint32_t fntcol=TFT_WHITE);
int fontDump(uint16_t x, uint16_t y, char* pUTF8, uint8_t sz, uint32_t fntcol=TFT_WHITE, bool fontfileOpenClose=true);
