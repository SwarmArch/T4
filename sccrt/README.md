Runtime library used by the Swarm compiler
==========================================

This is a runtime library in the strict sense that application code should not
explicitly call anything in this library. It is only the compiler toolchain
that inserts calls into this runtime library into compiled application code.
