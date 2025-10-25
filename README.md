[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/J7xkl66X)

# Mini Shell

## Understanding Project Structure

The project already contains `tokenizer.cc` and `command.cc` files

## Modifications To Fix Existing Structure and Issues

- Modify the main loop of the `prompt()` function to a while loop that calls `tokenize()`, `parse()`, and `execute()` functions in sequence instead of recursively calling the functions at the end of each one.
- Compiler warning: In the `Command` class constructor, it uses `_numberOfSimpleCommands` before initializing it. In fact, it should actually use the `_numberOfAvailableSimpleCommands` variable instead. It also made me realize that the other flags such as `_append` and `_out_error` were not initialized to `0`.