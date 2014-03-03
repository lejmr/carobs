from numpy import *
import networkx as nx

MAP='paneurope'
FILE='/media/sf_Dropbox/CVUT/Omnetpp/carobs/references/%s/lines.csv'%MAP
POS='/media/sf_Dropbox/CVUT/Omnetpp/carobs/references/%s/nodes.csv'%MAP


def genG(FILE,POS):
    I= [ x.strip().split(',') for x in open(FILE,'r').readlines() ][1:]
    C= list(set( [ x[0] for x in I ] + [ x[1] for x in I ] ))
    C.sort()

    elist= []
    for i in I:
        src, dst, l= i
        elist.append( (src,dst,float(l)) )    

    # Positions
    P= [ x.strip().split(',') for x in open(POS,'r').readlines() ][1:]
    pos= {}
    for x in P:
        pos[ x[0] ] = ( float(x[1]), 2000-float(x[2]) )


    G=nx.Graph()
    G.add_weighted_edges_from(elist)
    
    return G

def dijikstra_weight(G):
    P= dict( zip( C, list( zeros( (1,len(C)) )[0] ) ) )
    for src in G.node:
        for dst in G.node:
            if src == dst: continue
            
            PATH= nx.dijkstra_path(G,src,dst)
            for node in PATH[1:-1]:
                P[node]+=1
    
    return P



"""
nx.draw(G,pos,node_color=numpy.array(P.values()),
        cmap='OrRd',
        node_size=numpy.array(G.degree().values())*200 )
show()
#savefig('dijkstra_path_%s.pdf'%MAP)
"""