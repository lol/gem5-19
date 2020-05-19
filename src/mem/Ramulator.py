# -*- mode:python -*-
from m5.params import *
from AbstractMemory import *

# A wrapper for Ramulator multi-channel memory controller
class Ramulator(AbstractMemory):
    type = 'Ramulator'
    cxx_header = "mem/ramulator.hh"

    # A single port for now
    port = SlavePort("Slave port")

    config_file = Param.String("", "configuration file")
    num_cpus = Param.Unsigned(1, "Number of cpu")
    
    # gagan
    real_warm_up = Param.UInt64(100, "specify the real warm up time")
    output_dir = Param.String("", "Ramulator trace output")
