@echo off
setlocal

if not exist bin mkdir bin
pushd bin

:: --- Build Profile Configuration ---
:: Default to debug if no argument is provided
set PROFILE=%1
if "%PROFILE%"=="" set PROFILE=debug

:: --- Toolchain Setup ---
set CC=arm-none-eabi-gcc
set OBJCOPY=arm-none-eabi-objcopy
set MCU=-mcpu=cortex-m3 -mthumb

:: --- Base MISRA/Industry Flags ---
set BASE_CFLAGS=%MCU% -std=c99 -Wall -Wextra -Wpedantic -Werror -ffreestanding -nostdlib -ffunction-sections -fdata-sections
set LDFLAGS=%MCU% -T ../bsw/mcal/stm32f103.ld -nostartfiles -Wl,-Map=can_gateway.map,--cref -Wl,--gc-sections

:: --- Apply Profile Flags ---
if /I "%PROFILE%"=="release" (
    echo [BUILD] Starting Bare-Metal Compilation [RELEASE PROFILE]...
    set CFLAGS=%BASE_CFLAGS% -O2 -DNDEBUG
) else (
    echo [BUILD] Starting Bare-Metal Compilation [DEBUG PROFILE]...
    :: -O0 prevents inlining so lines match exactly. -g3 includes maximum debug symbols and macros.
    set CFLAGS=%BASE_CFLAGS% -O0 -g3 -DDEBUG
)

:: 1. Compile MCAL & Startup
%CC% %CFLAGS% -c ../bsw/mcal/startup_stm32f103.c -o startup_stm32f103.o
if %errorlevel% neq 0 exit /b %errorlevel%

%CC% %CFLAGS% -c ../bsw/mcal/Mcu.c -o Mcu.o
if %errorlevel% neq 0 exit /b %errorlevel%

%CC% %CFLAGS% -c ../bsw/mcal/Can.c -o Can.o
if %errorlevel% neq 0 exit /b %errorlevel%

:: 2. Compile Library Sources
%CC% %CFLAGS% -c ../lib/can_ring.c -o can_ring.o
if %errorlevel% neq 0 exit /b %errorlevel%

:: 3. Compile Application
%CC% %CFLAGS% -c ../main.c -o main.o
if %errorlevel% neq 0 exit /b %errorlevel%

:: 4. Link Object Files into ELF
%CC% %LDFLAGS% main.o can_ring.o Can.o Mcu.o startup_stm32f103.o -o can_gateway.elf
if %errorlevel% neq 0 exit /b %errorlevel%

:: 5. Extract Raw Binary
%OBJCOPY% -O binary can_gateway.elf can_gateway.bin
if %errorlevel% neq 0 exit /b %errorlevel%

echo [BUILD] SUCCESS. Binary ready for flashing.

popd