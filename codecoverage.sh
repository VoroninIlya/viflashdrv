#!/bin/bash

lcov -t "tst_viflashdrv" -o tst_viflashdrv.info -c -d .
genhtml -o report tst_viflashdrv.info