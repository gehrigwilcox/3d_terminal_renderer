# terminal_renderer

A software terminal renderer in C, using [lalib](https://github.com/gehrigwilcox/lalib) 
for linear algebra.

## Building

Requires gcc and g++ (C11 and C++17).

    git clone --recurse-submodules https://github.com/gehrigwilcox/terminal_renderer.git
    cd terminal_renderer
    make

    # Debug build
    make DEBUG=1

    # Run
    make run

## Project Structure

    include/        C-facing headers (render_math.hpp)
    src/            Source files
    src/lib         Files to be compiled and linked against when hot-reloading
    external/lalib/ Linear algebra library (submodule)
    docs/           Reference material

## Dependencies

- [lalib](https://github.com/gehrigwilcox/lalib) — vendored as a submodule, 
  no manual install needed