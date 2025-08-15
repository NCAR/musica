# PowerShell script to set up Windows build environment
Write-Host "Setting up Windows build environment..."

# The GitHub Actions Windows runners already have various compilers installed
# We need to ensure GFortran is available and in PATH

# Check if gfortran is already available
try {
    $gfortranVersion = & gfortran --version 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "GFortran already available:"
        Write-Host $gfortranVersion
        exit 0
    }
} catch {
    Write-Host "GFortran not found in PATH, setting up..."
}

# Use winget to install MSYS2 which includes GFortran
Write-Host "Installing MSYS2 with GFortran..."
winget install --id MSYS2.MSYS2 --silent --accept-package-agreements --accept-source-agreements

# Add MSYS2 to PATH for this session
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;" + $env:PATH

# Update MSYS2 and install GFortran
Write-Host "Installing GFortran through MSYS2..."
& C:\msys64\usr\bin\bash.exe -lc "pacman -Syu --noconfirm"
& C:\msys64\usr\bin\bash.exe -lc "pacman -S --noconfirm mingw-w64-x86_64-gcc-fortran"

# Verify installation
Write-Host "Verifying GFortran installation..."
& C:\msys64\mingw64\bin\gfortran.exe --version

Write-Host "Windows build environment setup complete."

# Export the PATH for subsequent steps
Write-Host "##vso[task.setvariable variable=PATH]C:\msys64\mingw64\bin;C:\msys64\usr\bin;$env:PATH"
