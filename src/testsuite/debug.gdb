target extended-remote | openocd -f openocd.cfg -c "gdb_port pipe"
monitor reset halt
load
break TEST_SUCCESS
commands
	quit 0
end
break TEST_FAIL
commands
	quit 1
end
run
quit 1
