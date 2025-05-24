# Define source and target directories
$SourceDir = "shader"
$TargetDir = "build/Debug"

# Define the full path for the destination of the shader directory
$DestinationShaderDir = Join-Path $TargetDir (Split-Path $SourceDir -Leaf)

# Check if source directory exists
if (-not (Test-Path $SourceDir -PathType Container)) {
    Write-Host "Error: Source directory '$SourceDir' does not exist." -ForegroundColor Red
    exit 1
}

# Check if target base directory (build/Debug) exists, create if not
if (-not (Test-Path $TargetDir -PathType Container)) {
    Write-Host "Creating target base directory: '$TargetDir'" -ForegroundColor Green
    New-Item -ItemType Directory -Force -Path $TargetDir | Out-Null
}

Write-Host "Copying entire directory '$SourceDir' to '$TargetDir'..."

# Use Copy-Item command to copy the entire directory
# -Recurse copies all subdirectories and files within the source directory
# -Force overwrites existing files and directories
try {
    # This command copies the 'shader' folder itself into 'build/Debug'
    Copy-Item -Path $SourceDir -Destination $TargetDir -Recurse -Force -ErrorAction Stop
    Write-Host "Directory '$SourceDir' copied successfully to '$DestinationShaderDir'!" -ForegroundColor Green
} catch {
    Write-Host "Directory copy failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Read-Host "Press any key to continue..."