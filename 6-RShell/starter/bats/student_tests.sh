#!/usr/bin/env bats

# File: student_tests.sh
#
# Unit tests for the remote shell (rsh) implementation.

# Test 1: Check if the server starts and accepts connections
@test "Test server starts and accepts connections" {
    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2 

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
echo Hello, World!
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello, World!"* ]]

    kill $SERVER_PID
}

# Test 2: Check if the exit command works
@test "Test exit command" {
    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
exit
EOF

    [ "$status" -eq 0 ]

    kill $SERVER_PID
}

# Test 3: Check if the cd command works
@test "Test cd command" {
    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
cd /
pwd
exit
EOF
    echo "Output: $output"
    [[ "$output" == *"/"* ]]
    [ "$status" -eq 0 ]

    kill $SERVER_PID
}

# Test 4: Check if the stop-server command works
@test "Test stop-server command" {
    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
stop-server
EOF

    [ "$status" -eq 0 ]

    sleep 1
    ! kill -0 $SERVER_PID 2>/dev/null
}

# Test 5: Check if commands with pipes work
@test "Test command with pipes" {
    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
echo hello | grep hello
exit
EOF
    echo "Output: $output"
    [[ "$output" == *"hello"* ]]
    [ "$status" -eq 0 ]
}

# Test 6: Check error handling for invalid commands
@test "Test invalid command followed by correct command" {
    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
invalid_command
echo Hello, World!
EOF

    echo "Output: $output"
    [[ "$output" == *"Hello, World!"* ]]
}

# Test 7: Check large output handling
@test "Test large output handling" {
    large_input=$(printf 'A%.0s' {1..70000})
    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
$large_input
exit
EOF
    echo "Output: $output"
    [ "$status" -eq 0 ]
    [[ "$output" == *"Error: Command execution failedcmd loop returned 0"* ]]
}

# Test 8: Check multiple clients
@test "Test multiple clients" {
    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
echo Client 1
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"Client 1"* ]]

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
echo Client 2
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"Client 2"* ]]
}

# Test 9: Check server stop and restart
@test "Test server stop and restart" {
    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
stop-server
EOF

    [ "$status" -eq 0 ]

    sleep 1
    ! kill -0 $SERVER_PID 2>/dev/null

    ./dsh -s -i 127.0.0.1 -p 5678 &
    SERVER_PID=$!
    sleep 2

    run ./dsh -c -i 127.0.0.1 -p 5678 <<EOF
echo Server restarted
EOF

    [ "$status" -eq 0 ]
    [[ "$output" == *"Server restarted"* ]]

    kill $SERVER_PID
}
