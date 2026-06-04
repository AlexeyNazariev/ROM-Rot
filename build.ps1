$gitPath = 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd'
$env:PATH = $env:PATH + ';' + $gitPath

if (Test-Path "C:\ROM-Rot\build") {
    Remove-Item -Recurse -Force "C:\ROM-Rot\build"
}

New-Item -ItemType Directory -Force -Path "C:\ROM-Rot\build"
Set-Location "C:\ROM-Rot\build"

Write-Output "Configuring CMake..."
& "C:\Program Files\CMake\bin\cmake.exe" .. -DCMAKE_BUILD_TYPE=Release
if ($LastExitCode -ne 0) {
    Write-Error "CMake configuration failed!"
    exit 1
}

Write-Output "Building Project in Release mode..."
& "C:\Program Files\CMake\bin\cmake.exe" --build . --config Release
if ($LastExitCode -ne 0) {
    Write-Error "Build failed!"
    exit 1
}

Write-Output "Build Succeeded!"
