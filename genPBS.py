#!/usr/bin/env python
import os
import re
import sys
import copy
import ConfigParser
import itertools
import math
import shutil
from collections import defaultdict

slurm_cpus_per_task = 1
sims_subset_jump = 7
memory_overprovision = 1.25

def build_slurm(params_list, sim_list, args):
    if args.sim_per_pbs < 1:
        sys.exit("--sim-per-pbs has to be >=1.\n")
    
    print("Number of benchmarks to simulate: " + str(len(sim_list)) + "\n")
    print("Total simulations: " + str(len(params_list) * len(sim_list)) + "\n")

    all_sims = defaultdict(list)
    
    for p in params_list:
        for s in sim_list:
            all_sims[s.mem_size].append(cmd_string(p, s, True))

    keys = sorted(all_sims.keys())

    total_files = 0
    for key in keys:
        sims = all_sims[key]
        sims_subset_list = [ sims[i : i + sims_subset_jump] for i in range(0, len(sims), sims_subset_jump) ]
        total_files += len(sims_subset_list)
    
    num_digits = len(str(total_files))
    file_count = 0
    for key in keys:
        sims = all_sims[key]
        sims_subset_list = [ sims[i : i + sims_subset_jump] for i in range(0, len(sims), sims_subset_jump) ]
        subset_file_count = 0
        for subset in sims_subset_list:
            slurm_out_file(subset, key, args, file_count, num_digits, subset_file_count)
            subset_file_count += 1
            file_count += 1

        
def slurm_out_file(sims_subset, key, args, file_count, num_digits, subset_file_count):
    curr_out = args.pbs_prefix + str(file_count).zfill(num_digits)
    curr_out_file = curr_out + "._pbs"
    track_simulations = []
    f = open(curr_out_file, 'w')
    f.write(slurm_init_string(len(sims_subset), curr_out, key, subset_file_count))
    for s in sims_subset:
        if s.startswith("./"):
            track_simulations.append(s.split(" ")[3])
            f.write("srun -N1 --ntasks=1 " + s[2:] + "\n\n")
        else:
            f.write("srun -N1 --ntasks=1 " + s + "\n\n")
    f.write("sleep 1\n\n")
    f.write("wait\n")
    f.close()

    if args.copy_pbs == True:
        curr_out_file_path = os.getcwd() + "/" +  curr_out_file
        #print(curr_out_file_path)
        for sim_dir in track_simulations:
            #print("\t" + t)
            if not os.path.exists(sim_dir):
                os.makedirs(sim_dir)
                shutil.copy(curr_out_file_path, sim_dir)


def slurm_init_string(ntasks, curr_out, mem_per_cpu, subset_file_count):
    
    over_mem_per_cpu = int(float(memory_overprovision) * int(mem_per_cpu[:-2]))
    
    init = []
    
    init.append("#!/bin/bash\n")
    init.append("#SBATCH --ntasks=" + str(ntasks) + "\n")
    init.append("#SBATCH --job-name=" + curr_out + "-" + mem_per_cpu + "_" + str(subset_file_count) + "\n")
    init.append("#SBATCH --mem-per-cpu=" + str(over_mem_per_cpu) + "G" + "\n")
    init.append("#SBATCH --cpus-per-task=" + str(slurm_cpus_per_task) + "\n")
    init.append("#SBATCH -t 32:00:00\n")
    init.append("#SBATCH -p normal_q\n")
    init.append("#SBATCH -A server_mem\n")
    init.append("#########################################\n")
    init.append("\n\n")
    init.append("source /home/gpanwar/shrc\n")
    init.append("module load gcc/5.2.0\n")
    init.append("\n\n")

    init_str = ''.join(init)
    
    return init_str

