#! /bin/bash

if ! [[ -x wish ]]; then
    echo "wish executable does not exist"
    exit 1
fi

echo "Starting tests with run-tests.sh"

../tester/run-tests.sh $*

echo "Tests completed"
