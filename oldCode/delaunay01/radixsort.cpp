#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cassert>
#include <cmath>
#include <unistd.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sys/time.h>
#include <time.h>
#include <limits> 

#include "radixsort.h"

//////////////////////////////////////////////////////////////////////////
void initialize_ocl(cl_vars_t& cv)
{
  cv.err = clGetPlatformIDs(1, &(cv.platform), &(cv.platforms));
  CHK_ERR(cv.err);

  cv.err = clGetDeviceIDs(cv.platform, CL_DEVICE_TYPE_GPU, 1, &(cv.device_id), NULL);
  CHK_ERR(cv.err);

  cv.context = clCreateContext(0, 1, &(cv.device_id), NULL, NULL, &(cv.err));
  CHK_ERR(cv.err);

  cv.commands = clCreateCommandQueue(cv.context, cv.device_id, 
				     CL_QUEUE_PROFILING_ENABLE, &(cv.err));
  CHK_ERR(cv.err);


#ifdef DEBUG
  std::cout << "CL fill vars success" << std::endl;

  // Device info
  cl_ulong mem_size;
  cv.err = clGetDeviceInfo(cv.device_id, CL_DEVICE_GLOBAL_MEM_SIZE, 
			   sizeof(cl_ulong), &mem_size, NULL);

  std::cout << "Global mem size: " << mem_size << std::endl;

  size_t max_work_item[3];
  cv.err = clGetDeviceInfo(cv.device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, 
			   sizeof(max_work_item), max_work_item, NULL);

  std::cout << "Max work item sizes: " 
	    << max_work_item[0] << ", " 
	    << max_work_item[1] << ", " 
	    << max_work_item[2] 
	    << std::endl;
#endif
}

//////////////////////////////////////////////////////////////////////////
void uninitialize_ocl(cl_vars_t & clv)
{
  cl_int err;
  err = clFlush(clv.commands);
  CHK_ERR(err);

  for(std::list<cl_kernel>::iterator it = clv.kernels.begin();
      it != clv.kernels.end(); it++)
    {
      err = clReleaseKernel(*it);
      CHK_ERR(err);
    }
  clv.kernels.clear();
    
  err = clReleaseProgram(clv.main_program);
  CHK_ERR(err);
    
  err = clReleaseCommandQueue(clv.commands);
  CHK_ERR(err);

  err = clReleaseContext(clv.context);
  CHK_ERR(err);
}

//////////////////////////////////////////////////////////////////////////
void ocl_device_query(cl_vars_t &cv)
{
  char buf[256];
  cl_int err;
  memset(buf,0,sizeof(buf));
  err = clGetDeviceInfo(cv.device_id,CL_DEVICE_NAME,
			sizeof(buf),(void*)buf,
			NULL);
  CHK_ERR(err);
  printf("Running on a %s\n", buf);
}

//////////////////////////////////////////////////////////////////////////
void compile_ocl_program(cl_kernel & kernel, cl_vars_t &cv,
			 const char * cl_src, const char * kname)
{
  cl_int err;
  cv.main_program = clCreateProgramWithSource(cv.context, 1, 
					      (const char **) &cl_src, NULL, &err);
  //std::cout << cv.main_program << std::endl;
  CHK_ERR(err);

  err = clBuildProgram(cv.main_program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS)
    {
      size_t len;
      char buffer[2048];
      std::cout << "Error: Failed to build program executable: " << kname <<  std::endl;
      clGetProgramBuildInfo(cv.main_program, cv.device_id, CL_PROGRAM_BUILD_LOG, 
			    sizeof(buffer), buffer, &len);
      std::cout << buffer << std::endl;
      exit(1);
    }
  
  kernel = clCreateKernel(cv.main_program, kname, &(err));
  if(!kernel || err != CL_SUCCESS)
  {
    std::cout << "Failed to create kernel: " << kname  << std::endl;
    exit(1);
  }
  cv.kernels.push_back(kernel);
#ifdef DEBUG
  std::cout << "Successfully compiled " << kname << std::endl;
#endif
}

