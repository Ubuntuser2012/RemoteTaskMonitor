# Microsoft Developer Studio Project File - Name="remotecpu2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=remotecpu2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "remotecpu2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "remotecpu2.mak" CFG="remotecpu2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "remotecpu2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "remotecpu2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "remotecpu2 - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /ZI /Od /I "..\imagelib\libgd-733361a31aab\src" /I "..\imagelib\zlib-1.2.5" /I "..\imagelib\lpng157" /I "..\imagelib\libjpeg-8d" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "_MY_LOG_" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 Iphlpapi.lib Ws2_32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /incremental:yes /map /debug /machine:I386 /out:"out/win32/rtmonitor.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "remotecpu2 - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GR /GX /ZI /Od /I "..\imagelib\libgd-733361a31aab\src" /I "..\imagelib\zlib-1.2.5" /I "..\imagelib\lpng157" /I "..\imagelib\libjpeg-8d" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Iphlpapi.lib Ws2_32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /map /debug /machine:I386 /out:"out/win32/rtmonitor.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "remotecpu2 - Win32 Release"
# Name "remotecpu2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\1-LNK2005-remedy.cpp"
# End Source File
# Begin Source File

SOURCE=.\comm.c
# End Source File
# Begin Source File

SOURCE=..\GameDev\crypt.cpp
# End Source File
# Begin Source File

SOURCE=.\Device.cpp
# End Source File
# Begin Source File

SOURCE=.\DeviceList.cpp
# End Source File
# Begin Source File

SOURCE=.\dialog2.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgDbgPs1.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgDbgPs2.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgDbgPs3.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgDbgPsInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgError.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgLoadingData.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgRemotecpu.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSaved.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgStartProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgThreadPriority.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWatchBase.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCfg.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCPU.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCPUAlert.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCPUTotal.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCPUTotalAlert.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWatchMem.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWatchMemAlert.cpp
# End Source File
# Begin Source File

SOURCE=..\DS\ds.cpp
# End Source File
# Begin Source File

SOURCE=.\EditUrl.cpp
# End Source File
# Begin Source File

SOURCE=..\FILEIO\FILEIO.cpp
# End Source File
# Begin Source File

SOURCE=.\GDGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\graph.cpp
# End Source File
# Begin Source File

SOURCE=.\graphwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\graphwnd2.cpp
# End Source File
# Begin Source File

SOURCE=.\History.cpp
# End Source File
# Begin Source File

SOURCE=.\License.cpp
# End Source File
# Begin Source File

SOURCE=.\ListCtrl2.cpp
# End Source File
# Begin Source File

SOURCE=.\Process.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcessList.cpp
# End Source File
# Begin Source File

SOURCE=.\PropPageAbout.cpp
# End Source File
# Begin Source File

SOURCE=.\PropPageBase.cpp
# End Source File
# Begin Source File

SOURCE=.\PropPageDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\PropPageGraphs.cpp
# End Source File
# Begin Source File

SOURCE=.\PropPageOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\PropPageProcesses.cpp
# End Source File
# Begin Source File

SOURCE=.\PropSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\raphael.cpp
# End Source File
# Begin Source File

SOURCE=.\remotecpu.cpp
# End Source File
# Begin Source File

SOURCE=.\remotecpu.rc
# End Source File
# Begin Source File

SOURCE=.\SMTP.CPP
# End Source File
# Begin Source File

SOURCE=.\startps.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\ThreadList.cpp
# End Source File
# Begin Source File

SOURCE=..\common\util.cpp
# End Source File
# Begin Source File

SOURCE=.\WatchList.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\comm.h
# End Source File
# Begin Source File

SOURCE=..\GameDev\crypt.h
# End Source File
# Begin Source File

SOURCE=.\Device.h
# End Source File
# Begin Source File

SOURCE=.\DeviceList.h
# End Source File
# Begin Source File

SOURCE=.\dialog2.h
# End Source File
# Begin Source File

