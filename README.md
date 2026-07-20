# MiniShell

A lightweight Unix-like shell implemented in **C++** that supports command execution, built-in commands, input/output redirection, pipes, background processes, and command history.

## Features

- Execute external Linux/Unix commands using `fork()` and `execvp()`
- Built-in commands:
  - `cd`
  - `pwd`
  - `mkdir`
  - `history`
  - `exit`
- Input redirection (`<`)
- Output redirection (`>`)
- Append output redirection (`>>`)
- Multiple command pipelines (`|`)
- Background execution using (`&`)
- Command history
- Execute previous command using `!!`
- Custom shell prompt displaying:
  - Username
  - Hostname
  - Current working directory
- Automatic cleanup of background processes using `SIGCHLD`

---

## Technologies Used

- C++
- POSIX System Calls
- Linux/macOS System Programming APIs

---

## Project Structure

```
MiniShell/
├── shell.cpp
├── README.md
└── .gitignore
```

---

## Compilation

Compile the program using g++:

```bash
g++ shell.cpp -o myshell
```

---

## Running the Shell

```bash
./myshell
```

You will see a prompt similar to:

```
username@hostname:~/current_directory$
```

---

## Supported Commands

### Built-in Commands

| Command | Description |
|---------|-------------|
| `cd <directory>` | Change current directory |
| `pwd` | Print current working directory |
| `mkdir <name>` | Create a new directory |
| `history` | Display previously entered commands |
| `!!` | Execute the previous command |
| `exit` | Exit the shell |

---

## Input / Output Redirection

Input:

```bash
sort < input.txt
```

Output:

```bash
ls > files.txt
```

Append Output:

```bash
echo Hello >> output.txt
```

---

## Pipes

Supports multiple commands connected using pipes.

Example:

```bash
cat file.txt | grep hello | wc -l
```

---

## Background Execution

Run commands in the background using `&`.

Example:

```bash
sleep 10 &
```

The shell immediately returns to accept new commands.

---

## Command History

Display command history:

```bash
history
```

Execute the previous command:

```bash
!!
```

---

## System Calls Used

- `fork()`
- `execvp()`
- `waitpid()`
- `pipe()`
- `dup2()`
- `open()`
- `close()`
- `chdir()`
- `getcwd()`
- `mkdir()`
- `signal()`

---

## Future Improvements

- Command auto-completion
- Arrow key navigation for history
- Environment variable support
- Wildcard expansion
- Job control (`fg`, `bg`, `jobs`)
- Command aliases

---

## Author

**Disha Hapani**
# MiniShell

A lightweight Unix-like shell implemented in **C++** that supports command execution, built-in commands, input/output redirection, pipes, background processes, and command history.

## Features

- Execute external Linux/Unix commands using `fork()` and `execvp()`
- Built-in commands:
  - `cd`
  - `pwd`
  - `mkdir`
  - `history`
  - `exit`
- Input redirection (`<`)
- Output redirection (`>`)
- Append output redirection (`>>`)
- Multiple command pipelines (`|`)
- Background execution using (`&`)
- Command history
- Execute previous command using `!!`
- Custom shell prompt displaying:
  - Username
  - Hostname
  - Current working directory
- Automatic cleanup of background processes using `SIGCHLD`

---

## Technologies Used

- C++
- POSIX System Calls
- Linux/macOS System Programming APIs

---

## Project Structure

```
MiniShell/
├── shell.cpp
├── README.md
└── .gitignore
```

---

## Compilation

Compile the program using g++:

```bash
g++ shell.cpp -o myshell
```

---

## Running the Shell

```bash
./myshell
```

You will see a prompt similar to:

```
username@hostname:~/current_directory$
```

---

## Supported Commands

### Built-in Commands

| Command | Description |
|---------|-------------|
| `cd <directory>` | Change current directory |
| `pwd` | Print current working directory |
| `mkdir <name>` | Create a new directory |
| `history` | Display previously entered commands |
| `!!` | Execute the previous command |
| `exit` | Exit the shell |

---

## Input / Output Redirection

Input:

```bash
sort < input.txt
```

Output:

```bash
ls > files.txt
```

Append Output:

```bash
echo Hello >> output.txt
```

---

## Pipes

Supports multiple commands connected using pipes.

Example:

```bash
cat file.txt | grep hello | wc -l
```

---

## Background Execution

Run commands in the background using `&`.

Example:

```bash
sleep 10 &
```

The shell immediately returns to accept new commands.

---

## Command History

Display command history:

```bash
history
```

Execute the previous command:

```bash
!!
```

---

## System Calls Used

- `fork()`
- `execvp()`
- `waitpid()`
- `pipe()`
- `dup2()`
- `open()`
- `close()`
- `chdir()`
- `getcwd()`
- `mkdir()`
- `signal()`

---

## Future Improvements

- Command auto-completion
- Arrow key navigation for history
- Environment variable support
- Wildcard expansion
- Job control (`fg`, `bg`, `jobs`)
- Command aliases

---

## Author

**Disha Hapani**

B.Tech – Electronics and Communication Engineering

