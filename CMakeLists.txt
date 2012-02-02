cmake_minimum_required(VERSION 2.6)

PROJECT(curvedsalsa)

IF (NOT WIN32)
#	ADD_DEFINITIONS(-Wall -Wno-missing-braces -g0)
#	ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)
ENDIF (NOT WIN32)

FILE(GLOB source_files *.cpp *.h *.inl)

ADD_EXECUTABLE(curvedsalsa 
	${source_files}
	curve25519-donna-1.1/curve25519-donna.c
)

IF (WIN32)
	TARGET_LINK_LIBRARIES(curvedsalsa optimized "cryptopp/Win32/output/release/cryptlib")
	TARGET_LINK_LIBRARIES(curvedsalsa debug "cryptopp/Win32/output/debug/cryptlib")

	STRING(REPLACE	"/MDd"	"/MTd"	CMAKE_CXX_FLAGS_DEBUG	${CMAKE_CXX_FLAGS_DEBUG})	
	STRING(REPLACE	"/MD"	"/MT"	CMAKE_CXX_FLAGS_DEBUG	${CMAKE_CXX_FLAGS_DEBUG})	
ELSE (WIN32)
	TARGET_LINK_LIBRARIES(curvedsalsa dl)
ENDIF (WIN32)
