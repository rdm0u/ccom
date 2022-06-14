#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$input => $expected expected, but got $actual"
	exit 1
    fi
}

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 41 "  12  +  34  -5"
assert 47 '5 +   6*7'
assert 50 ' 3 * 3 + (4 + 7) + (6 *5)'
assert 30 ' (6 * 5) '
assert 3 '(2+4)/2'
assert 10 '- 10 + 20'
assert 9 ' -3*-3'
assert 0 '0 == 1'
assert 1 '10 - 1 == 9 * 1'
assert 1 ' 0*1 != 1*1'
assert 0 ' 42*1 != 42 * 1'

assert 1 '1 > 0'
assert 0 '1 > 1'
assert 1 '2 >= 1'
assert 1 '1>=1'
assert 0 ' 0>=1'

assert 1 '0 < 2'
assert 0 '1 < 1'
assert 1 '1 <= 2'
assert 1 '1<=1'
assert 0 ' 1 <= 0'

echo OK