######################################
# builds the PBS files
# typical PBS filter =
#        f.write(init_string())
#        f.write(cmd_string(p, s, bool))
#        f.write(end_string_single()) or f.write(end_string_wait())
def build_pbs(params_list, sim_list, args):

    if args.sim_per_pbs < 1:
        sys.exit("--sim-per-pbs has to be >=1.\n")

    #print("Length of params_list = " + str(len(params_list)) + "\n")
    print("Number of benchmarks to simulate: " + str(len(sim_list)) + "\n")
    print("Total simulations: " + str(len(params_list) * len(sim_list)) + "\n")

    max_files = len(params_list) * len(sim_list)
    num_digits = len(str(max_files))

    if args.sim_per_pbs is not None:
        max_files = int(math.ceil(float(len(params_list) * len(sim_list)) / args.sim_per_pbs))
        num_digits = len(str(max_files))
        print("Simulations per pbs: " + str(args.sim_per_pbs) + "\n")
        print("PBS files created: " + str(max_files) + "\n")

    file_count = 0
    sim_count = 0

    #sys.exit()

    curr_out_file = []
    temp_bench_list = []
    # track_simulations, for copying the pertaining PBS file to the simulation output dir
    track_simulations = []
    header_string = init_string(args.cluster)
    for p in params_list:
        for s in sim_list:
            if sim_count % args.sim_per_pbs == 0:
                curr_out_file = args.pbs_prefix + str(file_count).zfill(num_digits) + "._pbs"
                f = open(curr_out_file, 'w')
                f.write(header_string)
            
            # one gem5 simulation per PBS file, don't want '&' at the end
            if args.sim_per_pbs == 1:
                out_dir_path = str(p.get("output_dir")) + "/" + str(s.benchmark)
                track_simulations.append(out_dir_path)
                f.write(cmd_string(p, s, False))
            else:
                out_dir_path = str(p.get("output_dir")) + "/" + str(s.benchmark)
                track_simulations.append(out_dir_path)
                f.write(cmd_string(p, s, True))

            #end_string.append("echo \"Done " + benchStr + "\"\n")
            #temp_bench_list.append(str(s.benchmark) + "\n")

            temp_bench_list.append("echo \"" + str(s.benchmark) + "\"\n")
            
            sim_count += 1

            if sim_count % args.sim_per_pbs == 0:
                file_count += 1
                f.write(''.join(temp_bench_list))
                temp_bench_list = []

                if args.sim_per_pbs == 1:
                    f.write(end_string_single())
                else:
                    f.write(end_string_wait())
                f.close()
                
                # copy PBS to output folder
                if args.copy_pbs == True:
                    curr_out_file_path = os.getcwd() + "/" +  curr_out_file
                    #print(curr_out_file_path)
                    for sim_dir in track_simulations:
                        #print("\t" + t)
                        if not os.path.exists(sim_dir):
                            os.makedirs(sim_dir)
                        shutil.copy(curr_out_file_path, sim_dir)
                # clear track_simulations
                track_simulations = []
    
    # handle the last file for sims_per_pbs >= 2
    if temp_bench_list:
        f.write(''.join(temp_bench_list))
        if args.sim_per_pbs == 1:
            f.write(end_string_single())
        else:
            f.write(end_string_wait())
            f.close()

    if args.copy_pbs == True:
        curr_out_file_path = os.getcwd() + "/" +  curr_out_file
        #print(curr_out_file_path)
        for sim_dir in track_simulations:
            #print("\t" + t)
            if not os.path.exists(sim_dir):
                os.makedirs(sim_dir)
            shutil.copy(curr_out_file_path, sim_dir)
        
        

# reads "args.cluster".pbsinit file and uses it as the
# header for the PBS file
def init_string(cluster):
    if not os.path.isfile(cluster + ".pbsinit"):
        sys.exit(cluster + ".pbsinit does not exist.")

    with open(cluster + ".pbsinit") as f:
        pbs_init_str = f.read()

    pbs_init_str = pbs_init_str + "\n"
    #print(pbs_init_str)
    return pbs_init_str

