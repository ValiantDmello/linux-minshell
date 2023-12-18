# Minishell

Minishell is a simple Unix shell implemented in C, capable of handling sequential commands, pipes, redirection, and conditional execution.

## Features

- Sequential execution of commands separated by semicolons (`;`).
- Pipe (`|`) handling for command chaining.
- Redirection (`>`, `>>`, `<`) support for input and output.
- Conditional execution (`&&`, `||`) for command chaining based on the success or failure of previous commands.
- Background process execution with `&`.
- Basic signal handling for interrupt (`CTRL+C`).

## Usage

1. **Compilation:** Compile the `minishell.c` file using a C compiler (e.g., gcc).

    ```bash
    gcc minishell.c -o minishell
    ```

2. **Execution:** Run the compiled executable.

    ```bash
    ./minishell
    ```

3. **Usage Examples:**

    - Execute a single command:

        ```bash
        ls -l
        ```

    - Sequential execution:

        ```bash
        ls -l; echo "Hello, Minishell"
        ```

    - Pipe handling:

        ```bash
        ls -l | grep "file" | wc -l
        ```

    - Redirection:

        ```bash
        echo "Hello, Minishell" > output.txt
        ```

    - Conditional execution:

        ```bash
        echo "Success" && ls -l || echo "Failure"
        ```

    - Background process:

        ```bash
        sleep 10 &
        ```

## Notes

- The program assumes a maximum of 6 arguments for each command.
- This is a basic implementation and may not handle all edge cases.

Feel free to explore and enhance the Minishell according to your requirements!
