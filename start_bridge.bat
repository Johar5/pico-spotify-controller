@echo off
title Pico Spotify Bridge
echo ===================================
echo  Pico Spotify Bridge Controller
echo ===================================
echo Make sure your Pico is plugged in!
echo Starting Host Script...

cd host
python bridge.py

echo.
echo Connection closed or error occurred.
pause
