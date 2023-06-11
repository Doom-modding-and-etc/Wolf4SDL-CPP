#ifndef __ID_PM__
#define __ID_PM__

#ifdef USE_HIRES
#define PMPageSize 16384
#else
#define PMPageSize 4096
#endif

extern int ChunksInFile;
extern int PMSpriteStart;
extern int PMSoundStart;

extern boolean PMSoundInfoPagePadded;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
extern uint8_t **PMPages;

void PM_Startup();
void PM_Shutdown() NOEXCEPT;

extern inline uint32_t PM_GetPageSize(int page);

extern inline uint8_t* PM_GetPage(int page);

extern inline uint8_t* PM_GetEnd() NOEXCEPT;

extern inline byte* PM_GetTexture(int wallpic);

extern inline uint16_t *PM_GetSprite(int shapenum);

extern inline byte* PM_GetSound(int soundpagenum);

#endif
