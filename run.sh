for i in *._pbs; do
qsub "$i"
done
