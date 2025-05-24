# Define source and target directories
$SourceDir = "resource"
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

Write-Host "Copying '$SourceDir' to '$TargetDir'..."

# Use Copy-Item command to copy files and directories
# -Recurse copies all subdirectories and files
# -Force overwrites existing files
# -Container copies content into the target directory if it's a directory
# -Exclude parameter can be used to exclude specific files or folders
# For copying only new/updated files, PowerShell's default behavior is more complex than xcopy -D,
# often requiring additional logic or more advanced commands, but sufficient for simple copying.
try {
    Copy-Item -Path "$SourceDir\*" -Destination $TargetDir -Recurse -Force -ErrorAction Stop
    Write-Host "Files copied successfully!" -ForegroundColor Green
} catch {
    Write-Host "File copy failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Read-Host "Press any key to continue..."