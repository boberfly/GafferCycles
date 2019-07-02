#!/usr/bin/env bash

python build.py --platform windows --gafferVersion 0.53.6.2 --version 0.5.3 --cyclesVersion 0.5.3 --docker 1 --upload 0 --forceCxxCompiler cl.exe $@
