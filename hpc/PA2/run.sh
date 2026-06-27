. /home/spack/spack/share/spack/setup-env.sh

spack load cuda && spack load gcc@10.2.0

make

srun -N 1 --gres=gpu:1 ./benchmark $*