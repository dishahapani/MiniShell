#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <limits.h>
#include <pwd.h>

// System call headers 
#include <unistd.h>    // fork, exec, chdir, getcwd, pipe, dup2
#include <sys/wait.h>  // waitpid
#include <sys/stat.h>  // mkdir
#include <fcntl.h>     // open, O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC, O_APPEND
#include <signal.h>    // signal, SIGCHLD
#include <cstring>     // strerror
#include <cerrno>      // errno

struct RedirectInfo {
    std::string inFile;   
    std::string outFile;  
    bool append = false;
};


struct Command {
    std::vector<std::string> args;
    RedirectInfo redirect;
    bool background = false;
};


void sigchldHandler(int) {
    while (waitpid(-1, nullptr, WNOHANG) > 0) {}
}


void showPrompt() {
    // Get username
    struct passwd* pw = getpwuid(getuid());
    std::string user = pw ? pw->pw_name : "user";

    // Get hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0)
        strcpy(hostname, "localhost");

    // Get current working directory
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd)))
        strcpy(cwd, "?");

    // Shorten home directory to "~"
    std::string cwdStr(cwd);
    const char* home = getenv("HOME");
    if (home && cwdStr.find(home) == 0) {
        cwdStr = "~" + cwdStr.substr(strlen(home));
    }

    std::cout << "\033[1;32m" << user << "@" << hostname  // green: user@host
              << "\033[0m:\033[1;34m" << cwdStr            // blue: cwd
              << "\033[0m$ ";                              // reset: $
    std::cout.flush();
}

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string word;
    while (stream >> word) tokens.push_back(word);
    return tokens;
}


Command parseTokensToCommand(const std::vector<std::string>& tokens) {
    Command cmd;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == "<" && i + 1 < tokens.size()) {
            cmd.redirect.inFile = tokens[++i];
        } else if (tokens[i] == ">>" && i + 1 < tokens.size()) {
            cmd.redirect.outFile = tokens[++i];
            cmd.redirect.append = true;
        } else if (tokens[i] == ">" && i + 1 < tokens.size()) {
            cmd.redirect.outFile = tokens[++i];
            cmd.redirect.append = false;
        } else if (tokens[i] == "&") {
            cmd.background = true;
        } else {
            cmd.args.push_back(tokens[i]);
        }
    }
    return cmd;
}

std::vector<Command> splitByPipe(const std::vector<std::string>& tokens) {
    std::vector<Command> commands;
    std::vector<std::string> current;

    for (const std::string& tok : tokens) {
        if (tok == "|") {
            if (!current.empty()) {
                commands.push_back(parseTokensToCommand(current));
                current.clear();
            }
        } else {
            current.push_back(tok);
        }
    }
    if (!current.empty())
        commands.push_back(parseTokensToCommand(current));

    return commands;
}

bool applyRedirects(const RedirectInfo& r) {
    if (!r.inFile.empty()) {
        int fd = open(r.inFile.c_str(), O_RDONLY);
        if (fd < 0) { perror(("myshell: open " + r.inFile).c_str()); return false; }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (!r.outFile.empty()) {
        int flags = O_WRONLY | O_CREAT | (r.append ? O_APPEND : O_TRUNC);
        int fd = open(r.outFile.c_str(), flags, 0644);
        if (fd < 0) { perror(("myshell: open " + r.outFile).c_str()); return false; }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    return true;
}

std::vector<char*> toCArgs(const std::vector<std::string>& args) {
    std::vector<char*> cargs;
    for (const auto& s : args)
        cargs.push_back(const_cast<char*>(s.c_str()));
    cargs.push_back(nullptr);
    return cargs;
}


bool handleBuiltin(const Command& cmd) {
    if (cmd.args.empty()) return false;
    const std::string& name = cmd.args[0];

    // exit
    if (name == "exit") {
        std::cout << "Goodbye!\n";
        exit(0);
    }

    // cd [dir]
    if (name == "cd") {
        const char* home = getenv("HOME");
        std::string dir = (cmd.args.size() > 1) ? cmd.args[1] : (home ? home : "/");
        if (chdir(dir.c_str()) != 0)
            std::cerr << "myshell: cd: " << dir << ": " << strerror(errno) << "\n";
        return true;
    }

    // pwd
    if (name == "pwd") {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd))) std::cout << cwd << "\n";
        return true;
    }

    // mkdir <dir>
    if (name == "mkdir") {
        if (cmd.args.size() < 2) {
            std::cerr << "myshell: mkdir: missing operand\n";
        } else if (::mkdir(cmd.args[1].c_str(), 0755) != 0) {
            perror(("myshell: mkdir: " + cmd.args[1]).c_str());
        }
        return true;
    }

    // history
    // (handled in main, so we just return false here)

    return false;
}

