# Custom Unix Shell in C

## Overview

Welcome to my custom Unix shell written in C! This shell is designed to provide an advanced command-line environment with features such as symbol tables, word expansion, tilde expansion, and more. Whether you're a Unix veteran or a newcomer, this shell aims to enhance your command-line experience.

## Features

This Unix shell project includes the following features:

- **Command Execution:** Execute basic commands like `ls`, `cd`, `pwd`, `exit`, and more.

- **Job Control:** Bring jobs to the foreground with `fg`, send them to the background with `bg`, and list current jobs with `jobs`.

- **Built-in Commands:** Access a variety of commands built directly into the shell.

- **Signal Handling:** Handle signals like `SIGINT` (Ctrl+C) and `SIGTSTP` (Ctrl+Z) gracefully.


- **Custom Prompt:** Customize your shell prompt to suit your preferences.

- **Symbol Tables:** Efficiently manage variables and streamline command parsing using advanced symbol tables.

- **Word Expansion:** Experience flexible and dynamic command input with powerful word expansion capabilities.

- **Tilde Expansion:** Seamlessly reference home directories using integrated tilde expansion for convenient shortcuts.

- **...and more:** The shell incorporates various advanced concepts to deliver a comprehensive Unix experience.

## Building the Project

To build this project, you will need to have `make` and `gcc` installed on your system. Once you have these prerequisites, you can build the project by following these steps:

1. Open a terminal and navigate to the root directory of the project.

2. Run the `make` command:

    ```bash
    make
    ```

## File Structure
```plaintext
.
├── builtins
│   ├── builtins.c
│   └── dump.c
├── executor.c
├── executor.h
├── initsh.c
├── main.c
├── Makefile
├── node.c
├── node.h
├── parser.c
├── parser.h
├── pattern.c
├── prompt.c
├── Readme.md
├── scanner.c
├── scanner.h
├── shell.h
├── shunt.c
├── source.c
├── source.h
├── strings.c
├── symtab
│   ├── symtab.c
│   └── symtab.h
└── wordexp.c
```

## Code Formatting

This project uses `clang-format` for code formatting. The style is defined in the `.clang-format` file in the root directory of the project.

To format your code, navigate to the root directory of the project in your terminal and run the following command:

```bash
find . -iname '*.c' -o -iname '*.h' | xargs clang-format -i
```

## Author

This project was created by [Huzaifa Naseer](https://github.com/HN026).

Connect with me on [LinkedIn](https://linkedin.com/in/huzaifanaseer).