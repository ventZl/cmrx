Platform support template
=========================

This directory contains template for creating new specific platforms which need to introduce source code.

Platform support currently boils down to two callbacks:

- thread creation code
- CPU core initialization code
- platform-specific system calls support

In these two situations platform-specific code can be called. Files in this directory can be used as template
for creating new platform.
