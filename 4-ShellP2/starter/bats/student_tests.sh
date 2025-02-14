#!/usr/bin/env bats

setup() {
    current=$(pwd)
    test_dir="/tmp/dsh-test"

    # Ensure cleanup before creating test directory
    rm -rf "$test_dir"
    mkdir -p "$test_dir"

    # Ensure 'dsh' exists and is executable
    if [ ! -x "${current}/dsh" ]; then
        echo "Error: dsh executable not found or not executable" >&2
        exit 1
    fi
}

teardown() {
    rm -rf "$test_dir"
}

@test "Shell prompt appears" {
    run "${current}/dsh" <<EOF
exit
EOF
    [[ "$output" == *"dsh2>"* ]]
}

@test "Exit command works" {
    run "${current}/dsh" <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Basic command execution (ls)" {
    run "${current}/dsh" <<EOF
ls
exit
EOF
    [[ "$output" == *"dsh2>"* ]]
}

@test "Which command for ls" {
    run "${current}/dsh" <<EOF
which ls
exit
EOF
    [[ "$output" == *"/bin/ls"* ]]
}

@test "Echo command with quotes" {
    run "${current}/dsh" <<EOF
echo "hello   world"
exit
EOF
    [[ "$output" == *"hello   world"* ]]
}

@test "Change directory and confirm with pwd" {
    run "${current}/dsh" <<EOF
cd /tmp
pwd
exit
EOF
    [[ "$output" == *"/tmp"* ]]
}

@test "Change to home directory with cd" {
    run "${current}/dsh" <<EOF
cd
pwd
exit
EOF
    [[ "$output" == *"$HOME"* ]]
}

@test "Change to a non-existent directory" {
    run "${current}/dsh" <<EOF
cd /this/does/not/exist
exit
EOF
    [[ "$output" == *"cd: No such file or directory"* ]]
}

@test "Invalid command handling" {
    run "${current}/dsh" <<EOF
invalidcommand
exit
EOF
    [[ "$output" == *"-6"* ]]
}

@test "Handling of unterminated quotes" {
    run "${current}/dsh" <<EOF
echo "unterminated string
exit
EOF
    [[ "$output" == *"-6"* ]]
}

@test "Multiple spaces between arguments" {
    run "${current}/dsh" <<EOF
echo    hello       world
exit
EOF
    [[ "$output" == *"hello world"* ]]
}

@test "Pipeline handling (ls | grep dsh-test)" {
    run "${current}/dsh" <<EOF
ls /tmp | grep dsh-test
exit
EOF
    [[ "$output" == *"dsh-test"* ]]
}

@test "Background process handling (sleep 2 &)" {
    run "${current}/dsh" <<EOF
sleep 2 &
exit
EOF
    [[ "$output" == *"&"* ]]
}
