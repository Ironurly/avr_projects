@echo off
cd /d "C:\Users\Mikhail Kaya\Desktop\Baumnka\73b\KursachMPS\Software\KursachV1\default"
avrdude -c usbasp -p m328p -B 32 -U flash:w:"KursachV1.hex":i
pause