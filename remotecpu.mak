# Microsoft Developer Studio Generated NMAKE File, Based on grope.dsp
!IF "$(CFG)" == ""
CFG=grope - Win32 Debug
!MESSAGE No configuration specified. Defaulting to grope - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "grope - Win32 Release" && "$(CFG)" != "grope - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "grope.mak" CFG="grope - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "grope - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "grope - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "grope - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\grope.exe"


CLEAN :
	-@erase "$(INTDIR)\Edit2.obj"
	-@erase "$(INTDIR)\grope.obj"
	-@erase "$(INTDIR)\grope.pch"
	-@erase "$(INTDIR)\grope.res"
	-@erase "$(INTDIR)\gropeDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\grope.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\grope.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\grope.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\grope.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\grope.pdb" /machine:I386 /out:"$(OUTDIR)\grope.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Edit2.obj" \
	"$(INTDIR)\grope.obj" \
	"$(INTDIR)\gropeDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\grope.res"

"$(OUTDIR)\grope.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "grope - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\grope.exe" "$(OUTDIR)\grope.pch" "$(OUTDIR)\grope.bsc"


CLEAN :
	-@erase "$(INTDIR)\Edit2.obj"
	-@erase "$(INTDIR)\Edit2.sbr"
	-@erase "$(INTDIR)\grope.obj"
	-@erase "$(INTDIR)\grope.pch"
	-@erase "$(INTDIR)\grope.res"
	-@erase "$(INTDIR)\grope.sbr"
	-@erase "$(INTDIR)\gropeDlg.obj"
	-@erase "$(INTDIR)\gropeDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\util.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\grope.bsc"
	-@erase "$(OUTDIR)\grope.exe"
	-@erase "$(OUTDIR)\grope.ilk"
	-@erase "$(OUTDIR)\grope.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\grope.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\grope.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Edit2.sbr" \
	"$(INTDIR)\grope.sbr" \
	"$(INTDIR)\gropeDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\util.sbr"

"$(OUTDIR)\grope.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\grope.pdb" /debug /machine:I386 /out:"$(OUTDIR)\grope.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Edit2.obj" \
	"$(INTDIR)\grope.obj" \
	"$(INTDIR)\gropeDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\grope.res"

"$(OUTDIR)\grope.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("grope.dep")
!INCLUDE "grope.dep"
!ELSE 
!MESSAGE Warning: cannot find "grope.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "grope - Win32 Release" || "$(CFG)" == "grope - Win32 Debug"
SOURCE=.\Edit2.cpp

!IF  "$(CFG)" == "grope - Win32 Release"


"$(INTDIR)\Edit2.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\grope.pch"


!ELSEIF  "$(CFG)" == "grope - Win32 Debug"


"$(INTDIR)\Edit2.obj"	"$(INTDIR)\Edit2.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\grope.cpp

!IF  "$(CFG)" == "grope - Win32 Release"


"$(INTDIR)\grope.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\grope.pch"


!ELSEIF  "$(CFG)" == "grope - Win32 Debug"


"$(INTDIR)\grope.obj"	"$(INTDIR)\grope.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\grope.rc

"$(INTDIR)\grope.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\gropeDlg.cpp

!IF  "$(CFG)" == "grope - Win32 Release"


"$(INTDIR)\gropeDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\grope.pch"


!ELSEIF  "$(CFG)" == "grope - Win32 Debug"


"$(INTDIR)\gropeDlg.obj"	"$(INTDIR)\gropeDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "grope - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\grope.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\grope.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "grope - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\grope.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\grope.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\..\common\util.cpp

!IF  "$(CFG)" == "grope - Win32 Release"


"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\grope.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "grope - Win32 Debug"


"$(INTDIR)\util.obj"	"$(INTDIR)\util.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

