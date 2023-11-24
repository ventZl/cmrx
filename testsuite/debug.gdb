set $_TEST_STEP = 0
target extended-remote | openocd -f openocd.cfg -c "gdb_port pipe"
monitor reset halt
load

break TEST_SUCCESS
commands
    detach
	quit 0
end

break TEST_FAIL
commands
    bt
    detach
	quit 1
end

break TEST_STEP
commands
    if step != ($_TEST_STEP + 1)
        detach
        quit 1
    end
    set $_TEST_STEP = step
    continue
end

run
quit 2
