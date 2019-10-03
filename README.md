# CUDA-URB


## Package Requirements

On a general Linux system, such as Ubuntu 18.04 which we commonly use, you will need the following packages installed:
* libgdal-dev
* libnetcdf-c++4-dev
* libnetcdf-cxx-legacy-dev
* libnetcdf-dev
* netcdf-bin
* libboost-all-dev
* cmake
* cmake-curses-gui 

To use the GPU system (and even build the code) you will need a NVIDIA
GPU with the CUDA library installed.  We have tested with CUDA 8.0, 10.0, and 10.1.
If your version of CUDA is installed in a non-uniform location, you
will need to remember the path to the cuda install directory.

## Building the Code

The most active development occurs in the *workingBranch*. We suggest you use that branch at this time.  You can checkout this branch with the following git command:

``` git checkout workingBranch ```

If you are unsure about which branch you are on, the ``` git status ``` command can provide you with this information.




### Building on general Linux system

We separate the build 
```
mkdir build
cd build
cmake ..
```

You can then build the source:

```
make
```


## Instructions for CHPC Cluster

Our code does run on the CHPC cluster.  We mostly have tested with notchpeak nodes.

After logging into your CHPC account, you will need to load specific modules. The following module commands will take care of these requirements:

```
module load cuda/8.0
module load gcc/5.4.0
module load cmake/3.11.2 
module load gdal/2.3.1
module load boost/1.66.0
ml netcdf-cxx
```

```
module list

Currently Loaded Modules:
  1) chpc/1.0   2) cuda/8.0 (g)   3) gcc/5.4.0   4) cmake/3.11.2   5) gdal/2.3.1   6) hdf5/1.8.17   7) netcdf-c/4.4.1   8) netcdf-cxx/4.3.0

  Where:
   g:  built for GPU
```

```
cmake -DCUDA_TOOLKIT_DIR=/usr/local/cuda-8.0 -DCUDA_SDK_ROOT_DIR=/usr/local/cuda-8.0 -DCMAKE_PREFIX_PATH=/uufs/chpc.utah.edu/sys/installdir/gdal/2.1.3-c7 -DNETCDF_DIR=/uufs/chpc.utah.edu/sys/installdir/netcdf-c/4.4.1-c7/include -DNETCDF_CXX_DIR=/uufs/chpc.utah.edu/sys/installdir/netcdf-cxx/4.3.0-5.4.0g/include ..
```

To run the cudaUrb executable on notchpeak


```
#!/bin/bash
#SBATCH --account=efd-np
#SBATCH --partition=efd-shared-np
#SBATCH --job-name=moser395
#SBATCH --nodes=1
#SBATCH --mem=15G
#SBATCH --gres=gpu:titanv:1
#SBATCH --time=01:00:00
#SBATCH -e init_error.log
#SBATCH -o init_out.log
module load gcc/5.4.0
ulimit -c unlimited -s
./cudaUrb/cudaUrb -q ../data/QU_Files/GaussianHill.xml -s 2 -o gaussianHill.nc
```


```
sbatch runurb_GPU.sh
```

## Tips and Tricks

In case things don't go as planned with these instructions, here are some tips for correcting some build or run issues:


## Building the Documentation via Doxygen

After the build is configured the Doxygen documentation can be built. The output from this process is the updating of the _html_ and _latex_ folders in the top-level _docs_ folders.

```
make doc
```


### Continuous Integration

We are running continuous integration on Travis-CI.

[Basic Concepts for Travis Continuous Integration](https://docs.travis-ci.com/user/for-beginners/)


