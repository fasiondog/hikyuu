CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(sqlite3)

SET(sqlite3_src
    sqlite3.c
	sqlite3.h
	sqlite3ext.h)
	
add_definitions(-D "SQLITE_API=__declspec(dllexport)")
ADD_LIBRARY(sqlite3 SHARED ${sqlite3_src})