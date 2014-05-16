@echo off
REM :: To Rebuild DSP.DV, do the following:
REM ::  1. Get Latest Version in your Workspace, including Packages\DSP
REM ::  2. Build DEBUG version of dsp_compiler.exe in your Workspace
REM ::  3. Build DEBUG x64 version of esif_uf_64.exe in your Workspace
REM ::  4. Check out dsp.dv file from Perforce in your Workspace
REM ::  5. Open a Command Prompt in Administrator Mode
REM ::  6. CD to your [Workspace\Packages\DSP] folder and run this script
REM ::  7. After successful build, check dsp.dv into Perforce
REM ::
SET DSP_DIR=%CD%
SET ESIF_ROOT=%CD%\..\..
SET DSP_COMPILER="%ESIF_ROOT%\Packages\DSP Compiler\sources\win\projs\Debug\dsp_compiler.exe"
SET ESIF_DIR=%ESIF_ROOT%\Products\ESIF_UF\Sources\win\projs
SET ESIF_UF="%ESIF_DIR%\x64\Win8Debug\esif_uf_64.exe"
SET ESIF_DV=%WINDIR%\ServiceProfiles\LocalService\AppData\Local\Intel\DPTF

if not exist %DSP_COMPILER% (
	echo %DSP_COMPILER% Not Found
	echo Please Build and run again
	goto :eof
)
if not exist %ESIF_UF% (
	echo %ESIF_UF% Not Found
	echo Please Build and run again
	goto :eof
)

CD /D "%DSP_DIR%"
if /i "%1"=="GO" goto :eof

echo ** Builing .edp files ...
for %%f in (*.dsp) do (
	%DSP_COMPILER% %%~nf.dsp %%~nf.edp
)

CD /D "%ESIF_DIR%"
echo.
echo ** Builing dsp.dv ...
%ESIF_UF% server -f ..\..\..\..\..\Packages\DSP\dspdv.scr

CD /D "%DSP_DIR%"

echo.
echo ** Copying dsp.dv ...
if exist "%ESIF_DV%\dsp.dv" copy /y "%ESIF_DV%\dsp.dv"

echo.
echo ** Completed. Affected File List:

dir *.edp dsp.dv

echo.
echo ** Verify Timestamps on above files. If correct, check into Perforce
echo ** Be sure to checkout dsp.dv and *.edp files from Perforce before building
