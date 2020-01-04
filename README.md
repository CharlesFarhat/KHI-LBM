# KHI-LBM

## Reqirement installation
In the order you will need to install :
* Qt (only x86_64 lib for cmake needed)
* ccmake (for paraview installation)
* Paraview
* OpenMPI
* OnpenLB

### Qt install :

Download community installer on their webstite :

```
https://www.qt.io/download-qt-installer
```
Install 5.9.9 (best compatibility) only (x86_64)
### ccmake install :

```
sudo apt-get install ccmake
```
### Paraview install :

Follow their instructions :

```
https://www.paraview.org/Wiki/ParaView:Build_And_Install
```

In the ccmake install config (use c to compile and g to generate) :
* change QT_DIR to /pathtoQT/gcc_64/lib/cmake

### openlb installation :

* Dependencies :

```
sudo apt-get install g++ openmpi-bin openmpi-doc libopenmpi-dev
```
* Download last version :
```
https://www.openlb.net/download/
```
* Go to root folder:


  * change config.mk to enable OMPI
  * Then : ``` make -j 15 ```

## Usage

Compile the cpp source code for simulation, then run it with command :

```
mpirun -N 1 ./bin/KHI-LBM
```