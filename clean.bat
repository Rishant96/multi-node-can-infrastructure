@echo off
echo [CLEAN] Purging build artifacts...
del /q *.o *.elf *.bin *.map 2>nul
echo [CLEAN] Directory clean.
