@echo off
echo ==================================================================
echo   Building and Validating The Klang Farmer under Linux Docker
echo ==================================================================
docker compose -f docker\docker-compose.yml up --build --abort-on-container-exit
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Linux Docker validation failed with code %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
)
echo [SUCCESS] Linux build and pluginval tests passed cleanly!
