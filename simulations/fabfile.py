from __future__ import with_statement
from fabric.api import *
from fabric.contrib.console import confirm
import os 
import tempfile

#env.use_ssh_config=True
env.keepalive=60
env.connection_attempts=1
env.forward_agent=True
env.hosts = ['kozakmil@briaree1.rqchp.qc.ca',]


SIMROOT="/RQexec/kozakmil/omnetpp"
ROOT="/home/kozakmil/simulations/carobs"


def test():
    run('whoami')

"""
    This function creates tar file in order to simplify sending process
    of the simulation onto cluster frontend
"""
def drivers(simulation, config):
    if os.path.exists("%s.tar.bz2"%simulation):
        print 'Removing old simulation file', "%s.tar.bz2"%simulation
        os.unlink("%s.tar.bz2"%simulation)
    # Packing
    local("tar cjvf {0}.tar.bz2 {0}/*.* {0}/runners/{1}".format(simulation,config))
    return "%s.tar.bz2"%simulation



"""
    Function prepars content of runner file which is then moved onto cluster
"""
def make_runner(simulation, run, config=None, mem=16, ln=4, hours=24):

    extra = ""
    if config != None:
        extra = "%s -c %s"%(extra,config)
    
    template= """#!/bin/csh
#
#PBS -N omnetpp__{1}_{5}_{2}
#PBS -l walltime={6}:00:00
#PBS -l nodes=1:ppn=1
#PBS -l mem={4}GB
#PBS -o output/{1}-{2}.dat
#PBS -j oe
#PBS -W umask=022
#PBS -r n

cd {7}/{1}
{0}/src/carobs -f omnetpp.ini -u Cmdenv -r {2} -n {0}/references/:{0}/simulations/:{0}/src/ {3}
""".format(ROOT,simulation, '%0*d'%(ln,run), extra, mem, config,hours,SIMROOT)
    
    f = tempfile.NamedTemporaryFile(delete=False)
    f.write(template)
    f.close()
    return f.name
    
""" Return the number of runs for a given configuration """
def get_number_of_runs(simulation, config):
    if config == None: config="General"
    cmd="""cd {1} && ../../src/carobs -f omnetpp.ini -u Cmdenv -n ../../references/:../../simulations/:../../src/ -x {2}""".format(ROOT,simulation, config)
    output= local( '%s | grep "Number of runs:"'%cmd, capture=True) 
    a= output.split(' ')
    return int(a[-1])
    
def prepare_runners(simulation, config="General", mem=16,hours=24):
    os.system( "mkdir %s -p"% os.path.join(simulation,'runners',config,'output') )
    
    maximum= get_number_of_runs(simulation, config)
    print "Going to prepare %s runners"%maximum
    for runid in range(0, maximum):
        tmp_runner= make_runner(simulation, runid, config=config, mem=mem, ln=len(str(maximum)), hours=hours)
        rnnr= "{0}/runners/{2}/{0}-{1}.pbs".format(simulation, '%0*d'%(len(str(maximum)),runid) ,config)
        os.system("mv %s %s"%(tmp_runner, rnnr) ) 

def clean_up(simulation):
    os.system( "rm -rf %s"% os.path.join(simulation,'runners') )
   
    
"""
    The main worker which takes carre of sending simulation onto cluster
    frontend. Do:
     * Packs simulation to send
     * Uploads the pack
     * Unpacks the tar file with simulation
     * Created runners
     * Loads runners to queue
"""
def upload(simulation, config='General', mem='16',hours=24):
    
    print ''
    print '======================================='+ (len(config)+len(simulation)+len(mem))*'='
    print '| Uploading simulation %s/%s with mem=%sGB  |'%(simulation,config,mem)
    print '======================================='+ (len(config)+len(simulation)+len(mem))*'='
    print ''
    
    print ' *** Prepare directory:', simulation
    prepare_runners(simulation, config, mem, hours)
    
    print ' *** Packing:', simulation
    filename= drivers(simulation,config)
    
    print ' *** Uploading file:', filename
    put(filename, SIMROOT )


    print ' *** Unpacking'
    with cd(SIMROOT):
        run("tar jxvf %s.tar.bz2 -C %s"%(simulation,SIMROOT) )
        run("rm %s.tar.bz2"%simulation)
        
    # removing old simulation file
    os.unlink(filename)
    
    print " *** Verification that runners are loaded"
    run("qstat | grep kozakmil")
    run("qstat | grep kozakmil | wc")

    print " *** Clean up local directory"    
    clean_up(simulation)
    
        
"""
    Function downloads the result files of given simulation back to 
    desktop in order to reproduce data
"""
def download(simulation):
    print 'Downloading results of simulation:', simulation
