#include "EnginePrivate.h"

#include "../../../External/freetype-2.3.5/include/ft2build.h"
#include "../../../External/freetype-2.3.5/include/freetype/freetype.h"

#if (defined _DEBUG) && (defined USE_DEBUG_FREETYPE)
	#pragma message("Linking Win32 DEBUG Freetype Libs")
	#pragma comment(lib, "../../External/freetype-2.3.5/objs/freetype235MT_D.lib")	
#else
	#pragma message("Linking Win32 RELEASE Freetype Libs")
	#pragma comment(lib, "../../External/freetype-2.3.5/objs/freetype235MT.lib")
#endif

FT_Library GFreetype;

