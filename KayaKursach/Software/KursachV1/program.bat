@echo off
cd /d "%~dp0"
echo Programming KursachV1.hex...
avrdude -c usbasp -p m328p -B 32 -U flash:w:"KursachV1.hex":i
if %errorlevel% == 0 (
    echo ✅ SUCCESS! Arduino programmed!
) else (
    echo ❌ FAILED! Check connections.
)
pause