#!/bin/bash

# Define the program name and the source file
PROGRAM="barber"
SOURCE="barber.c"

# Compile the source code
gcc -o $PROGRAM $SOURCE -lpthread

# Check if the compilation was successful
if [ $? -ne 0 ]; then
  echo "Compilation failed"
  exit 1
fi

# Define the number of chairs and the number of test runs
CHAIRS=(5 10 12) # Test with different numbers of chairs
INFO_MODES=("" "-info") # Test with and without info mode

# Loop through the test cases
for chairs in "${CHAIRS[@]}"; do
  for info_mode in "${INFO_MODES[@]}"; do
    echo "Running test with $chairs chairs $info_mode"
    if [ -z "$info_mode" ]; then
      ./$PROGRAM $chairs
    else
      ./$PROGRAM $chairs $info_mode
    fi

    # Check the exit status of the program
    if [ $? -ne 0 ]; then
      echo "Test with $chairs chairs $info_mode failed"
    else
      echo "Test with $chairs chairs $info_mode succeeded"
    fi
  done
done

# Clean up
rm $PROGRAM