SOURCE=.\DlgDbgPs1.h
# End Source File
# Begin Source File

SOURCE=.\DlgDbgPs2.h
# End Source File
# Begin Source File

SOURCE=.\DlgDbgPs3.h
# End Source File
# Begin Source File

SOURCE=.\DlgDbgPsInfo.h
# End Source File
# Begin Source File

SOURCE=.\DlgError.h
# End Source File
# Begin Source File

SOURCE=.\dlgLoadingData.h
# End Source File
# Begin Source File

SOURCE=.\dlgRemotecpu.h
# End Source File
# Begin Source File

SOURCE=.\DlgSaved.h
# End Source File
# Begin Source File

SOURCE=.\dlgStartProcess.h
# End Source File
# Begin Source File

SOURCE=.\dlgThreadPriority.h
# End Source File
# Begin Source File

SOURCE=.\DlgWatchBase.h
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCfg.h
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCPU.h
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCPUAlert.h
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCPUTotal.h
# End Source File
# Begin Source File

SOURCE=.\DlgWatchCPUTotalAlert.h
# End Source File
# Begin Source File

SOURCE=.\DlgWatchMem.h
# End Source File
# Begin Source File

SOURCE=.\DlgWatchMemAlert.h
# End Source File
# Begin Source File

SOURCE=..\DS\ds.h
# End Source File
# Begin Source File

SOURCE=.\EditUrl.h
# End Source File
# Begin Source File

SOURCE=..\FILEIO\fileio.h
# End Source File
# Begin Source File

SOURCE=.\GDGraph.h
# End Source File
# Begin Source File

SOURCE=.\graph.h
# End Source File
# Begin Source File

SOURCE=.\graphwnd.h
# End Source File
# Begin Source File

SOURCE=.\graphwnd2.h
# End Source File
# Begin Source File

SOURCE=.\History.h
# End Source File
# Begin Source File

SOURCE=.\License.h
# End Source File
# Begin Source File

SOURCE=.\ListCtrl2.h
# End Source File
# Begin Source File

SOURCE=.\Process.h
# End Source File
# Begin Source File

SOURCE=.\ProcessList.h
# End Source File
# Begin Source File

SOURCE=.\PropPageAbout.h
# End Source File
# Begin Source File

SOURCE=.\PropPageBase.h
# End Source File
# Begin Source File

SOURCE=.\PropPageDevice.h
# End Source File
# Begin Source File

SOURCE=.\PropPageGraphs.h
# End Source File
# Begin Source File

SOURCE=.\PropPageOptions.h
# End Source File
# Begin Source File

SOURCE=.\PropPageProcesses.h
# End Source File
# Begin Source File

SOURCE=.\PropSheet.h
# End Source File
# Begin Source File

SOURCE=.\raphael.h
# End Source File
# Begin Source File

SOURCE=.\remotecpu.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SMTP.H
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Thread.h
# End Source File
# Begin Source File

SOURCE=.\ThreadList.h
# End Source File
# Begin Source File

SOURCE=..\common\util.h
# End Source File
# Begin Source File

SOURCE=.\WatchList.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\alert_triangle.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bluesqr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\crash_triangle.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\device.bmp
# End Source File
# Begin Source File

SOURCE=.\res\greensqr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icon.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\listicon.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ps1.ico
# End Source File
# Begin Source File

SOURCE=.\res\remotecpu.ico
# End Source File
# Begin Source File

SOURCE=.\res\remotecpu.rc2
# End Source File
# Begin Source File

SOURCE=.\res\skull.bmp
# End Source File
# Begin Source File