# builds the gem5 command string
def cmd_string(params, sim, background):

    cmd = []
    
    _params = copy.deepcopy(params)

    cmd.append(_params.getP("binary_path") + " ")
    cmd.append("-r -d" + " ")
    #cmd.append(_params.get("output_dir") + "/" + str(sim.group) + "/" + str(sim.benchmark) + " ")
    cmd.append(_params.getP("output_dir") + "/" + str(sim.benchmark) + " ")
    cmd.append(_params.getP("config_path") + " ")

    if _params.get("mem-type") != False:
        if _params.get("mem-type") != '':
            cmd.append("--" + "mem-type=" + _params.getP("mem-type") + " ")
        else:
            _params.getP("mem-type")
        
    if _params.get("dramsim2-sys-conf") != False:
        if _params.get("dramsim2-sys-conf") != '':
            cmd.append("--" + "dramsim2-sys-conf=" + _params.getP("dramsim2-sys-conf") + " ")
        else:
            _params.getP("dramsim2-sys-conf")

    if _params.get("dramsim2-dev-conf") != False:
        if _params.get("dramsim2-dev-conf") != '':
            cmd.append("--" + "dramsim2-dev-conf=" + _params.getP("dramsim2-dev-conf") + " ")
        else:
            _params.getP("dramsim2-dev-conf")

    cmd.append("--mem-size=" + str(sim.mem_size) + " ")
    cmd.append("-n " + str(sim.cores) + " ")
    cmd.append("-r " + str(1) + " ")
    cmd.append("--disk-image=" + str(sim.disk) + " ")
    cmd.append("--checkpoint-dir=" + str(sim.checkpoint_dir) + str(sim.benchmark) + " ")

    if _params.get("cpu-type") != False:
        if _params.get("cpu-type") != '':
            cmd.append("--" + "cpu-type=" + _params.getP("cpu-type") + " ")
        else:
            _params.getP("cpu-type")
    
    if _params.get("atomic-warm-up") != False:
        if _params.get("atomic-warm-up") != '':
            cmd.append("--" + "atomic-warm-up=" + time_parser(_params.getP("atomic-warm-up")) + " ")
        else:
            _params.getP("atomic-warm-up")
            
    if _params.get("atomic-warm-up2") != False:
        if _params.get("atomic-warm-up2") != '':
            cmd.append("--" + "atomic-warm-up2=" + time_parser(_params.getP("atomic-warm-up2")) + " ")
        else:
            _params.getP("atomic-warm-up2")


    if _params.get("real-warm-up") != False:
        if _params.get("real-warm-up") != '':
            cmd.append("--" + "real-warm-up=" + time_parser(_params.getP("real-warm-up")) + " ")
        else:
            _params.getP("real-warm-up")

    if _params.get("rel-max-tick") != False:
        if _params.get("rel-max-tick") != '':
            cmd.append("--" + "rel-max-tick=" + time_parser(_params.getP("rel-max-tick")) + " ")
        else:
            _params.getP("rel-max-tick")

    
    remaining_keys = sorted(_params.params.keys())
    
    for key in remaining_keys:
        val = _params.get(key)
        if key != 'misc' and val != None:
            cmd.append("--" + key + "=" + _params.getP(key) + " ")
        elif key != 'misc' and val == None:
            cmd.append("--" + key + " ")
        elif key == 'misc':
            cmd.append(_params.getP(key) + " ")

    if background == True:
        cmd.append("&" + " ")

    cmd.append("\n\n")

    cmd_str = ''.join(cmd)
                          
    return cmd_str

# required when one PBS file contains multiple simulations that are
# dispatched to the background using &
def wait_string():
    wait = []

    wait.append("\n")
    wait.append("a=$(jobs | wc -l)")
    wait.append("\n")
    wait.append("while [ $a -gt 0 ]")
    wait.append("\n")
    wait.append("do")
    wait.append("\n")
    wait.append("\tsleep 1")
    wait.append("\n")
    wait.append("\ta=$(jobs | grep Running | wc -l)")
    wait.append("\n")
    #wait.append("\techo \"$a\"")
    # wait.append("\n")
    wait.append("done")
    wait.append("\n")

    wait_str = ''.join(wait)

    #print(wait_str)

    return wait_str

# pbs footer string if there is only one simulation
def end_string_single():
    end_string = []

    #end_string.append("echo \"Done " + benchStr + "\"\n")
    end_string.append("echo \"Job Ended at: `date`\"")
    end_string.append("\n")

    return ''.join(end_string)

# pbs footer string if there are multiple simulations
# includes the wait string
def end_string_wait():
    end_string = []

    end_string.append(str(wait_string()))
    end_string.append("echo \"Job Ended at: `date`\"")
    end_string.append("\n")

    return ''.join(end_string)

class Simulation:
    def __init__(self, _benchmark, _group, _disk, _checkpoint_dir, _cores, _mem_size):
        self.benchmark = _benchmark
        self.group = _group
        self.disk = _disk

        if _checkpoint_dir.endswith("/"):
            self.checkpoint_dir = _checkpoint_dir
        else:
            self.checkpoint_dir = _checkpoint_dir + "/"
            
        self.cores = _cores
        self.mem_size = _mem_size

    def info(self):
        print(str(self.benchmark) + " " + str(self.group) + " " +
              str(self.disk) + " " + str(self.checkpoint_dir) + " " +
              str(self.cores) + " " + str(self.mem_size))
    

