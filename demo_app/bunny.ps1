# Bunny Framework command wrapper for Windows PowerShell
# This wrapper allows executing: bunny flash, bunny flash clean, etc.

param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$Arguments
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
python "$scriptDir\bunny" @Arguments
exit $LASTEXITCODE