SOURCE=.\res\spy.bmp
# End Source File
# End Group
# Begin Group "libgd"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\annotate.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\bmp.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\entities.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_bmp.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_color.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_color.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_color_map.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_color_map.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_crop.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_filter.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_gd.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_gd2.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_gif_in.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_gif_out.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_io.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_io.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_io_dp.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_io_file.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_io_ss.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_io_stream.cxx"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_io_stream.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_jpeg.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_nnquant.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_nnquant.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_pixelate.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_png.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_security.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_ss.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_tga.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_tga.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_tiff.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_topal.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_transform.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gd_wbmp.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdcache.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdcache.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfontg.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfontg.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfontl.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfontl.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfontmb.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfontmb.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfonts.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfonts.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfontt.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfontt.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdft.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfx.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdfx.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdhelpers.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdhelpers.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdkanji.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdpp.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdtables.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\gdxpm.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\jisx0208.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\wbmp.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libgd-733361a31aab\src\wbmp.h"
# End Source File
# End Group
# Begin Group "libpng"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\imagelib\lpng157\png.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\png.h
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngconf.h
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngdebug.h
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngerror.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngget.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pnginfo.h
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pnglibconf.h
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngmem.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngpread.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngpriv.h
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngread.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngrio.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngrtran.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngrutil.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngset.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngstruct.h
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngtrans.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngwio.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngwrite.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngwtran.c
# End Source File
# Begin Source File

SOURCE=..\imagelib\lpng157\pngwutil.c
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\adler32.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\blast.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\blast.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\compress.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\crc32.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\crc32.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\crypt.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\deflate.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\deflate.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\gzclose.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\gzguts.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\gzlib.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\gzread.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\gzwrite.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\infback.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\infback9.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\infback9.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inffas86.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inffas8664.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inffix9.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inffixed.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inflate.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inflate.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inflate9.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inftree9.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inftree9.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inftrees.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\inftrees.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\ioapi.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\ioapi.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\iowin32.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\iowin32.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\mztools.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\mztools.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\puff.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\puff.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\trees.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\trees.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\uncompr.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\unzip.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\unzip.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\zconf.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\zip.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\zip.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\zlib.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\zutil.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\zlib-1.2.5\zutil.h"
# End Source File
# End Group
# Begin Group "libjpeg"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\cderror.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\cdjpeg.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\cdjpeg.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jaricom.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcapimin.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcapistd.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcarith.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jccoefct.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jccolor.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcdctmgr.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jchuff.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcinit.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcmainct.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcmarker.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcmaster.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcomapi.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcparam.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcprepct.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jcsample.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jctrans.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdapimin.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdapistd.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdarith.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdatadst.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdatasrc.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdcoefct.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdcolor.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdct.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jddctmgr.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdhuff.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdinput.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdmainct.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdmarker.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdmaster.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdmerge.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdpostct.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdsample.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jdtrans.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jerror.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jerror.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jfdctflt.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jfdctfst.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jfdctint.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jidctflt.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jidctfst.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jidctint.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jinclude.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jmemansi.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jmemmgr.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jmemsys.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jmorecfg.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jpegint.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jpeglib.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jquant1.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jquant2.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jutils.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\jversion.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\rdbmp.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\rdcolmap.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\rdgif.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\rdppm.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\rdrle.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\rdswitch.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\rdtarga.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\transupp.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\transupp.h"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\wrbmp.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\wrgif.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\wrppm.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\wrrle.c"
# End Source File
# Begin Source File

SOURCE="..\imagelib\libjpeg-8d\wrtarga.c"
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\css.htm
# End Source File
# Begin Source File

SOURCE=.\res\headerbg.png
# End Source File
# Begin Source File

SOURCE=.\res\html1.htm
# End Source File
# Begin Source File

SOURCE=.\res\html2.htm
# End Source File
# Begin Source File

SOURCE=.\res\html3.htm
# End Source File
# Begin Source File

SOURCE=.\res\html4.htm
# End Source File
# Begin Source File

SOURCE=.\res\padding.txt
# End Source File
# Begin Source File

SOURCE=.\res\pbg.png
# End Source File
# Begin Source File

SOURCE=.\res\plot_js.htm
# End Source File
# Begin Source File

SOURCE=.\res\raphael.js
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\res\remotetaskmontorlogo.png
# End Source File
# End Target
# End Project