# creates a list of benchmark/checkpoint specific parameter objects
def get_simulations(simFilePath):
    if not os.path.isfile(simFilePath):
        sys.exit(simFilePath + " does not exist.\n")
    
    sim_ini = ConfigParser.ConfigParser()
    sim_ini.read(simFilePath)

    sim_list = []
    
    #for section in sim.sections():
    #    print("Section = " + str(section))
    #    for (key, val) in sim.items(section):
    #        print key
    #        print val

    for section in sim_ini.sections():
        benchmark_list = sim_ini.get(section, 'benchmarks')
        benchmark_list = benchmark_list.split()

        _group = section
        _disk = sim_ini.get(section, 'disk')
        _checkpoint_dir = sim_ini.get(section, 'checkpoint_dir')
        _cores = sim_ini.get(section, 'cores')
        _mem_size = sim_ini.get(section, 'mem_size')
        
        for _benchmark in benchmark_list:
            sim_list.append(Simulation(_benchmark, _group, _disk, _checkpoint_dir, _cores, _mem_size))

    #for i in sim_list:
    #    i.info()

    return sim_list

def filter_simulations(sim_list, subsetFilePath):
    if not os.path.isfile(subsetFilePath):
        sys.exit(subsetFilePath + "does not exist.\n")

    
    subset_ini = ConfigParser.ConfigParser()
    subset_ini.read(subsetFilePath)

    if not subset_ini.get('run', 'run'):
        sys.exit("In " + subsetFilePath + ", make sure you have [run] with an item run.\n")
    
    subset_benchmarks = list(subset_ini.get('run', 'run').split())

    # check whether all subset_benchmarks are in sim_list
    # to do
    subset_sim_list = []
    for s in sim_list:
        if s.benchmark in subset_benchmarks:
            subset_sim_list.append(s)

    #for s in subset_sim_list:
    #    s.info()

    return subset_sim_list


# class for architectural specific parameters
class Params:
    def __init__(self):
        self.params = {}

    # set function
    def set(self, _attrib, _val):
        #if _attrib == "atomic-warm-up" or _attrib == "real-warm-up" or _attrib == "rel-max-tick":
        #    self.params[_attrib] = time_parser(_val)
        #else:
        self.params[_attrib] = _val
        #return True


    # get function
    def get(self, _attrib):
        if _attrib in self.params:
            return self.params[_attrib]
        else:
            return False
        #return self.params[_attrib]


    def getP(self, _attrib):
        if _attrib in self.params:
            ret = self.params.pop(_attrib)
            return ret
        else:
            return False

    # print info
    '''
    def info(self):
        print(str(self.binary_path) + " " + str(self.output_dir) + " " +
              str(self.config_path) + " " + str(self.cpu_type) + " " +
              str(self.mem_type) + " " + str(self.dramsim2_sys_conf) + " " +
              str(self.dramsim2_dev_conf) + " " + str(self.atomic_warm_up) + " " +
              str(self.real_warm_up) + " " + str(self.rel_max_tick) + " " +
              str(self.misc))
    '''


