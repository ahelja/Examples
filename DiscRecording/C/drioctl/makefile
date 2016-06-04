CC                  = /usr/bin/cc
CFLAGS              = -W -Wmost -Wall -Wno-unused -g
FRAMEWORKS          = DiscRecording CoreFoundation
SEARCH_DIRS         = ../discrecutils/
LIBRARIES           = 
LIB_SEARCH_DIRS     =
FRAMEWORK_FLAGS     = ${foreach i,${FRAMEWORKS},-framework $i}
LIBRARY_FLAGS       = ${foreach i,${LIBRARIES},-l$i} ${foreach i,${LIB_SEARCH_DIRS},-L$i}
SEARCH_DIR_FLAGS    = ${foreach i,${SEARCH_DIRS},-I$i}


SOURCE_FILES        =	drioctl.c \
			../discrecutils/dru_devices.c

OUTPUTNAME          = drioctl

all:
	${CC} ${CFLAGS} ${FRAMEWORK_FLAGS} ${LIBRARY_FLAGS} ${SEARCH_DIR_FLAGS} ${SOURCE_FILES} -o ${OUTPUTNAME}

clean:
	@rm -f ${OUTPUTNAME}
