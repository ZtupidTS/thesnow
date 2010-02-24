@echo Setting environment for using Microsoft Visual Studio 2008 x86 tools.
@echo.
@echo ==============================
@SET REG=SOFTWARE
@if "%PROCESSOR_ARCHITECTURE%"=="AMD64" SET REG=SOFTWARE\Wow6432Node
@call :GetVSDir
@echo VS2008 DIR:%VSINSTALLDIR%
@call :GetVCDir
@echo VC2008 DIR:%VCINSTALLDIR%
@echo ==============================
@if "%VSINSTALLDIR%"=="" goto error_no_VSINSTALLDIR
@if "%VCINSTALLDIR%"=="" goto error_no_VCINSTALLDIR

@call :GetWindowsSdkDir

@if not "%WindowsSdkDir%" == "" (
	set "PATH=%WindowsSdkDir%bin;%PATH%"
	set "INCLUDE=%WindowsSdkDir%include;%INCLUDE%"
	set "LIB=%WindowsSdkDir%lib;%LIB%"
)

@rem
@rem Root of Visual Studio IDE installed files.
@rem
@set DevEnvDir=%VSINSTALLDIR%\Common7\IDE

@set PATH=%DevEnvDir%;%VCINSTALLDIR%\BIN;%VSINSTALLDIR%\Common7\Tools;%VSINSTALLDIR%\Common7\Tools\bin;%VCINSTALLDIR%\VCPackages;%PATH%
@set INCLUDE=%VCINSTALLDIR%\ATLMFC\INCLUDE;%VCINSTALLDIR%\INCLUDE;%INCLUDE%
@set LIB=%VCINSTALLDIR%\ATLMFC\LIB;%VCINSTALLDIR%\LIB;%LIB%
@set LIBPATH=%VCINSTALLDIR%\ATLMFC\LIB;%VCINSTALLDIR%\LIB;%LIBPATH%

@goto end

:GetWindowsSdkDir
@call :GetWindowsSdkDirHelper HKLM > nul 2>&1
@if errorlevel 1 call :GetWindowsSdkDirHelper HKCU > nul 2>&1
@if errorlevel 1 set VSINSTALLDIR=%VCINSTALLDIR%\PlatformSDK\
@exit /B 0

:GetVSDir
@call :GetVSDirHelper HKLM > nul 2>&1
@if errorlevel 1 call :GetVSDirHelper HKCU > nul 2>&1
@if errorlevel 1 set VSINSTALLDIR=
@exit /B 0

:GetVCDir
@call :GetVCDirHelper HKLM > nul 2>&1
@if errorlevel 1 call :GetVCDirHelper HKCU > nul 2>&1
@if errorlevel 1 set VCINSTALLDIR=
@exit /B 0

:GetWindowsSdkDirHelper
@SET WindowsSdkDir=
@for /F "tokens=1,2*" %%i in ('reg query "%1\%REG%\Microsoft\Microsoft SDKs\Windows" /v "CurrentInstallFolder"') DO (
	if "%%i"=="CurrentInstallFolder" (
		SET "WindowsSdkDir=%%k"
	)
)
@if "%WindowsSdkDir%"=="" exit /B 1
@exit /B 0

:GetVSDirHelper
@SET VSINSTALLDIR=
@for /F "tokens=1,2*" %%i in ('reg query "%1\%REG%\Microsoft\VisualStudio\9.0\Setup\VS" /v "ProductDir"') DO (
	if "%%i"=="ProductDir" (
		SET "VSINSTALLDIR=%%k"
	)
)
@if "%VSINSTALLDIR%"=="" exit /B 1
@exit /B 0

:GetVCDirHelper
@SET VCINSTALLDIR=
@for /F "tokens=1,2*" %%i in ('reg query "%1\%REG%\Microsoft\VisualStudio\9.0\Setup\VC" /v "ProductDir"') DO (
	if "%%i"=="ProductDir" (
		SET "VCINSTALLDIR=%%k"
	)
)
@if "%VCINSTALLDIR%"=="" exit /B 1
@exit /B 0

:error_no_VSINSTALLDIR
@echo ERROR: VSINSTALLDIR variable is not set. 
@PAUSE
EXIT

:error_no_VCINSTALLDIR
@echo ERROR: VCINSTALLDIR variable is not set. 
@PAUSE
EXIT

:end
@echo Scintilla Compling ...
@title Scintilla Compling ...
@cd win32
@nmake -f scintilla.mak
@echo Finished! Press any key to continue.
@title Finished!
@pause >nul