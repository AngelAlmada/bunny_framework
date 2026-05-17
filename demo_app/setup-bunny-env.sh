#!/bin/bash
################################################################################
# Bunny Framework - Setup Environment Variables for Linux/macOS
################################################################################
# This script adds the Bunny Framework directory to your PATH
# so you can use 'bunny flash' from any location
################################################################################

set -e

# Get the script's directory (bunny_framework root)
BUNNY_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Determine the shell configuration file
if [ -n "$ZSH_VERSION" ]; then
    SHELL_RC="$HOME/.zshrc"
    SHELL_NAME="Zsh"
elif [ -n "$BASH_VERSION" ]; then
    SHELL_RC="$HOME/.bashrc"
    SHELL_NAME="Bash"
else
    # Fallback - try bashrc
    SHELL_RC="$HOME/.bashrc"
    SHELL_NAME="Shell (default)"
fi

# For macOS, also check .bash_profile
if [[ "$OSTYPE" == "darwin"* ]]; then
    if [ ! -f "$SHELL_RC" ] && [ -f "$HOME/.bash_profile" ]; then
        SHELL_RC="$HOME/.bash_profile"
    fi
fi

echo ""
echo "================================================================================"
echo "Bunny Framework - Environment Setup ($SHELL_NAME)"
echo "================================================================================"
echo ""
echo "Bunny directory: $BUNNY_DIR"
echo "Shell config file: $SHELL_RC"
echo ""

# Check if already in PATH
if grep -q "$BUNNY_DIR" "$SHELL_RC" 2>/dev/null; then
    echo "✓ Bunny is already in PATH!"
    echo "You can now use 'bunny flash' from any location."
    echo ""
    exit 0
fi

# Show what we're about to do
echo "This will add Bunny to your PATH by modifying: $SHELL_RC"
echo ""
read -p "Continue? (Y/n) " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Nn]$ ]]; then
    echo "Cancelled."
    exit 0
fi

# Create backup
BACKUP_FILE="${SHELL_RC}.backup.$(date +%s)"
cp "$SHELL_RC" "$BACKUP_FILE"
echo "Backup created: $BACKUP_FILE"

# Add to PATH
echo "" >> "$SHELL_RC"
echo "# Bunny Framework - Added by setup-bunny-env.sh" >> "$SHELL_RC"
echo "export PATH=\"\$PATH:$BUNNY_DIR\"" >> "$SHELL_RC"

echo ""
echo "================================================================================"
echo "✓ Success! Bunny has been added to PATH"
echo "================================================================================"
echo ""
echo "To apply the changes immediately, run:"
echo "  source $SHELL_RC"
echo ""
echo "Or restart your terminal."
echo ""
echo "You can then use:"
echo "  bunny flash"
echo "  bunny flash clean"
echo "  bunny --help"
echo ""
echo "From any location on your computer."
echo ""

# Optional: source the file now
read -p "Apply changes now? (Y/n) " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Nn]$ ]]; then
    source "$SHELL_RC"
    echo "✓ PATH updated in current session"
    echo ""
fi
