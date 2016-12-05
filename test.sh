#! /bin/bash

mkdir /tmp/hw3
mkdir /tmp/dir
echo "ls ls" | ./hwcopy
diff -s /bin/ls /tmp/ls
echo ""
echo "cp /alias/cp" | ./hwcopy
diff -s /bin/cp /tmp/hw3/cp
echo ""
echo "cp /alias2/cp" | ./hwcopy
diff -s /bin/cp /tmp/dir/cp
echo ""
echo "../mkdir /mkdir" | ./hwcopy
echo ""
echo "mkdir ../mkdir" | ./hwcopy
echo ""
echo "mkdir /dir/../mkdir" | ./hwcopy
diff -s /bin/mkdir /tmp/mkdir
