# Creation de l'arborescence complete MoBot4

$basePath = $PSScriptRoot

# Creer les dossiers
Write-Host "Creating directory structure..." -ForegroundColor Cyan
$folders = @(
    "src\core\types",
    "src\core\utils",
    "src\core\abstract",
    "src\factories",
    "src\measurement_systems\optitrack"
)

foreach ($folder in $folders) {
    $fullPath = Join-Path $basePath $folder
    if (!(Test-Path $fullPath)) {
        New-Item -Path $fullPath -ItemType Directory -Force | Out-Null
        Write-Host "  Created: $folder" -ForegroundColor Green
    } else {
        Write-Host "  Already exists: $folder" -ForegroundColor Yellow
    }
}

# Creer les fichiers vides si necessaire
Write-Host ""
Write-Host "Creating empty source files..." -ForegroundColor Cyan

$files = @(
    "src\main.cpp",
    "src\core\types\MeasurementFrame.h",
    "src\core\types\SystemCapabilities.h",
    "src\core\types\ConnectionConfig.h",
    "src\core\types\AcquisitionConfig.h",
    "src\core\types\PerformanceMetrics.h",
    "src\core\utils\CircularBuffer.h",
    "src\core\utils\CoordinateConverter.h",
    "src\core\utils\CoordinateConverter.cpp",
    "src\core\abstract\IMeasurementSystem.h",
    "src\core\abstract\IMeasurementSystem.cpp",
    "src\factories\SystemFactory.h",
    "src\factories\SystemFactory.cpp",
    "src\measurement_systems\optitrack\OptiTrackConfig.h",
    "src\measurement_systems\optitrack\OptiTrackSystem.h",
    "src\measurement_systems\optitrack\OptiTrackSystem.cpp"
)

foreach ($file in $files) {
    $fullPath = Join-Path $basePath $file
    if (!(Test-Path $fullPath)) {
        New-Item -Path $fullPath -ItemType File -Force | Out-Null
        Write-Host "  Created: $file" -ForegroundColor Green
    } else {
        Write-Host "  Already exists: $file" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Project structure created successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:"
Write-Host "1. Copy the code content into each file"
Write-Host "2. Open the folder in Visual Studio"
Write-Host "3. CMake will detect all files automatically"
Write-Host ""
pause
