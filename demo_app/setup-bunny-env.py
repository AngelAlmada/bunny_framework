#!/usr/bin/env python3
"""
Bunny Framework - Universal Setup Script for Environment Variables
This script detects your operating system and runs the appropriate setup
"""

import os
import platform
import subprocess
import sys
from pathlib import Path


def setup_windows_cmd():
    """Setup for Windows CMD"""
    script_path = Path(__file__).parent / "setup-bunny-env.bat"
    
    print("\n" + "=" * 80)
    print("Bunny Framework - Windows CMD Setup")
    print("=" * 80)
    print(f"\nRunning: {script_path}\n")
    
    try:
        subprocess.run(["cmd.exe", "/c", str(script_path)], check=True)
    except subprocess.CalledProcessError:
        return False
    
    return True


def setup_windows_powershell():
    """Setup for Windows PowerShell"""
    script_path = Path(__file__).parent / "setup-bunny-env.ps1"
    
    print("\n" + "=" * 80)
    print("Bunny Framework - Windows PowerShell Setup")
    print("=" * 80)
    print(f"\nRunning: {script_path}\n")
    
    try:
        # Run PowerShell script
        subprocess.run([
            "powershell.exe",
            "-ExecutionPolicy", "Bypass",
            "-File", str(script_path)
        ], check=True)
    except subprocess.CalledProcessError:
        return False
    
    return True


def setup_unix():
    """Setup for Linux/macOS"""
    script_path = Path(__file__).parent / "setup-bunny-env.sh"
    
    print("\n" + "=" * 80)
    print("Bunny Framework - Unix/Linux/macOS Setup")
    print("=" * 80)
    print(f"\nRunning: {script_path}\n")
    
    try:
        # Make script executable
        os.chmod(script_path, 0o755)
        
        # Run the script
        subprocess.run(["/bin/bash", str(script_path)], check=True)
    except subprocess.CalledProcessError:
        return False
    
    return True


def main():
    """Detect OS and run appropriate setup"""
    system = platform.system().lower()
    
    print("\n" + "=" * 80)
    print("Bunny Framework - Environment Setup")
    print("=" * 80)
    print(f"\nDetected OS: {system.upper()}")
    
    if system == "windows":
        print("\nWhich shell do you use?")
        print("  1) CMD (Command Prompt) - Traditional Windows shell")
        print("  2) PowerShell - Modern Windows shell")
        print("  3) Both")
        
        try:
            choice = input("\nEnter choice (1-3) [default: 1]: ").strip() or "1"
        except (KeyboardInterrupt, EOFError):
            print("\n\nCancelled.")
            return 1
        
        success = False
        
        if choice in ("1", "3"):
            if not setup_windows_cmd():
                print("\n✗ CMD setup failed")
            else:
                success = True
        
        if choice in ("2", "3"):
            if not setup_windows_powershell():
                print("\n✗ PowerShell setup failed")
            else:
                success = True
        
        if not success:
            return 1
    
    elif system in ("linux", "darwin"):
        if not setup_unix():
            print("\n✗ Unix setup failed")
            return 1
    
    else:
        print(f"\n✗ Unsupported operating system: {system}")
        return 1
    
    print("\n" + "=" * 80)
    print("✓ Setup complete!")
    print("=" * 80)
    print("\nYou can now use 'bunny' command from anywhere:")
    print("  bunny flash")
    print("  bunny flash clean")
    print("  bunny --help")
    print()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