# create a list of architectural specific parameter objects
def get_params(paramsFilePath):
    if not os.path.isfile(paramsFilePath):
        sys.exit(paramsFilePath + " does not exist.\n")
        
    params_ini = ConfigParser.ConfigParser()
    params_ini.read(paramsFilePath)

    if not params_ini.has_section('static'):
        sys.exit("In " + paramsFilePath + ", [static] section not defined\n.")

    # list of params in [static] section. Used in every run_mode
    static_params_list = list(params_ini.items('static'))

    # run_mode = 1 for [dynamic] with/without [bottom]
    # run_mode = 2 for [bottom]
    # run_mode = 3 for no [dynamic] and no [bottom]
    # check whether items in [dynamic] section are specified in hierarchy item in [hierarchy]
    # check everything, whether both sections exist etc.
    if params_ini.has_section('dynamic') or params_ini.has_section('hierarchy'):
        run_mode = 1
        if not params_ini.has_section('hierarchy'):
            sys.exit("In " + paramsFilePath + ", [dynamic] specified without [hierarchy].\n")
        else:
            if not params_ini.get('hierarchy', 'hierarchy'):
                sys.exit("In " + paramsFilePath + ", [hierarchy] does not have item hierarchy.\n") 
        if not params_ini.has_section('dynamic'):
            sys.exit("In " + paramsFilePath + ", [hierarchy] specified without [dynamic].\n")

    if not params_ini.has_section('dynamic') and not params_ini.has_section('hierarchy'):
        if params_ini.has_section('bottom'):
            run_mode = 2
        else:
            run_mode = 3

    #print(run_mode)
            
       
    #dynamic_items = list(params_ini.items('dynamic'))
    if run_mode == 1:
        print("Found [dynamic] and [hierarchy]\n")
        dynamic_items = params_ini.options('dynamic')
        hierarchy_items = params_ini.get('hierarchy', 'hierarchy').split()

        if sorted(dynamic_items) != sorted(hierarchy_items):
            sys.exit("In " + paramsFilePath + ", not all options in [dynamic] are listed in hierarchy in [hierarchy]\n")



        # list of lists of dynamic_parameters
        dynamic_params_list = []
        for i in hierarchy_items:
            dynamic_params_list.append(list(params_ini.get('dynamic', i).split()))

        # create product permutation for the hierarchy.
        # example, [ [a, b, c], [d, e] ]
        # = [(a, d), (a, e), (b, d), (b, e), (c, d), (c, e)]
        # works with as many items in the hierarchy
        dynamic_params_list = list(itertools.product(*dynamic_params_list))
        

        params_obj_list = []
        for i in dynamic_params_list:
            # new object, all attribs assigned to None
            obj = Params()

            #print(hierarchy_items)
            #print(i)

            # assign all static attribs first using the set() function
            for j in static_params_list:
                #print(j)
                #obj.set(str(j[0]), str(j[1]))
                obj.set(str(j[0]), str(j[1]))
            
            #print(obj.params)
            #print("\n")

            # len(hierarchy_items) is equal to len(i). i is a tuple.
            # output_dir already set a static parameter. Will have to append
            # next folders for hierarchial output
            _output_dir = obj.get("output_dir")
            for j in xrange(len(i)):
                # set dynamic attribs using the set() function
                obj.set(hierarchy_items[j], i[j])

                # fix output_dir as per hierarchy.
                # Replace / with _ to avoid unnecessary directories.
                # eg, ini/DDR4_3200_tRFC550_nt.ini would unnecessarily
                # add another level of hierarchy because of 'ini/'
                # _output_dir = _output_dir + "/" + i[j].split("/")[-1]
                _output_dir = _output_dir + "/" + i[j].replace("/", "-")

            obj.set("output_dir", _output_dir) 
            
            #print(obj.params)
            #print("\n")

            params_obj_list.append(obj)

        print("Simulations after accounting for [dynamic] and [hierarchy]: " + str(len(params_obj_list)) + "\n")

        #for i in params_obj_list:
        #    print(i.params)

        if params_ini.has_section('bottom'):
            print("Found [bottom]\n")
            _params_obj_list = params_obj_list
            params_obj_list = []

            bottom_items = params_ini.options('bottom')
            bottom_params_list = []
            for i in bottom_items:
                bottom_params_list.append(params_ini.get('bottom', i))

            _split_bottom_params = []
            for i in bottom_params_list:
                _split_bottom_params.append(split_bottom_params(i))
            bottom_params_list = _split_bottom_params

            #print(bottom_params_list)

            arch_count = 0
            for i in bottom_params_list:
                for j in _params_obj_list:
                    obj = copy.deepcopy(j)
                    # i is for bottom params, could have something like "--caches" with no value argument
                    for k in i:
                        if len(k) == 2:
                            obj.set(k[0], k[1])
                        else:
                            obj.set(k[0], None)
                    _output_dir = obj.get("output_dir")
                    #print(bottom_items[arch_count])
                    _output_dir = _output_dir + "/" + bottom_items[arch_count]
                    obj.set("output_dir", _output_dir) 
                    params_obj_list.append(obj)
                    #obj.info()
                arch_count += 1
            
        #for i in params_obj_list:
        #    i.info()
        print("Simulations after accounting for [bottom]: " + str(len(params_obj_list)) + "\n")
    # end run_mode = 1

    if run_mode == 2:
        #print(run_mode)
        params_obj_list = []

        bottom_items = params_ini.options('bottom')
        bottom_params_list = []
        for i in bottom_items:
            bottom_params_list.append(params_ini.get('bottom', i))

        _split_bottom_params = []
        for i in bottom_params_list:
            _split_bottom_params.append(split_bottom_params(i))
        bottom_params_list = _split_bottom_params

        arch_count = 0
        #print(bottom_params_list)
        for i in bottom_params_list:
            obj = Params()

            # set static params for obj
            for j in static_params_list:
                #obj.set(str(j[0]), str(j[1]))
                obj.set(j[0], j[1])
                
            
            # i is for bottom params, could have something like "--caches" with no value argument
            for k in i:
                #obj.set(str(k[0]), str(k[1]))
                if len(k) == 2:
                    obj.set(k[0], k[1])
                else:
                    obj.set(k[0], None)


            _output_dir = obj.get("output_dir")
            _output_dir = _output_dir + "/" + bottom_items[arch_count]
            obj.set("output_dir", _output_dir)

            params_obj_list.append(obj)
            arch_count += 1

        print("Simulations after accounting for [bottom]: " + str(len(params_obj_list)) + "\n")
            
    if run_mode == 3:
        params_obj_list = []
        obj = Params()

        for j in static_params_list:
            #obj.set(str(j[0]), str(j[1]))
            if len(j) == 2:
                obj.set(j[0], j[1])
            else:
                obj.set(j[0], None)

        # only one obj in this case
        params_obj_list.append(obj)

        print("Only [static] defined.\n")


    return params_obj_list

