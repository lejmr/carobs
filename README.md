# Car Optical Burst Switching OMNeT++ simulator

Source codes of all-optical simulator based on [OMNeT++](https://omnetpp.org) event driven simulator. This simulator implements a new paradigm of Optical Burst Switching (OBS) framework called [Car OBS](http://spectrum.library.concordia.ca/979415/1/NR71141.pdf) that concatenates a number of burst for better link utilization. 

In order to use a given network topology as efficiently as possible [Stream-line effect](http://ieeexplore.ieee.org/document/1589625/) (SLE) is used. Then combination of SLE **r**outing and **w**avelength **a**ssignment (RWA) with **g**rooming provided by OBS framework [GRWA](https://dspace.cvut.cz/handle/10467/61383) algorithm is proposed and verified. This verification was tackled by this simulator.

Details about SLE and GRWA can be found in this [publication](http://www.sciencedirect.com/science/article/pii/S1573427715000193):

```bibtex
@article{Kozak201535,
title = "On the efficiency of stream line effect for contention avoidance in optical burst switching networks ",
journal = "Optical Switching and Networking ",
volume = "18, Part 1",
number = "",
pages = "35 - 50",
year = "2015",
note = "",
issn = "1573-4277",
doi = "http://dx.doi.org/10.1016/j.osn.2015.03.002",
url = "http://www.sciencedirect.com/science/article/pii/S1573427715000193",
author = "M. Kozak and B. Jaumard and L. Bohac",
keywords = "Optical burst switching",
keywords = "Large scale optimization",
keywords = "Burst loss",
keywords = "Stream line effect",
keywords = "Wavelength efficiency "
}
```