//////////////////////////////////////////////////////////////////////////
void compile_ocl_program(std::map<std::string, cl_kernel> &kernels, 
			 cl_vars_t &cv, const char * cl_src, 
			 std::list<std::string> knames)
{
  cl_int err;
  cv.main_program = clCreateProgramWithSource(cv.context, 1, (const char **) &cl_src, 
					      NULL, &err);
  CHK_ERR(err);

  err = clBuildProgram(cv.main_program, 0, NULL, NULL, NULL, NULL);

  if (err != CL_SUCCESS)
    {
      size_t len;
      char buffer[2048];
      std::cout << "Error: Failed to build program executable " << std::endl;
      clGetProgramBuildInfo(cv.main_program, cv.device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
      std::cout << buffer << std::endl;
      exit(1);
    }
  
  for(std::list<std::string>::iterator it = knames.begin(); it != knames.end(); it++)
    {
      cl_kernel kernel = clCreateKernel(cv.main_program, (*it).c_str(), &(err));
      if(!kernel || err != CL_SUCCESS)
	{
	  std::cout << "Failed to create kernel: " << (*it).c_str()  << std::endl;
	  exit(1);
	}
#ifdef DEBUG
      std::cout << "Successfully compiled " << (*it).c_str() << std::endl;
#endif
      cv.kernels.push_back(kernel);
      kernels[*it] = kernel;
    }
}

//////////////////////////////////////////////////////////////////////////
void readFile(std::string& fileName, std::string &out)
{
  std::ifstream in(fileName.c_str(), std::ios::in | std::ios::binary);
  if(in)
    {
      in.seekg(0, std::ios::end);
      out.resize(in.tellg());
      in.seekg(0, std::ios::beg);
      in.read(&out[0], out.size());
      in.close();
    }
  else
    {
      std::cout << "Failed to open " << fileName << std::endl;
      exit(-1);
    }
}

//////////////////////////////////////////////////////////////////////////
double timestamp()
{
  struct timeval tv;
  gettimeofday (&tv, 0);
  return tv.tv_sec + 1e-6*tv.tv_usec;
}

//////////////////////////////////////////////////////////////////////////
void adjustWorkSize(size_t &global, size_t local)
{
  if(global % local != 0)
    {
      global = ((global/local) + 1) * local;  
    }
}

//////////////////////////////////////////////////////////////////////////
std::string reportOCLError(cl_int err)
{
  std::stringstream stream;
  switch (err) 
    {
    case CL_DEVICE_NOT_FOUND:          
      stream << "Device not found.";
      break;
    case CL_DEVICE_NOT_AVAILABLE:           
      stream << "Device not available";
      break;
    case CL_COMPILER_NOT_AVAILABLE:     
      stream << "Compiler not available";
      break;
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:   
      stream << "Memory object allocation failure";
      break;
    case CL_OUT_OF_RESOURCES:       
      stream << "Out of resources";
      break;
    case CL_OUT_OF_HOST_MEMORY:     
      stream << "Out of host memory";
      break;
    case CL_PROFILING_INFO_NOT_AVAILABLE:  
      stream << "Profiling information not available";
      break;
    case CL_MEM_COPY_OVERLAP:        
      stream << "Memory copy overlap";
      break;
    case CL_IMAGE_FORMAT_MISMATCH:   
      stream << "Image format mismatch";
      break;
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:         
      stream << "Image format not supported";    break;
    case CL_BUILD_PROGRAM_FAILURE:     
      stream << "Program build failure";    break;
    case CL_MAP_FAILURE:         
      stream << "Map failure";    break;
    case CL_INVALID_VALUE:
      stream << "Invalid value";    break;
    case CL_INVALID_DEVICE_TYPE:
      stream << "Invalid device type";    break;
    case CL_INVALID_PLATFORM:        
      stream << "Invalid platform";    break;
    case CL_INVALID_DEVICE:     
      stream << "Invalid device";    break;
    case CL_INVALID_CONTEXT:        
      stream << "Invalid context";    break;
    case CL_INVALID_QUEUE_PROPERTIES: 
      stream << "Invalid queue properties";    break;
    case CL_INVALID_COMMAND_QUEUE:          
      stream << "Invalid command queue";    break;
    case CL_INVALID_HOST_PTR:            
      stream << "Invalid host pointer";    break;
    case CL_INVALID_MEM_OBJECT:              
      stream << "Invalid memory object";    break;
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  
      stream << "Invalid image format descriptor";    break;
    case CL_INVALID_IMAGE_SIZE:           
      stream << "Invalid image size";    break;
    case CL_INVALID_SAMPLER:     
      stream << "Invalid sampler";    break;
    case CL_INVALID_BINARY:                    
      stream << "Invalid binary";    break;
    case CL_INVALID_BUILD_OPTIONS:           
      stream << "Invalid build options";    break;
    case CL_INVALID_PROGRAM:               
      stream << "Invalid program";
    case CL_INVALID_PROGRAM_EXECUTABLE:  
      stream << "Invalid program executable";    break;
    case CL_INVALID_KERNEL_NAME:         
      stream << "Invalid kernel name";    break;
    case CL_INVALID_KERNEL_DEFINITION:      
      stream << "Invalid kernel definition";    break;
    case CL_INVALID_KERNEL:               
      stream << "Invalid kernel";    break;
    case CL_INVALID_ARG_INDEX:           
      stream << "Invalid argument index";    break;
    case CL_INVALID_ARG_VALUE:               
      stream << "Invalid argument value";    break;
    case CL_INVALID_ARG_SIZE:              
      stream << "Invalid argument size";    break;
    case CL_INVALID_KERNEL_ARGS:           
      stream << "Invalid kernel arguments";    break;
    case CL_INVALID_WORK_DIMENSION:       
      stream << "Invalid work dimension";    break;
      break;
    case CL_INVALID_WORK_GROUP_SIZE:          
      stream << "Invalid work group size";    break;
      break;
    case CL_INVALID_WORK_ITEM_SIZE:      
      stream << "Invalid work item size";    break;
      break;
    case CL_INVALID_GLOBAL_OFFSET: 
      stream << "Invalid global offset";    break;
      break;
    case CL_INVALID_EVENT_WAIT_LIST: 
      stream << "Invalid event wait list";    break;
      break;
    case CL_INVALID_EVENT:                
      stream << "Invalid event";    break;
      break;
    case CL_INVALID_OPERATION:       
      stream << "Invalid operation";    break;
      break;
    case CL_INVALID_GL_OBJECT:              
      stream << "Invalid OpenGL object";    break;
      break;
    case CL_INVALID_BUFFER_SIZE:          
      stream << "Invalid buffer size";    break;
      break;
    case CL_INVALID_MIP_LEVEL:             
      stream << "Invalid mip-map level";   
      break;  
    default: 
      stream << "Unknown";
      break;
    }
  return stream.str();
 }

//////////////////////////////////////////////////////////////////////////
int int_compar(const void *x, const void *y)
{
  int i_x = *((int*)x);
  int i_y = *((int*)y);
  return i_x > i_y;
}

//////////////////////////////////////////////////////////////////////////
void rsort_scan(cl_command_queue &queue,
		    cl_context &context,
		    cl_kernel &scan_kern,
		    cl_kernel &update_kern,
		    cl_mem &in, 
		    cl_mem &out, 
		    int v,
		    int k,
		    int len);

//////////////////////////////////////////////////////////////////////////
void cpu_rscan(int *in, int *out, int v, int k, int n)
{
  int t = (in[0] >> k) & 0x1;
  out[0] = (t==v);
  for(int i = 1; i < n; i++)
    {
      t = (in[i] >> k) & 0x1;
      out[i] = out[i-1]+(t==v);
    }
}

//////////////////////////////////////////////////////////////////////////
unsigned int adjustSize(unsigned int N){
	int k = log2(N);
	if(pow(2, k)<N) return pow(2, k+1);
	else return N;
}

//////////////////////////////////////////////////////////////////////////
void sort_array_GPU(unsigned int *intArr, unsigned int intArrSize)
{
  std::string kernel_source_str;
  
  std::string arraycompact_kernel_file = 
    std::string("radixsort.cl");
  
  std::list<std::string> kernel_names;
  std::string scan_name_str = std::string("scan");
  std::string update_name_str = std::string("update");
  std::string reassemble_name_str = std::string("reassemble");

  kernel_names.push_back(scan_name_str);
  kernel_names.push_back(update_name_str);
  kernel_names.push_back(reassemble_name_str);

  cl_vars_t cv; 
  
  std::map<std::string, cl_kernel> 
    kernel_map;

  int c;
//  int n = (1<<20);
//  int n = 16384;
unsigned int n = adjustSize(intArrSize);
unsigned int *in , *out;
unsigned int *c_scan;
unsigned int n_out=-1;
  bool silent = false;

/*
  while((c = getopt(argc, argv, "n:s:"))!=-1)
    {
      switch(c)
	{
	case 'n':
	  n = 1 << atoi(optarg);
	  break;
	case 's':
	  silent = atoi(optarg) == 1;
	  break;
	}
    }
*/
  in = new unsigned int[n];
  out = new unsigned int[n];
  c_scan = new unsigned int[n];

for(unsigned int i=0; i<intArrSize; i++) in[i] = intArr[i];
if(n>intArrSize){// size has been changed
	unsigned int Max = std::numeric_limits<unsigned int>::max();
	for(unsigned int i=intArrSize; i<n; i++) in[i] = Max;
}
 
  bzero(out, sizeof(unsigned int)*n);
  bzero(c_scan, sizeof(unsigned int)*n);

  readFile(arraycompact_kernel_file,
	   kernel_source_str);
  
  initialize_ocl(cv);

  compile_ocl_program(kernel_map, cv, 
		      kernel_source_str.c_str(),
		      kernel_names);
  
  cl_mem g_in, g_zeros, g_ones, g_out;
  cl_mem g_temp;
  
  cl_int err = CL_SUCCESS;
  g_in = clCreateBuffer(cv.context,CL_MEM_READ_WRITE,
		       sizeof(unsigned int)*n,NULL,&err);
  CHK_ERR(err);  

  g_ones = clCreateBuffer(cv.context,CL_MEM_READ_WRITE,
			  sizeof(unsigned int)*n,NULL,&err);
  CHK_ERR(err);

  g_zeros = clCreateBuffer(cv.context,CL_MEM_READ_WRITE,
			  sizeof(unsigned int)*n,NULL,&err);
  CHK_ERR(err);
  
  g_out = clCreateBuffer(cv.context,CL_MEM_READ_WRITE,
			 sizeof(unsigned int)*n,NULL,&err);
  CHK_ERR(err);
  
  //copy data from host CPU to GPU
  err = clEnqueueWriteBuffer(cv.commands, g_in, true, 0, sizeof(unsigned int)*n,
			     in, 0, NULL, NULL);
  CHK_ERR(err);

  err = clEnqueueWriteBuffer(cv.commands, g_out, true, 0, sizeof(unsigned int)*n,
			     c_scan, 0, NULL, NULL);
  CHK_ERR(err);
 
  size_t global_work_size[1] = {n};
  size_t local_work_size[1] = {128};


  adjustWorkSize(global_work_size[0], local_work_size[0]);
  global_work_size[0] = std::max(local_work_size[0], global_work_size[0]);
  int left_over = 0;

  double t0 = timestamp();

	///   Radix sort implementation  ///
	for(int k=0; k< 32; k++)
	{		
		//fill the zeros array with indexes where the LSB is zero.
		rsort_scan(cv.commands,
			cv.context,
			kernel_map[scan_name_str],
			kernel_map[update_name_str],
			g_in, 
			g_zeros, 
			0,
			k,
			n);
		
		//fill the ones array with indexes where the LSB is one.	
		rsort_scan(cv.commands,
			cv.context,
			kernel_map[scan_name_str],
			kernel_map[update_name_str],
			g_in, 
			g_ones, 
			1,
			k,
			n);
	
		//set Kernel Arguments
		err = clSetKernelArg(kernel_map[reassemble_name_str], 0, sizeof(cl_mem), &g_in);
		CHK_ERR(err);
		err = clSetKernelArg(kernel_map[reassemble_name_str], 1, sizeof(cl_mem), &g_out);
		CHK_ERR(err);
		err = clSetKernelArg(kernel_map[reassemble_name_str], 2, sizeof(cl_mem), &g_zeros);
		CHK_ERR(err);
		err = clSetKernelArg(kernel_map[reassemble_name_str], 3, sizeof(cl_mem), &g_ones);
		CHK_ERR(err);
		err = clSetKernelArg(kernel_map[reassemble_name_str], 4, sizeof(int), &k);
		CHK_ERR(err);
		err = clSetKernelArg(kernel_map[reassemble_name_str], 5, sizeof(unsigned int), &n);
		CHK_ERR(err);
	
		//calls the scatter kernel on the GPU, which scatters the zeros' and ones' to the correct positions in the output array.
		err = clEnqueueNDRangeKernel(cv.commands,
			       kernel_map[reassemble_name_str],
			       1,//work_dim,
			       NULL, //global_work_offset
			       global_work_size, //global_work_size
			       local_work_size, //local_work_size
			       0, //num_events_in_wait_list
			       NULL, //event_wait_list
			       NULL //
			       );
		CHK_ERR(err);
		
		//need to swap the input and output arrays to get ready for the next iteration.
		g_temp = g_in;
		g_in = g_out;
		g_out = g_temp;

  }
  
  t0 = timestamp() - t0;

  /* Sort array on CPU for comparison */
  double t1 = timestamp();
//  qsort(in, n, sizeof(int), int_compar);
  t1 = timestamp() - t1;

  err = clEnqueueReadBuffer(cv.commands, g_in, true, 0, sizeof(unsigned int)*n,			    out, 0, NULL, NULL);
//  err = clEnqueueReadBuffer(cv.commands, g_in, true, 0, sizeof(unsigned int)*n,			    intArr, 0, NULL, NULL);

  CHK_ERR(err);

	for(unsigned int i = 0; i < intArrSize; i++){
		intArr[i] = out[i];
//		std::cout<<intArr[i]<<" ";
	}
//	std::cout<<std::endl;


/*
 for(int i = 0; i < n; i++)
    {
      if(in[i] != out[i])
	{
	  if(!silent)
	    printf("not sorted @ %d: %d vs %d!\n", i, in[i], out[i]);
	  goto done;
	}
    }
 if(!silent)
   printf("array sorted\n");

 if(silent)
   {
     printf("%d,%g,%g\n",n,t1,t0);
   }
 else
   {
     printf("GPU: array of length %d sorted in %g seconds\n", n, t0);
     printf("CPU: array of length %d sorted in %g seconds\n", n, t1);
   }
*/

 done:

  clReleaseMemObject(g_in); 
  clReleaseMemObject(g_out);
  clReleaseMemObject(g_ones);
  clReleaseMemObject(g_zeros);
  
  uninitialize_ocl(cv);
  delete [] in;
  delete [] out;
  delete [] c_scan;
}

//////////////////////////////////////////////////////////////////////////
void rsort_scan(cl_command_queue &queue,
		cl_context &context,
		cl_kernel &scan_kern,
		cl_kernel &update_kern,
		cl_mem &in, 
		cl_mem &out, 
		int v,
		int k,
		int len)
{
  size_t global_work_size[1] = {len};
  size_t local_work_size[1] = {128};
  int left_over = 0;
  cl_int err;
  
  adjustWorkSize(global_work_size[0], local_work_size[0]);
  global_work_size[0] = std::max(local_work_size[0], global_work_size[0]);

  left_over = global_work_size[0] / local_work_size[0];
  
  cl_mem g_bscan = clCreateBuffer(context,CL_MEM_READ_WRITE, 
				  sizeof(int)*left_over,NULL,&err);
  CHK_ERR(err);

  err = clSetKernelArg(scan_kern, 0, sizeof(cl_mem), &in);
  CHK_ERR(err);

  err = clSetKernelArg(scan_kern, 1, sizeof(cl_mem), &out);
  CHK_ERR(err);

  /* CS194: Per work-group partial scan output */
  err = clSetKernelArg(scan_kern, 2, sizeof(cl_mem), &g_bscan);
  CHK_ERR(err);

  /* CS194: number of bytes for dynamically 
   * sized local (private memory) "buf"*/
  err = clSetKernelArg(scan_kern, 3, 2*local_work_size[0]*sizeof(cl_int), NULL);
  CHK_ERR(err);

  /* CS194: v will be either 0 or 1 in order to perform
   * a scan of bits set (or unset) */
  err = clSetKernelArg(scan_kern, 4, sizeof(int), &v);
  CHK_ERR(err);

  /* CS194: the current bit position (0 to 31) that
   * we want to operate on */
  err = clSetKernelArg(scan_kern, 5, sizeof(int), &k);
  CHK_ERR(err);

  err = clSetKernelArg(scan_kern, 6, sizeof(int), &len);
  CHK_ERR(err);

  err = clEnqueueNDRangeKernel(queue,
			       scan_kern,
			       1,//work_dim,
			       NULL, //global_work_offset
			       global_work_size, //global_work_size
			       local_work_size, //local_work_size
			       0, //num_events_in_wait_list
			       NULL, //event_wait_list
			       NULL //
			       );
  CHK_ERR(err);

  if(left_over > 1)
    {
      cl_mem g_bbscan = clCreateBuffer(context,CL_MEM_READ_WRITE, 
				      sizeof(int)*left_over,NULL,&err);

      /* Recursively perform scan if needed */
      rsort_scan(queue,context,scan_kern,update_kern,g_bscan,
		     g_bbscan,-1,k,left_over);

      err = clSetKernelArg(update_kern,0,
			   sizeof(cl_mem), &out);
      CHK_ERR(err);
      
      err = clSetKernelArg(update_kern,1,
			   sizeof(cl_mem), &g_bbscan);
      CHK_ERR(err);

      err = clSetKernelArg(update_kern,2,
			   sizeof(int), &len);
      CHK_ERR(err);
      
      /* Update partial scans */
      err = clEnqueueNDRangeKernel(queue,
				   update_kern,
				   1,//work_dim,
				   NULL, //global_work_offset
				   global_work_size, //global_work_size
				   local_work_size, //local_work_size
				   0, //num_events_in_wait_list
				   NULL, //event_wait_list
				   NULL //
				   );
      CHK_ERR(err);
      
      clReleaseMemObject(g_bbscan);
    }

  clReleaseMemObject(g_bscan);
}

//////////////////////////////////////////////////////////////////////////
/*unique function run on OpenCL after sorting
int dev_array is a sorted one, output is dev_temp_array
for ex: intArr:	   1 1 1 2 2 2 3 3 4 4 4 5 5 6 6 6 7 8 9 9 9
		dev_out:   0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
after running unique function, 
		dev_out:   0 0 1 0 0 2 0 3 0 0 4 0 5 0 0 6 7 8 0 0 9 
input: intArr, intArrSize;
output: uniqueArr, uniqueArrSize,
		uniqueArr: 1 2 3 4 5 6 7 8 9
*/
void uniqueList_GPU(unsigned int *intArr, unsigned int intArrSize, unsigned int *&uniqueArr, unsigned int *uniqueArrSize)
{
	std::string kernel_source_str; 
	std::string kernel_file = std::string("unique.cl");
	std::string kernel_name_str = std::string("unique");

	cl_vars_t cv;  
	unsigned int size= intArrSize;
	unsigned int *dev_out = new unsigned int[intArrSize];
	for(unsigned int i=0; i<intArrSize; i++) dev_out[i]=0;
	
	
	readFile(kernel_file, kernel_source_str);  
	initialize_ocl(cv);
	cl_kernel kernel;
	compile_ocl_program(kernel, cv, 
		      kernel_source_str.c_str(),
		      kernel_name_str.c_str());
  
  cl_mem g_in, g_out;
  
  cl_int err = CL_SUCCESS;
  //create device buffer
  g_in = clCreateBuffer(cv.context,CL_MEM_READ_WRITE,
		       sizeof(unsigned int)*size,NULL,&err);
  CHK_ERR(err);  
 
  g_out = clCreateBuffer(cv.context,CL_MEM_READ_WRITE,
			 sizeof(unsigned int)*size,NULL,&err);
  CHK_ERR(err);
  
  //copy data from host CPU to GPU
  err = clEnqueueWriteBuffer(cv.commands, g_in, true, 0, sizeof(unsigned int)*size,
			     intArr, 0, NULL, NULL);
  CHK_ERR(err);

  err = clEnqueueWriteBuffer(cv.commands, g_out, true, 0, sizeof(unsigned int)*size,
			     dev_out, 0, NULL, NULL);
  CHK_ERR(err);
 
  size_t global_work_size[1] = {intArrSize};
  size_t local_work_size[1] = {128};

  adjustWorkSize(global_work_size[0], local_work_size[0]);
  global_work_size[0] = std::max(local_work_size[0], global_work_size[0]);
  int left_over = 0;

	//set Kernel Arguments
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &g_in);
	CHK_ERR(err);

	err = clSetKernelArg(kernel, 1, sizeof(unsigned int), &size);
	CHK_ERR(err);

	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &g_out);
	CHK_ERR(err);

	//calls the scatter kernel on the GPU, which scatters the zeros' and ones' to the correct positions in the output array.
	err = clEnqueueNDRangeKernel(cv.commands,
			       kernel,
			       1,//work_dim,
			       NULL, //global_work_offset
			       global_work_size, //global_work_size
			       local_work_size, //local_work_size
			       0, //num_events_in_wait_list
			       NULL, //event_wait_list
			       NULL //
			       );
	CHK_ERR(err);
		

	err = clEnqueueReadBuffer(cv.commands, g_out, true, 0, sizeof(unsigned int)*size, dev_out, 0, NULL, NULL);
	CHK_ERR(err);

	//process output uniqueArr, 
	unsigned int tempSize = 0;
	for(unsigned int i = 0; i < intArrSize; i++) 
		if(dev_out[i]!=0) tempSize++;
	if(intArr[0]==0) tempSize++;
	
	uniqueArr = new unsigned int[tempSize];
	unsigned int index = 0, newIndex = 0;
	if(intArr[0]==0) {uniqueArr[0] = 0; newIndex++;}
	while(index<intArrSize){
		if(dev_out[index]!=0){
			uniqueArr[newIndex] = dev_out[index];
			newIndex++;
		}
		index++;
	}
	*uniqueArrSize = tempSize;

  clReleaseMemObject(g_in); 
  clReleaseMemObject(g_out);
  
  uninitialize_ocl(cv);
  delete [] dev_out;
}

