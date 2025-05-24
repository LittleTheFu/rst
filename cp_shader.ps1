# Define the source directory (where the script is located)
$SourceDir = Get-Location

# Define the target directory
$TargetDir = "build/Debug"

# Check if the target directory exists
if (Test-Path -Path $TargetDir -PathType Container) {
    Write-Host "Target directory '$TargetDir' exists."
    Write-Host "Copying .vert and .frag files..."

    # Copy .vert files
    Get-ChildItem -Path "$SourceDir\*.vert" | Copy-Item -Destination $TargetDir

    # Copy .frag files
    Get-ChildItem -Path "$SourceDir\*.frag" | Copy-Item -Destination $TargetDir

    Write-Host "File copy complete."
} else {
    Write-Host "Target directory '$TargetDir' does not exist, skipping copy."
}

# Get the current date and time with seconds
$CurrentDateTime = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

# Print the current time
Write-Host "Current time: $CurrentDateTime"

Read-Host "Press any key to continue..."