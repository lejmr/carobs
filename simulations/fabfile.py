from __future__ import with_statement
from fabric.api import *
from fabric.contrib.console import confirm
import os 
import tempfile

env.hosts = ['kozakmil@briaree1.rqchp.qc.ca',]
env.hosts = ['milos@localhost',]


ROOT="~/simulations/carobs"

"""
    This function creates tar file in order to simplify sending process
    of the simulation onto cluster frontend
"""
def drivers(simulation):
    if os.path.exists("%s.tar.bz2"%simulation):
        print 'Removing old simulation file', "%s.tar.bz2"%simulation
        os.unlink("%s.tar.bz2"%simulation)
    # Packing
    local("tar cjvf {0}.tar.bz2 {0}/*.* {0}/runners {0}/output".format(simulation))
    return "%s.tar.bz2"%simulation



"""
    Function prepars content of runner file which is then moved onto cluster
"""
def make_runner(simulation, run, config=None):

    extra = ""
    if config != None:
        extra = "%s -c %s"%(extra,config)
    
    template= """#!/bin/csh
#
#PBS -N omnetpp__{1}_{2}
#PBS -l walltime=24:00:00
#PBS -l nodes=1:ppn=1
#PBS -l mem=16GB
#PBS -o ../output/{1}-{2}.dat
#PBS -j oe
#PBS -W umask=022
#PBS -r n

cd {0}/simulations/{1}
{0}/src/{1} -f omnetpp.ini -u Cmdenv -r {2} -n ../../references/:../../simulations/:../../src/ {3}
""".format(ROOT,simulation, run, extra)
    
    f = tempfile.NamedTemporaryFile(delete=False)
    f.write(template)
    f.close()
    return f.name
    
""" Return the number of runs for a given configuration """
def get_number_of_runs(simulation, config):
    if config == None: config="General"
    cmd="""cd {1} && ../../src/{1} -f omnetpp.ini -u Cmdenv -n ../../references/:../../simulations/:../../src/ -x {2}""".format(ROOT,simulation, config)
    output= local( '%s | grep "Number of runs:"'%cmd, capture=True) 
    a= output.split(' ')
    return int(a[-1])
    

"""
    The main worker which takes carre of sending simulation onto cluster
    frontend. Do:
     * Packs simulation to send
     * Uploads the pack
     * Unpacks the tar file with simulation
     * Created runners
     * Loads runners to queue
"""
def upload(simulation, config=None):

    print ' *** Prepare directory:', simulation
    os.system( "mkdir %s -p"% os.path.join(simulation,'runners') )
    os.system( "mkdir %s -p"% os.path.join(simulation,'output') )
    
    maximum= get_number_of_runs(simulation, config)
    print 'poct runneru', maximum
    for runid in range(0, maximum):
        tmp_runner= make_runner(simulation, runid, config=config)
        rnnr= "{0}/runners/{0}-{1}.pbs".format(simulation,runid)
        os.system("mv %s %s"%(tmp_runner, rnnr) ) 
    
    print ' *** Packing:', simulation
    make_runner(simulation,1)


    print ' *** Uploading:', simulation
    filename= drivers(simulation)
    
    
    print ' *** Uploading file:', filename
    put(filename, "~/simulations/carobs/simulations/")
    
    
    print ' *** Unpacking'
    with cd("~/simulations/carobs/simulations/"):
        run("tar jxvf %s.tar.bz2"%simulation)
        
    # removing old simulation file
    os.unlink(filename)
    
    
    print " *** Verification that runners are loaded"
    run("qstat")
      
      
        
"""
    Function downloads the result files of given simulation back to 
    desktop in order to reproduce data
"""
def download(simulation):
    print 'Downloading results of simulation:', simulation
