param([switch]$Run,[switch]$Test)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$compiler = Get-Command g++.exe -ErrorAction SilentlyContinue
if (-not $compiler) {
    $fallback = 'C:\msys64\ucrt64\bin\g++.exe'
    if (Test-Path -LiteralPath $fallback) { $compiler = Get-Item -LiteralPath $fallback }
    else { throw 'Khong tim thay g++. Xem README.md de cai MSYS2 UCRT64 va FreeGLUT.' }
}
$compilerPath = if ($compiler.Source) { $compiler.Source } else { $compiler.FullName }
New-Item -ItemType Directory -Force -Path build | Out-Null
$sources = @('src/main.cpp','src/ui.cpp','src/view.cpp','src/storage.cpp','src/dice.cpp','src/lab.cpp','src/puzzle.cpp','src/throw_sim.cpp','src/sound.cpp','src/tests.cpp','src/throw_tests.cpp')
& $compilerPath -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude @sources -o build/Dice3D_Edu.exe -lfreeglut -lopengl32 -lglu32 -lgdi32 -lwinmm -static-libgcc -static-libstdc++
if ($LASTEXITCODE -ne 0) { throw 'Bien dich that bai.' }
$toolBin = Split-Path -Parent $compilerPath
foreach ($dll in @('libfreeglut.dll','libwinpthread-1.dll')) {
    $source = Join-Path $toolBin $dll
    if (Test-Path -LiteralPath $source) {
        $destination = Join-Path (Join-Path $PSScriptRoot 'build') $dll
        $needsCopy = -not (Test-Path -LiteralPath $destination)
        if (-not $needsCopy) { $needsCopy = (Get-FileHash -LiteralPath $source).Hash -ne (Get-FileHash -LiteralPath $destination).Hash }
        if ($needsCopy) { Copy-Item -LiteralPath $source -Destination $destination -Force }
    }
}
Write-Host 'Build OK: build\Dice3D_Edu.exe'
if ($Test) { & .\build\Dice3D_Edu.exe --test; if ($LASTEXITCODE -ne 0) { throw 'Tests failed.' } }
if ($Run) { & .\build\Dice3D_Edu.exe }
