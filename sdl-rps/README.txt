DESCRIPTION
--------------------------------------------------------------------------------

Rock-paper-scissor (RPS) cellular automaton.

USAGE
--------------------------------------------------------------------------------

Configure:

    $ source .configure [options]

See header of `configure.sh` for possible options.

Build and run:

    $ make && make run

FUTURE WORK
--------------------------------------------------------------------------------

The current implementation processes a pixel buffer on the CPU which is
subsequently mapped to a texture and dispatched to the GPU.  This looks rather
pixelated. A suggested solution is to smoothen the pixels using a shader in a
post-processing step.
