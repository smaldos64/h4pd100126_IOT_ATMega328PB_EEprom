@echo off
setlocal

if "%~1"=="" (
    echo Advarsel: Du skal angive et mappenavn som argument!
    echo Eksempel: %~nx0 AD_Konverter
    exit /b 1
)

SET "PROJECT_DIR=%~1"
SET "ELF_FILE_DIR=./out/%PROJECT_DIR%"
SET "ELF_FILE=%ELF_FILE_DIR%/default.elf"

if not exist "%ELF_FILE%" (
    echo Fejl: Kunne ikke finde %ELF_FILE%
    exit /b 1
)

REM 1. Generer listing-filen (.lst)
avr-objdump -h -S "%ELF_FILE%" > "%ELF_FILE_DIR%/projekt_oversigt.lst"
if errorlevel 1 (
    echo Fejl ved generering af projekt_oversigt.lst
    exit /b 1
)

REM 2. Generer symbol- og RAM-tabellen (.sym)
avr-nm -n -S "%ELF_FILE%" > "%ELF_FILE_DIR%/projekt_variable.sym"
if errorlevel 1 (
    echo Fejl ved generering af projekt_variable.sym
    exit /b 1
)

REM 3. Vis og gem Flash-, RAM- og EEPROM-forbrug
avr-size -C --mcu=atmega328pb "%ELF_FILE%"
avr-size -C --mcu=atmega328pb "%ELF_FILE%" > "%ELF_FILE_DIR%/projekt_forbrug.txt"
if errorlevel 1 (
    echo Fejl ved generering af projekt_forbrug.txt
    exit /b 1
)

echo Faerdig! Filerne er genereret i %ELF_FILE_DIR%.
endlocal