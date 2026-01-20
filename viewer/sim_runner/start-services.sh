#!/bin/bash

cd /app/viewer

# Start the C# WebSocket server in the background
dotnet run "tcp://0.0.0.0:2222" "658f9b28-6686-40d2-8800-611bd8466215" &

# Start the simulation
cd /app/simulation
dotnet run --project DynStack.SimulationRunner --sim HS --url tcp://0.0.0.0:2222 --id 658f9b28-6686-40d2-8800-611bd8466215 --settings Default --dur $SIMULATOR_DURATION

# Wait for any process to exit
wait -n

# Exit with status of process that exited first
exit $?