#!/bin/bash
docker build -t registry.onwarddb.ca/vanguard/daemon:$1 .
docker push registry.onwarddb.ca/vanguard/daemon:$1
