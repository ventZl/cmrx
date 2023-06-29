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
    detach
	quit 1
end
run
quit 2
