This is a demo of the Comms-CCF project using
the included FreeRTOS-Demo/.

First go into FreeRTOS-Demo/

	cd FreeRTOS-Demo/

Then compile it and run it

	cmake --fresh --preset=debug &&
	cmake --build --preset=debug --target=run |
		"$ROOT/recording/simplify-paths"

Now in another window, you can open the RPC
(the run target redirects the UART0 from qemu
to localhost:4321)

	python python/tcp.py --host localhost --port 4321

This opens up a REPL where you can execute
functions on the remote.
