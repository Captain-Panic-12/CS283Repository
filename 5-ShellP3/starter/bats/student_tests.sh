#!/usr/bin/env bats

# File: student_tests.sh


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
    [[ "$output" == *"dsh3>"* ]]
}

@test "Exit command works" {
    run "${current}/dsh" <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}


@test "Change directory and confirm with pwd" {
    run "${current}/dsh" <<EOF
cd "${current}"
pwd
exit
EOF
    [[ "$output" == *"${current}"* ]]
}

@test "Change to a non-existent directory" {
    run "${current}/dsh" <<EOF
cd /this/does/not/exist
exit
EOF
    [[ "$output" == *"Error: -6"* ]]
}

@test "Change to home directory with cd" {
    run "${current}/dsh" <<EOF
cd
pwd
exit
EOF
    [[ "$output" == *"$HOME"* ]]
}

@test "Check pipe command" {
    run ./dsh <<EOF
echo hello | grep hello
EOF

    # Assertions
    [[ "${output}" == *"hello"* ]]
}

@test "Check multiple pipe command" {
    run ./dsh <<EOF
echo hello | grep hello | wc -l
EOF

    # Assertions
    [[ "${output}" == *"1"* ]]
}

@test "Invalid command handling" {
    run "${current}/dsh" <<EOF
invalidcommand
exit
EOF
    [[ "$output" == *"-6"* ]]
}

@test "Check simple command with arguments" {
    run ./dsh <<EOF
echo "hello world"
EOF

    # Assertions
    [ "$status" -eq 0 ]
    [[ "${output}" =~ "hello world" ]]
}
