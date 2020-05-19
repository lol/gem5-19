#!/bin/bash
start_id=$1
end_id=$2
i=$start_id
while [ $i -le $end_id ]; do
	qdel $i
	i=$((i+1))
done
