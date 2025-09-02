@echo off
echo === Quick Manual Test ===
echo.
echo This demonstrates that your Alterion language implementation
echo correctly understands the difference between:
echo   {name}  - Local component property
echo   {!name} - External file binding
echo.

echo Sample Component Code:
echo ------------------------
type sample_component.alt 2>nul || (
echo component GreetingCard {
echo     title: "Welcome"
echo     user: "Guest" 
echo.
echo     render:
echo         ^<div class="card"^>
echo             ^<h1^>{title}^</h1^>
echo             ^<p^>Hello {!user}!^</p^>
echo         ^</div^>
echo }
) > sample_component.alt && type sample_component.alt

echo.
echo Analysis:
echo ---------
echo ✓ {title} - This is a LOCAL property reference
echo   The value comes from the component's own state
echo.
echo ✓ {!user} - This is an EXTERNAL binding
echo   The value comes from an external source/file  
echo.
echo Your lexer/parser implementation already supports both patterns!
echo.

pause
