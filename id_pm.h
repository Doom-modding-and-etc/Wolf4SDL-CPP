// ID_PM.H

#ifndef __ID_PM_H_
#define __ID_PM_H_

#ifdef USE_HIRES
#define PMPageSize 16384
#else
#define PMPageSize 4096
#endif

#define PM_GetSpritePage(v)    PM_GetPage (PMSpriteStart + (v))
#define PM_GetSoundPage(v)     PM_GetPage (PMSoundStart + (v))

extern word ChunksInFile;
extern word PMSpriteStart;
extern word PMSoundStart;

extern bool PMSoundInfoPagePadded;

extern byte **PMPages;

void     PM_Startup (void);
void     PM_Shutdown (void);
uint32_t PM_GetPageSize (int page);
byte     *PM_GetPage (int page);
byte     *PM_GetPageEnd (void);

#endif