//////////////////////////////////////////////////////////////////////////
/*unique function run on OpenCL after sorting
int dev_array is a sorted one, output is dev_temp_array
for ex: intArr:	   1 1 1 2 2 2 3 3 4 4 4 5 5 6 6 6 7 8 9 9 9
		uniqueArr: 1 2 3 4 5 6 7 8 9
input: intArr, intArrSize;
output: uniqueArr, uniqueArrSize,
*/
void uniqueList_CPU(unsigned int *intArr, unsigned int intArrSize, unsigned int *&uniqueArr, unsigned int *uniqueArrSize){

	unsigned int tempSize = 1;
	for(unsigned int i=0; i<intArrSize-1; i++)
		if(intArr[i]!=intArr[i+1]) tempSize++;

	unsigned int index = 0;
	uniqueArr = new unsigned int[tempSize];
	for(unsigned int i=0; i<intArrSize-1; i++)
		if(intArr[i]!=intArr[i+1]){
			uniqueArr[index] = intArr[i];
			index++;
		}
	uniqueArr[index] = intArr[intArrSize-1];
	*uniqueArrSize = tempSize;
}

void uniqueList(unsigned int *intArr, unsigned int intArrSize, unsigned int *&uniqueArr, unsigned int *uniqueArrSize){
unsigned int *intArrTemp = new unsigned int[intArrSize];
	for(unsigned int i=0; i<intArrSize; i++) intArrTemp[i] = intArr[i];
	
	if(intArrSize<=32768*128){
		qsort(intArrTemp, intArrSize, sizeof(unsigned int), int_compar);
		uniqueList_CPU(intArrTemp, intArrSize, uniqueArr, uniqueArrSize);
	}
	else{
		sort_array_GPU(intArrTemp, intArrSize);
		uniqueList_GPU(intArrTemp, intArrSize, uniqueArr, uniqueArrSize);
	}
delete [] intArrTemp;
}

