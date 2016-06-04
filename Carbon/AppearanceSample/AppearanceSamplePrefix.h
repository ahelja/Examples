#ifndef BUILDING_FOR_CARBON_8
#define BUILDING_FOR_CARBON_8	0
#endif

#ifndef HELP_TAGS_ENABLED
#define HELP_TAGS_ENABLED	1
#endif

#define rectTopLeft(r)	(((Point *) &(r))[0])
#define rectBotRight(r)	(((Point *) &(r))[1])

#include <Carbon/Carbon.h>
