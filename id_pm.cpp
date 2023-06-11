#include "wl_def.h"

int ChunksInFile;
int PMSpriteStart;
int PMSoundStart;

boolean PMSoundInfoPagePadded = false;

// holds the whole VSWAP
uint32_t *PMPageData;
size_t PMPageDataSize;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
uint8_t **PMPages;

void PM_Startup()
{
    char fname[13] = "vswap.";
    strcat(fname,extension);

    FILE *file = fopen(fname,"rb");
    if(!file)
        CA_CannotOpen(fname);

    ChunksInFile = 0;
    fread(&ChunksInFile, sizeof(word), 1, file);
    PMSpriteStart = 0;
    fread(&PMSpriteStart, sizeof(word), 1, file);
    PMSoundStart = 0;
    fread(&PMSoundStart, sizeof(word), 1, file);

    uint32_t* pageOffsets = wlreinterpret_cast_conversion(uint32_t *, SafeMalloc((ChunksInFile + 1) * sizeof(int32_t)));
    fread(pageOffsets, sizeof(uint32_t), ChunksInFile, file);

    word *pageLengths = wlreinterpret_cast_conversion(word *, SafeMalloc(ChunksInFile * sizeof(word)));
    fread(pageLengths, sizeof(word), ChunksInFile, file);

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    long pageDataSize = fileSize - pageOffsets[0];
    if(pageDataSize > wlstatic_cast_conversion(size_t, -1))
        Quit("The page file \"%s\" is too large!", fname);

    pageOffsets[ChunksInFile] = fileSize;

    uint32_t dataStart = pageOffsets[0];
    int i;

    // Check that all pageOffsets are valid
    for(i = 0; i < ChunksInFile; i++)
    {
        if(!pageOffsets[i]) continue;   // sparse page
        if(pageOffsets[i] < dataStart || pageOffsets[i] >= wlstatic_cast_conversion(size_t, fileSize))
            Quit("Illegal page offset for page %i: %u (filesize: %u)",
                    i, pageOffsets[i], fileSize);
    }

    // Calculate total amount of padding needed for sprites and sound info page
    int alignPadding = 0;
    for(i = PMSpriteStart; i < PMSoundStart; i++)
    {
        if(!pageOffsets[i]) continue;   // sparse page
        uint32_t offs = pageOffsets[i] - dataStart + alignPadding;
        if(offs & 1)
            alignPadding++;
    }

    if((pageOffsets[ChunksInFile - 1] - dataStart + alignPadding) & 1)
        alignPadding++;

    PMPageDataSize = wlstatic_cast_conversion(size_t, pageDataSize + alignPadding);
    PMPageData = wlreinterpret_cast_conversion(uint32_t *, SafeMalloc(PMPageDataSize));

    PMPages = wlreinterpret_cast_conversion(uint8_t **, SafeMalloc((ChunksInFile + 1) * sizeof(uint8_t *)));

    // Load pages and initialize PMPages pointers
    uint8_t *ptr = wlreinterpret_cast_conversion(uint8_t *, PMPageData);
    for(i = 0; i < ChunksInFile; i++)
    {
        if(i >= PMSpriteStart && i < PMSoundStart || i == ChunksInFile - 1)
        {
            size_t offs = ptr - wlreinterpret_cast_conversion(uint8_t *, PMPageData);

            // pad with zeros to make it 2-byte aligned
            if(offs & 1)
            {
                *ptr++ = 0;
                if(i == ChunksInFile - 1) PMSoundInfoPagePadded = true;
            }
        }

        PMPages[i] = ptr;

        if(!pageOffsets[i])
            continue;               // sparse page

        // Use specified page length, when next page is sparse page.
        // Otherwise, calculate size from the offset difference between this and the next page.
        uint32_t size;
        if(!pageOffsets[i + 1]) size = pageLengths[i];
        else size = pageOffsets[i + 1] - pageOffsets[i];

        fseek(file, pageOffsets[i], SEEK_SET);
        fread(ptr, 1, size, file);
        ptr += size;
    }

    // last page points after page buffer
    PMPages[ChunksInFile] = ptr;

    free(pageLengths);
    free(pageOffsets);
    fclose(file);
}

inline uint32_t PM_GetPageSize(int page)
{
    if (page < 0 || page >= ChunksInFile)
        Quit("PM_GetPageSize: Tried to access illegal page: %i", page);
    return wlstatic_cast_conversion(uint32_t, (PMPages[page + 1] - PMPages[page]));
}

inline uint8_t* PM_GetPage(int page)
{
    if (page < 0 || page >= ChunksInFile)
        Quit("PM_GetPage: Tried to access illegal page: %i", page);
    return PMPages[page];
}

inline uint8_t* PM_GetEnd() NOEXCEPT
{
    return PMPages[ChunksInFile];
}

inline byte* PM_GetTexture(int wallpic)
{
    return PM_GetPage(wallpic);
}

inline uint16_t* PM_GetSprite(int shapenum)
{
    // correct alignment is enforced by PM_Startup()
    return wlreinterpret_cast_conversion(uint16_t*, wlreinterpret_cast_conversion(void*, PM_GetPage(PMSpriteStart + shapenum)));
}

inline byte* PM_GetSound(int soundpagenum)
{
    return PM_GetPage(PMSoundStart + soundpagenum);
}

void PM_Shutdown() NOEXCEPT
{
    free(PMPages);
    free(PMPageData);
}
