# ============================================================================
# Bunny Framework - Setup Environment Variables for Windows PowerShell
# ============================================================================
# This script adds the Bunny Framework directory to the user's PATH
# so you can use 'bunny flash' from any location
# ============================================================================

param(
    [switch]$Force
)

# Get the script's directory (bunny_framework root)
$BunnyDir = Split-Path -Parent $MyInvocation.MyCommand.Path

Write-Host ""
Write-Host "============================================================================"
Write-Host "Bunny Framework - Environment Setup (PowerShell)" -ForegroundColor Cyan
Write-Host "============================================================================"
Write-Host ""
Write-Host "Bunny directory: $BunnyDir"
Write-Host ""

# Check if running as administrator for better UX
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "⚠ Note: This script is not running as Administrator." -ForegroundColor Yellow
    Write-Host "The changes will apply only to the current user's profile." -ForegroundColor Yellow
    Write-Host ""
}

# Get current PATH
$PathEnvVar = [Environment]::GetEnvironmentVariable("Path", "User")

# Check if already in PATH
if ($PathEnvVar -like "*$BunnyDir*") {
    Write-Host "✓ Bunny is already in PATH!" -ForegroundColor Green
    Write-Host "You can now use 'bunny flash' from any location." -ForegroundColor Green
    Write-Host ""
    exit 0
}

# Ask for confirmation
Write-Host "This will add Bunny to your user PATH variable." -ForegroundColor Cyan
Write-Host ""

if (-not $Force) {
    $confirm = Read-Host "Continue? (Y/n)"
    if ($confirm -eq "n" -or $confirm -eq "N") {
        Write-Host "Cancelled." -ForegroundColor Yellow
        exit 0
    }
}

# Add to PATH
Write-Host ""
Write-Host "Adding Bunny to PATH..."

try {
    $newPath = $PathEnvVar + ";" + $BunnyDir
    [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
    
    # Also update the current session
    $env:Path += ";$BunnyDir"
    
    Write-Host ""
    Write-Host "============================================================================"
    Write-Host "✓ Success! Bunny has been added to PATH" -ForegroundColor Green
    Write-Host "============================================================================"
    Write-Host ""
    Write-Host "Note: In some cases, you may need to restart PowerShell for changes to take effect."
    Write-Host ""
    Write-Host "You can now use:"
    Write-Host "  bunny flash" -ForegroundColor Green
    Write-Host "  bunny flash clean" -ForegroundColor Green
    Write-Host "  bunny --help" -ForegroundColor Green
    Write-Host ""
    Write-Host "From any location in your computer."
    Write-Host ""
    
    # Try to verify
    $testPath = [Environment]::GetEnvironmentVariable("Path", "User")
    if ($testPath -like "*$BunnyDir*") {
        Write-Host "✓ PATH updated successfully" -ForegroundColor Green
    }
}
catch {
    Write-Host "✗ Error: Failed to add Bunny to PATH" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    Write-Host "Try running PowerShell as Administrator and try again." -ForegroundColor Yellow
    exit 1
}
