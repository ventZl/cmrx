set $_TEST_STEP = 0
target extended-remote localhost:3333
monitor reset halt
load

break TEST_SUCCESS
commands
    disconnect
	quit 0
end

break TEST_FAIL
commands
    bt
    disconnect
	quit 1
end

break TEST_STEP
commands
    if step != ($_TEST_STEP + 1)
        disconnect
        quit 1
    end
    set $_TEST_STEP = step
    continue
end

run
detach
disconnect
quit 2
