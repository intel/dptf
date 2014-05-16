#!/bin/bash

FILETYPE=( .cpp .c .h .sh .txt .mk makefile )

for EXT in "${FILETYPE[@]}"
do 
	find . -iname "*${EXT}" -exec dos2unix '{}' \;
done

