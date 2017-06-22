# Microsoft Developer Studio Project File - Name="MeSdk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MeSdk - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MeSdk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MeSdk.mak" CFG="MeSdk - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MeSdk - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MeSdk - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MeSdk - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../sdk/lib/win32/libSdkR.lib"

!ELSEIF  "$(CFG)" == "MeSdk - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MeSdk___Win32_Debug"
# PROP BASE Intermediate_Dir "MeSdk___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "MeSdk___Win32_Debug"
# PROP Intermediate_Dir "MeSdk___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fo"Debug/" /Fd"Debug/" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../sdk/lib/win32/libSdkD.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=./copy.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "MeSdk - Win32 Release"
# Name "MeSdk - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\array.c
# End Source File
# Begin Source File

SOURCE=.\src\common.c
# End Source File
# Begin Source File

SOURCE=.\copy.bat
# End Source File
# Begin Source File

SOURCE=.\src\data_pack.c
# End Source File
# Begin Source File

SOURCE=.\src\event.c
# End Source File
# Begin Source File

SOURCE=.\src\file.c
# End Source File
# Begin Source File

SOURCE=.\src\list.c
# End Source File
# Begin Source File

SOURCE=.\src\lock.c
# End Source File
# Begin Source File

SOURCE=.\src\log.c
# End Source File
# Begin Source File

SOURCE=.\src\memory.c
# End Source File
# Begin Source File

SOURCE=.\src\socket.c
# End Source File
# Begin Source File

SOURCE=.\src\socket_addr.c
# End Source File
# Begin Source File

SOURCE=.\src\socket_common.c
# End Source File
# Begin Source File

SOURCE=.\src\socket_event_listener_select.c
# End Source File
# Begin Source File

SOURCE=.\src\socket_listen.c
# End Source File
# Begin Source File

SOURCE=.\src\socket_manager.c
# End Source File
# Begin Source File

SOURCE=.\src\socket_manager_dispatch.c
# End Source File
# Begin Source File

SOURCE=.\src\socket_tcp.c
# End Source File
# Begin Source File

SOURCE=.\src\socket_udp.c
# End Source File
# Begin Source File

SOURCE=.\src\thread.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\array.h
# End Source File
# Begin Source File

SOURCE=.\include\common.h
# End Source File
# Begin Source File

SOURCE=.\include\data_pack.h
# End Source File
# Begin Source File

SOURCE=.\include\event.h
# End Source File
# Begin Source File

SOURCE=.\include\file.h
# End Source File
# Begin Source File

SOURCE=.\include\include.h
# End Source File
# Begin Source File

SOURCE=.\include\list.h
# End Source File
# Begin Source File

SOURCE=.\include\lock.h
# End Source File
# Begin Source File

SOURCE=.\include\log.h
# End Source File
# Begin Source File

SOURCE=.\include\map.h
# End Source File
# Begin Source File

SOURCE=.\include\memory.h
# End Source File
# Begin Source File

SOURCE=.\include\platdefine.h
# End Source File
# Begin Source File

SOURCE=.\include\socket.h
# End Source File
# Begin Source File

SOURCE=.\include\socket_addr.h
# End Source File
# Begin Source File

SOURCE=.\include\socket_common.h
# End Source File
# Begin Source File

SOURCE=.\include\socket_event.h
# End Source File
# Begin Source File

SOURCE=.\include\socket_event_listener.h
# End Source File
# Begin Source File

SOURCE=.\include\socket_listen.h
# End Source File
# Begin Source File

SOURCE=.\include\socket_manager.h
# End Source File
# Begin Source File

SOURCE=.\include\socket_manager_dispatch.h
# End Source File
# Begin Source File

SOURCE=.\include\socket_tcp.h
# End Source File
# Begin Source File

SOURCE=.\include\socket_udp.h
# End Source File
# Begin Source File

SOURCE=.\include\thread.h
# End Source File
# End Group
# Begin Group "jsonc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\jsonc\arraylist.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\arraylist.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\bits.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\config.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\debug.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\debug.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\json.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_c_version.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_c_version.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_config.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_inttypes.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_object.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_object.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_object_iterator.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_object_iterator.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_object_private.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_tokener.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_tokener.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_util.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\json_util.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\libjson.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\linkhash.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\linkhash.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\math_compat.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\printbuf.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\printbuf.h
# End Source File
# Begin Source File

SOURCE=.\jsonc\random_seed.c
# End Source File
# Begin Source File

SOURCE=.\jsonc\random_seed.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\framework.des
# End Source File
# End Target
# End Project
