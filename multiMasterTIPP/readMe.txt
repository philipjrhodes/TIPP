Abstract
	multiMasterTIPP is a parallel triangulation based on independent partitions in two-level parallelism. See our paper "Delaunay Triangulation of Large-Scale Datasets Using Two-Level Parallelism" for more detail. We use C/C++ and MPI (Message Passing Interface) to implementand evaluate our algorithm on a cluster.

Dependencies
	TIPP requires an MPI installation (MPICH) on Linux system.

Description
- MultiMasterTIPP with two-level parallelism is working with several options.
- Option 1 is uniform data or nonUniform
- Option 2 is sharedFolder od nonSharedFolder

- uniform: data points are equally distributed over the domain. Point data is generated by rbox function from qhull which is embed to TIPP sytem. 
- nonUniform dataset is also generated by qhull, based on North Carolina ocean map.

- sharedFolder: a shareFolder on the master node which contains point dataset. This folder should be shared to all other processes. It means that other processes can read/write result directly to this folder. To avoid the contention from writing triangulation results, we can use Ceph Parallel Storage System.
- With nonShareFolder option, the data folder doesn't need to be shared to all nodes because the master propcess is responsible to read all data and send to worker processes or collect the triangulation results from worker processes and write the results locally.
- Both sharedFolder and nonSharedFolder are currently used for uniform dataset only.


Download, build, install, and run
- You can clone this repository or download the source code. From the top-level directory (multiMasterTIPP folder), you can build the execute code by using "make clean", then "make".
- Using shell script code (testUniform.sh, testNonUniform.sh, and testNonUniformARP_DPA.sh) for experiments.
	+ testUniform.sh is used for testing uniform data 
	+ testNonUniform.sh is used for testing non-uniform data without improvements
	+ testNonUniformARP_DPA.sh is used for testing non-uniform data with one or two improvements (Apdaptive Resolution Partitioning (ARP) and Dynamic Process Assigning (DPA))
- The node IP addresses are declatred in machinefile file.
	


