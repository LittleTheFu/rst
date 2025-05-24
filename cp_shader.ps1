# Define source and target directories
$SourceDir = "shader"
$TargetDir = "build/Debug"

# Check if source directory exists
if (-not (Test-Path $SourceDir -PathType Container)) {
    Write-Host "Error: Source directory '$SourceDir' does not exist." -ForegroundColor Red
    exit 1
}

# Check if target directory exists, create if not
if (-not (Test-Path $TargetDir -PathType Container)) {
    Write-Host "Creating target directory: '$TargetDir'" -ForegroundColor Green
    New-Item -ItemType Directory -Force -Path $TargetDir | Out-Null
}

Write-Host "Copying contents of '$SourceDir' to '$TargetDir'..."

# Use Copy-Item command to copy files and directories
# -Recurse copies all subdirectories and files
# -Force overwrites existing files
# -Container copies content into the target directory if it's a directory
try {
    Copy-Item -Path "$SourceDir\*" -Destination $TargetDir -Recurse -Force -ErrorAction Stop
    Write-Host "Shader files copied successfully!" -ForegroundColor Green
} catch {
    Write-Host "Shader file copy failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Read-Host "Press any key to continue..."