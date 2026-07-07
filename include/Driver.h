#ifndef DRIVER_H
#define DRIVER_H

/// MainLoop - Main Compiler loop. Reads the token in the
/// top of the program and dispatches to the right handles
/// until it finds EOF.
void MainLoop();

#endif // DRIVER_H