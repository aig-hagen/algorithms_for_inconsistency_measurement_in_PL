# InconsistencyMeasureAlgorithms
### Setup
* The repository contains a submodule, use the following command to clone the parent repository and initialize the submodule in one step:
```git clone --recursive https://github.com/anachronismus/InconsistencyMeasureAlgorithms.git``` <br> Alternatively, you can clone this repository normally and run 
```git submodule update --init```
* To pull from both the repository and the submodule, use ```git pull --recurse-submodules```
* Build the project
<br> ```cmake CMakeLists.txt```
<br> ```make```


### To generate a Makefile using cmake the packages bison and re2c need
to be installed, otherwise the clingo library cannot be compiled