def split_bottom_params(_str):
    _str = _str.split()
    l_str = []
    for s in _str:
        l_str.append(s.split("="))
        #remove starting --
        l_str[-1][0] = l_str[-1][0][2:]
        #print(_tup_str[-1][0])

    #print(l_str)
    return l_str
    
    
# takes in a string such as 20ns and returns an
# integer that represents simulation time in picoseconds
# useful for atomic_warm_up, real_warm_up and rel_max_tick
def time_parser(_str):
    _str = _str.lower()

    try:
        return str(int(_str))
    except ValueError:
        try:
            timeStr = _str[:-2]

            try:
                time = int(timeStr)
            except ValueError:
                try:
                    time = float(timeStr)
                except ValueError:
                    return "InValid"

            #print(time)
            unit = _str[-2:]

            if unit == "ms":
                time = time * (10**9)
            elif unit == "us":
                time = time * (10**6)
            elif unit == "ns":
                time = time * (10**3)
            elif unit == "ps":
                time = time * (10**0)

            #print(time)
            return str(int(time))
        except ValueError:
            return "Invalid"



                
if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    
    parser.add_argument("--misc", help="Misc argument")
    parser.add_argument("--params", required=True, help="Parameters.ini file. Contains static and dynamic parameters. Also, hierarchy of dynamic parameters")
    parser.add_argument("--benchmarks", required=True, help="Benchmarks.ini file. Contains benchmark families and checkpoints.")
    parser.add_argument("--run", default=None, help="Used to select which benchmarks to run. If not specified, all benchmarks will be run.")
    parser.add_argument("--sim-per-pbs", type=int, default=1, help="Simulations per PBS job file. If > 1, gem5 commands are released to background and wait loop is appended at the end of the PBS file.")
    parser.add_argument("--cluster", required=True, default="sushi", help="Cluster you are simulating on.")
    parser.add_argument("--pbs-prefix", type=str, default="gem5_", help="Prefix for the output PBS files. eg, gem5_")
    parser.add_argument("--copy-pbs", type=lambda x: (str(x).lower() == 'true'), default=True, help="Copy pbs file to the simulation folder.")

    args, unknown = parser.parse_known_args()

    if unknown:
        parser.print_help(sys.stderr)
        sys.exit("Unknown arguments specified. See usage above.\n")

        
    if not os.path.isfile(args.params):
        sys.exit(args.params + " does not exist.\n")
    if not os.path.isfile(args.benchmarks):
        sys.exit(args.benchmarks + " does not exist.\n")
    if args.cluster != "slurm":
        if not os.path.isfile(args.cluster + ".pbsinit"):
            sys.exit(args.cluster + ".pbsinit does not exist.\n")
    if args.run is not None:
        if not os.path.isfile(args.run):
            sys.exit(args.run + " does not exist.\n")


    params_list = get_params(args.params)
    sim_list = get_simulations(args.benchmarks)

    if args.run != None:
        sim_list = filter_simulations(sim_list, args.run)

    #print("\n\nsim_list\n\n")
    #for i in sim_list:
    #    i.info()


    #print("\n\nparams_list\n\n")
    #for i in params_list:
    #    i.info()


    #test = cmd_string(params_list[0], sim_list[1], False)

    #build_pbs(params_list, sim_list, args)

    #print(test)
    #build_pbs(args, sim_list)

    if args.cluster == "slurm":
        build_slurm(params_list, sim_list, args)
    else:
        build_pbs(params_list, sim_list, args)

    

    

    
