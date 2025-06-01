# Shellify

<div align="center">
  <img src="https://github.com/user-attachments/assets/50b51327-2729-4922-ae90-5466619997f2" alt="Shellify Logo" width="200">
</div>

Shellify is a simple, lightweight shell implementation written in C. It provides core shell functionality with a clean, straightforward interface.

## ✨ Features

Shellify currently supports:

-   Basic command execution
-   Command-line argument parsing with support for spaces and special characters
-   Built-in commands:
    -   `cd` - Change directory (with support for empty argument to navigate to HOME)
    -   `pwd` - Print working directory
    -   `path` - Set search paths for executable discovery
    -   `exit` - Exit the shell
-   I/O redirection with `>` operator
-   Command piping with `|` operator (multiple pipes supported)
-   Path search for executable programs
-   Signal handling
-   Quoted argument handling (both single and double quotes)
-   Process group management for proper job control

## 📥 Installation

To build and install Shellify:

```bash
git clone https://github.com/AhmedSobhy01/shellify.git
cd shellify
make
```

## 🚀 Usage

After building, run the shell:

```bash
./shellify
```

### 💻 Basic Commands

```bash
shellify> ls -l                # Run a command with arguments
shellify> cd /path/to/dir      # Change directory
shellify> cd                   # Change to HOME directory
shellify> pwd                  # Show current directory
shellify> ls -la > output.txt  # Redirect output to a file
shellify> exit                 # Exit the shell
```

### 🔄 Command Piping

```bash
shellify> ls -l | grep ".txt"                     # Pipe ls output to grep
shellify> cat file.txt | sort | uniq              # Chain multiple pipes
shellify> ps aux | grep bash | wc -l              # Count bash processes
shellify> find . -type f | grep "\.c$" > src.txt  # Combine pipes with redirection
```

### 🛣️ Path Management

Set directories where Shellify should look for executables:

```bash
shellify> path /bin /usr/bin /usr/local/bin
```

Using no arguments clears the path:

```bash
shellify> path
```

### 💬 Quoted Arguments

Shellify supports quoted arguments for handling spaces and special characters:

```bash
shellify> echo "Hello World"                  # Double quotes
shellify> touch "my file.txt"                 # Spaces in filenames
shellify> echo 'Single quoted string'         # Single quotes
shellify> grep "search term" "file with spaces.txt"
```

Quotes also support multi-token arguments:

```bash
shellify> echo "This is all one argument despite the spaces"
```

## 🔧 Implementation Details

Shellify is structured into several modules:

-   **shellify.c/h**: Main shell logic, command processing, signal handling, and pipeline execution
-   **commands.c/h**: Built-in command implementations and path resolution
-   **utils.c/h**: Utility functions for argument parsing and command handling

### Architecture

The shell follows a modular design:

1. **Input Processing**: Command-line input is read and parsed into arguments
2. **Command Execution**:
    - Built-in commands are handled internally
    - External commands are executed by searching the PATH
    - Pipelines create multiple processes with connected I/O
3. **Signal Handling**: Custom handlers manage interrupts and terminal control
4. **Process Management**: Process groups are used for job control

### Signal Handling

Shellify implements proper signal handling to manage:

-   Ctrl+C (SIGINT) for interrupting running commands
-   Terminal control signals for managing foreground/background processes
-   Process groups to ensure signals are delivered properly

## 🔮 Future Plans

-   Command history navigation with the arrow keys
-   Support for environment variables
-   Input/output redirection for standard error

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.
