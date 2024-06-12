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


Call im_app using the following command line structue:

./im-app <kb_file> <measure> <method> <format> <cenc> <debug> <m>


=====List of existing <method> and <measure> combinations in im_app=====

<method> = sat (binary search for correct inconsistency value between 0 and upper bound)
possible <measure> values: contension, hs, hitdalal, maxdalal, sumdalal, forget

<method> = linsat (linear search from value 0 to upper bound)
possible <measure> values: contension, hs, hitdalal, maxdalal, sumdalal, forget

<method> = asp (encoding into answer set program)
possible <measure> values: contension, hs, hitdalal, maxdalal, sumdalal, forget, contension-ltl, 
			   drastic-ltl, mv, mv2, v3b, mv3, mv-mss, mv-mss2, p, p-2

<method> = maxsat
possible <measure> values: contension

<method> = naivecpp
possible <measure> values: forget 