void executeSingle(const Command& cmd) {
    if (cmd.args.empty()) return;
    if (handleBuiltin(cmd)) return;

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "myshell: fork failed: " << strerror(errno) << "\n";
        return;
    }

    if (pid == 0) {
        // ── CHILD ──
        if (!applyRedirects(cmd.redirect)) _exit(1);
        auto cargs = toCArgs(cmd.args);
        execvp(cargs[0], cargs.data());
        std::cerr << "myshell: " << cmd.args[0] << ": command not found\n";
        _exit(1);
    }

    // ── PARENT ──
    if (cmd.background) {
        std::cout << "[background] PID: " << pid << "\n";
        // sigchldHandler will reap it when done
    } else {
        waitpid(pid, nullptr, 0);
    }
}

void executePiped(const std::vector<Command>& commands, bool background) {
    int n = (int)commands.size();

    // Create all pipes upfront using a flat vector
    // pipefds[i*2]   = read  end of pipe i
    // pipefds[i*2+1] = write end of pipe i
    std::vector<int> pipefds((n - 1) * 2);
    for (int i = 0; i < n - 1; i++) {
        if (pipe(&pipefds[i * 2]) < 0) {
            perror("myshell: pipe");
            return;
        }
    }

    std::vector<pid_t> pids;

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("myshell: fork"); return; }

        if (pid == 0) {
            // ── CHILD i ──

            // Wire stdin from previous pipe (not for first command)
            if (i > 0)
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);

            // Wire stdout to next pipe (not for last command)
            if (i < n - 1)
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);

            // Close all pipe fds — we already dup'd what we need
            for (int fd : pipefds) close(fd);

            // Apply file redirects (< and > still work on pipe endpoints)
            if (!applyRedirects(commands[i].redirect)) _exit(1);

            auto cargs = toCArgs(commands[i].args);
            execvp(cargs[0], cargs.data());
            std::cerr << "myshell: " << commands[i].args[0] << ": command not found\n";
            _exit(1);
        }

        pids.push_back(pid);
    }

    // Parent closes all pipe fds AFTER forking all children
    // (crucial — otherwise children waiting for EOF never get it)
    for (int fd : pipefds) close(fd);

    if (background) {
        std::cout << "[background pipeline] PIDs:";
        for (pid_t p : pids) std::cout << " " << p;
        std::cout << "\n";
    } else {
        for (pid_t p : pids) waitpid(p, nullptr, 0);
    }
}

int main() {
    // Register SIGCHLD handler to auto-reap background children
    signal(SIGCHLD, sigchldHandler);

    char hostname[256]; gethostname(hostname, sizeof(hostname));
    struct passwd* pw = getpwuid(getuid());
    std::cout << "── myshell ──  (type 'exit' to quit, 'history' to see commands)\n";

    std::vector<std::string> history;

    std::string line;
    while (true) {
        showPrompt();

        if (!std::getline(std::cin, line)) {
            std::cout << "\nGoodbye!\n";
            break;
        }

        if (line.empty()) continue;

        // Save to history (skip duplicates of last entry)
        if (history.empty() || history.back() != line)
            history.push_back(line);

        // ── Built-in: history ──
        if (line == "history") {
            for (size_t i = 0; i < history.size(); i++)
                std::cout << "  " << i + 1 << "  " << history[i] << "\n";
            continue;
        }

        if (line == "!!") {
            if (history.size() < 2) {
                std::cout << "myshell: no previous command\n";
                continue;
            }
    
            line = history[history.size() - 2];
            std::cout << line << "\n"; // echo it like bash does
        }

        // Tokenize and parse into Command structs
        std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty()) continue;

        std::vector<Command> commands = splitByPipe(tokens);
        if (commands.empty()) continue;

        // Check if the whole pipeline should run in background
        bool background = commands.back().background;
        commands.back().background = false; // handled at pipeline level

        if (commands.size() == 1) {
            commands[0].background = background;
            executeSingle(commands[0]);
        } else {
            executePiped(commands, background);
        }
    }

    return 0;
}
