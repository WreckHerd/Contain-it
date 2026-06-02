#!/bin/sh

# ANSI Color Codes
GREEN='\033[1;32m'
RED='\033[1;31m'
CYAN='\033[1;36m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  Contain-It Environment Verification   ${NC}"
echo -e "${CYAN}========================================${NC}\n"

# 1. PID Namespace Test (Process IDs)
echo -e "${YELLOW}[TEST] Checking PID Isolation...${NC}"
# In an isolated namespace, the script's PID will be extremely low (usually < 10)
SHELL_PID=$$
if [ "$SHELL_PID" -lt 10 ]; then
    echo -e "${GREEN}[PASS] PID Namespace active. Current script PID: $SHELL_PID${NC}"
else
    echo -e "${RED}[FAIL] High PID detected ($SHELL_PID). Host processes leaked!${NC}"
fi

# 2. Virtual Filesystem Test (/proc)
echo -e "\n${YELLOW}[TEST] Checking Virtual Filesystems...${NC}"
if mount | grep -q "proc on /proc"; then
    echo -e "${GREEN}[PASS] /proc is mounted correctly.${NC}"
else
    echo -e "${RED}[FAIL] /proc is missing! top/ps commands will fail.${NC}"
fi

# 3. Memory Limit Test (The OOM Trap)
echo -e "\n${YELLOW}[TEST] Checking Memory Cgroup Limits...${NC}"
# We run an infinite array allocation in awk inside a subshell.
# We wrap it in a 3-second timeout. 
# If the memory limit works, the kernel will kill it instantly (Exit Code 137).
# If the limit fails, it will hit the timeout and survive (Exit Code 143 or 0).
timeout 3s awk 'BEGIN { while(1) a[++i]="A" }' > /dev/null 2>&1
EXIT_CODE=$?

if [ $EXIT_CODE -eq 137 ] || [ $EXIT_CODE -eq 128 ]; then
    echo -e "${GREEN}[PASS] Kernel successfully OOM-killed the memory stress test.${NC}"
else
    echo -e "${RED}[FAIL] Memory test survived. Limits are not being enforced! (Exit Code: $EXIT_CODE)${NC}"
fi

echo -e "\n${CYAN}========================================${NC}"
echo -e "${GREEN}      VERIFICATION COMPLETE             ${NC}"
echo -e "${CYAN}========================================${NC}\n"