void sort(unsigned int *intArr, unsigned int intArrSize){
	if(intArrSize<=2097152)
		qsort(intArr, intArrSize, sizeof(unsigned int), int_compar);
	else
		sort_array_GPU(intArr, intArrSize);
}


//////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){
//	unsigned int intArrSize=32768;
//	unsigned int *intArr = new unsigned[intArrSize];

unsigned int intArrSize=129;
	unsigned int intArr[] = {8,6,4,7,2,4,6,7,9,8,4,2,1,5,4,6,7,6,8,9,12,13,5,6,4,7,4,17,45,23,12,8,2,6,4,7,2,4,6,7,9,8,4,2,1,5,4,6,7,6,8,9,12,13,5,6,4,7,4,17,45,23,12,8,2, 4,7,2,4,6,7,9,8,4,2,1,5,4,6,7,6,8,9,12,13,5,6,4,7,4,17,45,23,12,8,2,6,4,7,2,4,6,7,9,8,4,2,1,5,4,6,7,6,8,9,12,13,5,6,4,7,4,17,45,23,12,8,2,0};

//	srand(5);
//	for(unsigned int i = 0; i < intArrSize; i++) intArr[i] = rand();
	unsigned int *uniqueArr;
	unsigned int uniqueArrSize;

//	if(intArrSize<=32768){
//		qsort(intArr, intArrSize, sizeof(unsigned int), int_compar);
//		uniqueList_CPU(intArr, intArrSize, uniqueArr, &uniqueArrSize);
//	}
//	else{
		sort_array_GPU(intArr, intArrSize);
		uniqueList_GPU(intArr, intArrSize, uniqueArr, &uniqueArrSize);
//	}

//	for(unsigned int i = 0; i<intArrSize; i++) std::cout<<intArr[i]<<" ";
//	std::cout<<std::endl;

	for(unsigned int i = 0; i<uniqueArrSize; i++) std::cout<<uniqueArr[i]<<" ";
	std::cout<<std::endl;
	delete [] uniqueArr;
//	delete [] intArr;
	return 0;
}

