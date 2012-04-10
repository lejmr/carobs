import os

file="Book1.csv"

print 'Parsuji', file

f= open(file)
txt= f.readlines()
f.close()

cities= txt[0].split(',')[1:]

lines= open("%s_lines.csv"%file, 'w')
nodes= open("%s_nodes.csv"%file, 'w')



lines.write('#src,dest \n')
nodes.write('#name,x,y, address \n')
#RTE7,RTE0,5e7,259 
i,j=0,-1
for line in txt[1:]:
    
    for D in line.split(',')[1:]:
        j+=1
        if D.strip() == '':continue
        line= '%s,%s,%s \n'%(cities[i].strip(),cities[j].strip(),D.strip())
        #print line
        lines.write(line)
    

        
    j=-1
    nodes.write( "%s,%d,%d \n"%(cities[i].strip(),i*50,i*50) )
    i+=1
    
    
lines.close()
nodes.close()
