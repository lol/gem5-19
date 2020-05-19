for i in omp*._pbs; do
qsub "$i"
done
