#!/usr/bin/env python

from bmlfactory import *
from basic_toolkit import *
from mxyzptlk import *
from beamline import *
from physics_toolkit import *
from physics_constants import *
import mappers
import chef_propagate

import math
import sys
import numpy
import string
import os.path

class Action:
    def __init__(self,type,length,initial_energy,final_energy,data,
                 synergia_action=None,element_name=None):
        self.data = data
        self.type = type
        self.length = length
        self.initial_energy = initial_energy
        self.final_energy = final_energy
        self.synergia_action = synergia_action
        self.element_name = element_name
    def get_type(self):
        return self.type
    def is_mapping(self):
        return self.type == "mapping"
    def is_synergia_action(self):
        return self.type == "synergia action"
    def get_data(self):
        return self.data
    def get_length(self):
        return self.length
    def get_initial_energy(self):
        return self.initial_energy
    def get_final_energy(self):
        return self.final_energy
    def get_synergia_action(self):
        return self.synergia_action
    def get_name(self):
        return self.element_name

accuracy_marker = marker("accuracy")
space_charge_marker = marker("space charge")
pacifier = drift("pacifier",0.0)

class Gourmet:
    '''An interface to the single-particle physics of CHEF'''
    def __init__(self, lattice_file, line_name, initial_kinetic_energy,
                 scaling_frequency, order=1, particle='proton', delay_complete=False,
                 exact_rf=False):
        self.lattice_file = lattice_file
        self.line_name = line_name
        self.scaling_frequency = scaling_frequency
        self.order = order
        self.saved_elements = []
        JetParticle.createStandardEnvironments(self.order)

        self.particle = particle
        if self.particle == 'proton':
            self.mass = PH_NORM_mp
        elif self.particle == 'positron':
            self.mass = PH_NORM_me
        else:
            raise RuntimeError, 'Unknown particle %s' % self.particle
        self.initial_kinetic_energy = initial_kinetic_energy
        self.initial_energy = self.initial_kinetic_energy + self.mass
        self.final_energy = None
        self.initial_momentum = math.sqrt(self.initial_energy**2 -
                                          self.mass**2)
        if os.path.splitext(lattice_file)[1] == '.xsif':
            #~ print "gourmet: using xsif parser for %s" % lattice_file
            self.factory = XSIFFactory(lattice_file)
            line_name = line_name.upper()
        else:
            #~ print "gourmet: using MAD8 parser for %s" % os.path.abspath(lattice_file)
            self.factory = MAD8Factory(os.path.abspath(lattice_file))
        brho = self.get_initial_particle().ReferenceBRho()
        self.beamline = self.factory.create_beamline(line_name,brho)
        self.beamline.insert(pacifier)
        self.beamline.append(pacifier)
        self.is_commissioned = False
        self.needs_commission = False
        self.exact_rf = exact_rf
    	if not delay_complete:
    		self.complete_setup() 
		
		
			
    def complete_setup(self):
        self.beamline = DriftsToSlots(self.beamline)
        self._commission()
        self.have_actions = 0
        self.have_fast_mappings = 0
        self.have_element_fast_mappings = 0
        self.context = BeamlineContext(self.get_initial_particle(),
                                       self.beamline)
        if not self.context.isTreatedAsRing():
            self.context.handleAsRing()

    def get_lattice_file(self):
        return self.lattice_file

    def get_line_name(self):
        return self.line_name

    def get_initial_energy(self):
        return self.initial_energy
    
    def get_initial_kinetic_energy(self):
        return self.initial_energy - self.mass
        
    def get_initial_particle(self):
        if self.particle == 'proton':
            particle = Proton(self.initial_energy)
        else:
            particle = Positron(self.initial_energy)
        return particle

    def get_mass(self):
        return self.mass
        
    def get_initial_jet_particle(self):
        if self.particle == 'proton':
            jet_particle = JetProton(self.initial_energy)
        else:
            jet_particle = JetPositron(self.initial_energy)
        return jet_particle

    def get_particle(self,energy):
        if self.particle == 'proton':
            particle = Proton(energy)
        else:
            particle = Positron(energy)
        return particle

    def get_jet_particle(self,energy):
        if self.particle == 'proton':
            jet_particle = JetProton(energy)
        else:
            jet_particle = JetPositron(energy)
        return jet_particle
    
    def _commission(self):
### The following is in reaction to the message:
### *** WARNING ***
### *** WARNING *** File: CF_sbend.cc, Line: 536
### *** WARNING *** void sbend::Split( double pc, bmlnElmnt** a, bmlnElmnt** b )
### *** WARNING *** The new, split elements must be commissioned with
### *** WARNING *** RefRegVisitor before being used.
### *** WARNING ***
### generated by insertions.
        if (self.is_commissioned == False) and (self.needs_commission == True): 
            compulsory_visitor = RefRegVisitor(self.get_initial_particle())
            self.beamline.accept(compulsory_visitor)
            self.is_commissioned = True
	    self.needs_commission = False 
        elif (self.is_commissioned == True) and (self.needs_commission == True):
            raise RuntimeError, "beamline already commissioned, cannot implement change"
        
    def insert_accuracy_markers(self, num_markers_per_element):
        master_insertion_point = 0.0
        ile_list = []
        particle = self.get_initial_particle()
        for element in self.beamline:
                if element.OrbitLength(particle) > 0:
                        marker_interval = element.OrbitLength(particle)/ \
                        (num_markers_per_element + 1.0)
                        insertion_point = master_insertion_point
                        for i in range(0,num_markers_per_element):
                                insertion_point += marker_interval
                                ile = accuracy_marker, insertion_point
                                ile_list.append(ile)
                master_insertion_point += element.OrbitLength(particle)
                element.propagate(particle)
        s_0 = 0.0
        self.beamline.InsertElementsFromList(particle, s_0, ile_list)
        self.beamline.append(accuracy_marker)
        self.needs_commission = True
        ##self._commission()
        
    def orbit_length(self):
        return self.beamline.OrbitLength(self.get_initial_particle())
    
    def length(self):
        return self.beamline.Length()
    
    def insert_elements(self,elements,positions):
        if type(elements) != type([]) and type(elements) != type(()):
            elements = (elements,)
            positions = (positions,)
        ile_list = []
        index = 0
        elements_to_insert = 0
        for element in elements:
            self.saved_elements.append(element)
            position = positions[index]
            if position > 0.0:
                ile = element, position
                ile_list.append(ile)
                elements_to_insert += 1
            else:
                self.beamline.insert(element)
            index += 1
        if elements_to_insert > 0:
            s_0 = 0.0
            particle = self.get_initial_particle()
            self.beamline.InsertElementsFromList(particle,s_0, ile_list)
        self.needs_commission = True
        ##self._commission()
    
    def insert_space_charge_markers(self, num_kicks):
        elements = []
        positions = []
        insertion_point = 0.0
        if (num_kicks == 0):
            elements.append(marker("synergia action:space charge endpoint"))
            positions.append(insertion_point)
            insertion_point +=  self.orbit_length()
        else:    
            kick_interval = self.orbit_length()/(1.0*num_kicks)
            for i in range(0,num_kicks):
                elements.append(marker("synergia action:space charge endpoint"))
                positions.append(insertion_point)
                insertion_point += kick_interval/2.0
                elements.append(marker("synergia action:space charge kick"))
                positions.append(insertion_point)
                insertion_point += kick_interval/2.0
        # begin bug workaround
        positions.append(insertion_point + 1000.0)
        elements.append(marker("bug workaround"))
        # end bug workaround
        end_marker = marker("synergia action:space charge endpoint")
        self.saved_elements.append(end_marker)
        self.beamline.append(end_marker)
        self.insert_elements(elements,positions)
        workaround2 = marker("beamline end bug workaround")
        self.beamline.append(workaround2)
        
    def insert_element_space_charge_markers(self, num_markers_per_element):
        if num_markers_per_element > 1:
            raise RuntimeError, \
                "insert_element_space_charge_markers only currently supports one marker per element"
        elements = []
        positions = []
        master_insertion_point = 0.0
        particle = self.get_initial_particle()
        elements.append(marker("synergia action:space charge endpoint"))
        positions.append(master_insertion_point)
        for element in self.beamline:
                if (element.OrbitLength(particle) > 0) and \
                    (element.Type() != 'rfcavity'):
#                        print "splitting",element.Type()
                        marker_interval = element.OrbitLength(particle)/ \
                        (num_markers_per_element + 1.0)
                        insertion_point = master_insertion_point
                        for i in range(0,num_markers_per_element):
                                insertion_point += marker_interval
                                elements.append(marker("synergia action:space charge kick"))
                                positions.append(insertion_point)
                        master_insertion_point += element.OrbitLength(particle)
                        elements.append(marker("synergia action:space charge endpoint"))
                        positions.append(master_insertion_point)
                else:
#                    print "not splitting",element.Name()
                    pass
                element.propagate(particle)
        positions.append(master_insertion_point + 1000.0)
        elements.append(marker("bug workaround"))
        self.beamline.append(accuracy_marker)
        self.insert_elements(elements,positions)
#        for (element,position) in zip(elements,positions):
#            print "jfa insertion list:",position,element.Name()
#        for element in self.beamline:
#            print "jfa beamline:",element.Name()
        ##self._commission()

    def insert_element_errors(self, element_name, multipole_order, pole_strength, pole_name="thinError"):
        if multipole_order ==2:
            thin_error = thin2pole(pole_name, pole_strength)
        elif multipole_order == 4:
            thin_error = thinQuad(pole_name, pole_strength)
        elif multipole_order == 6:
            thin_error = thinSextupole(pole_name, pole_strength)
        else:
            print "multipole error order ",multipole_order," not implemented, skipping isertion"
            pass
        self.saved_elements.append(thin_error)
        master_insertion_point = 0.0
        ile_list = []
        particle = self.get_initial_particle()
        inserted_error = False
        for element in self.beamline:
                if (element.OrbitLength(particle) > 0) and \
                    (element.Name().upper().find(element_name.upper()) > -1):
#                        print "splitting",element.Type()
                        marker_interval = element.OrbitLength(particle)/2
                        insertion_point = master_insertion_point + marker_interval  
                        ile = thin_error, insertion_point
                        ile_list.append(ile)
                        inserted_error = True
                else:
#                    print "not splitting",element.Name()
                    pass
                master_insertion_point += element.OrbitLength(particle)
                element.propagate(particle)
        s_0 = 0.0
        if inserted_error:
            self.beamline.InsertElementsFromList(particle, s_0, ile_list)
            self.needs_commission = True
            ##self._commission()


    def print_elements(self):
        i = 0
        s = 0
        print "%5s %10s %5s %13s %s" % ("index", 's_begin', 'length', 'type','name')
        for element in self.beamline:
            if element.Name() == "beamline end bug workaround":
                break
            print "%5d %10.3f %10.03f %10s %s" %(i,s,
                element.OrbitLength(self.get_initial_particle()),
                element.Type(), element.Name())
            i += 1
            s += element.OrbitLength(self.get_initial_particle())

    def get_lattice_functions(self):
        self._commission()
        lattice_function_array = Lattice_function_array()
        for  lattice_function in self.context.getTwissArray():
            lattice_function_array.append(lattice_function)
        return lattice_function_array

    def generate_actions(self):
        self._commission()
        self.delete_actions()
        s = 0.0
        particle = self.get_initial_particle()
        jet_particle = self.get_initial_jet_particle()
        element_names = ""
        has_propagated = 0
        energy = self.initial_energy
        for element in self.beamline:
            split_name = element.Name().split(":")
            if split_name[0] == "beamline end bug workaround":
                break
            if split_name[0] == "synergia action":
                if has_propagated:
                    mapping = mappers.Fast_mapping(self.get_u(energy),
                                                   jet_particle.State())
                    new_energy = jet_particle.ReferenceEnergy()
                    self.actions.append(
                        Action("mapping",
                               length=s,
                               initial_energy=energy,
                               final_energy=new_energy,
                               data=mapping,
                               element_name=element_names))
                    element_names = ""
                    energy = new_energy
                    s = element.OrbitLength(particle)
                    element.propagate(particle)
                    element.propagate(jet_particle)
                    if element_names == "":
                        element_names = element.Name()
                    else:
                        element_names += "+" + element.Name()
                new_energy = jet_particle.ReferenceEnergy()
                action_name= string.join(split_name[1:],':')
                if action_name == "space charge kick":
                     mydata=[]
                else:  
                     mydata=element
                       
                #if action_name == "space charge endpoint":
                    #mydata=element
                #elif action_name == "space charge kick":
                    #mydata=[]
                #else:
                    #element
                    #print "action name=",action_name
                    #raise RuntimeError, " wrong synergia action name"   
                self.actions.append(
                        Action("synergia action",
                            length=s,
                            initial_energy=energy,
                            final_energy=new_energy,
                            data=mydata,
                            synergia_action=action_name,
                            element_name=element_names))           
    
                energy = new_energy           
                jet_particle = self.get_jet_particle(energy)
                element_names = ""
                has_propagated = 0
                s = 0.0
            else:
                if element.Type() == 'rfcavity' and self.exact_rf:
                    if has_propagated:
                        mapping = mappers.Fast_mapping(self.get_u(energy),
                                                       jet_particle.State())
                        new_energy = jet_particle.ReferenceEnergy()
                        self.actions.append(
                            Action("mapping",
                                   length=s,
                                   initial_energy=energy,
                                   final_energy=new_energy,
                                   data=mapping,
                                   element_name=element_names))
                        energy = new_energy
                        s = element.OrbitLength(particle)
                        element.propagate(particle)
                        element.propagate(jet_particle)
                        new_energy = jet_particle.ReferenceEnergy()
                        self.actions.append(
                            Action("synergia action",
                                   length=s,
                                   initial_energy=energy,
                                   final_energy=new_energy,
                                   data=element,
                                   element_name=element.Name(),
                                   synergia_action='exact'))
                        energy = new_energy
                        jet_particle = self.get_jet_particle(energy)
                        element_names = ""
                        has_propagated = 0
                        s = 0.0
                elif not element.Type() == "marker":
                    s += element.OrbitLength(particle)
                    element.propagate(particle)
                    element.propagate(jet_particle)
                    if element_names == "":
                        element_names = element.Name()
                    else:
                        element_names += "+" + element.Name()
                    has_propagated = 1
        self.final_energy = jet_particle.ReferenceEnergy()
        self.have_actions = 1

    def get_actions(self):
        if not self.have_actions:
            self.generate_actions()
        return self.actions

    def print_actions(self):
        s = 0.0
        for action in self.get_actions():
            print "s:%10.4f" % s,
            print "%s," % action.get_type(),
            print "length = %0.4g," % action.get_length(),
            if action.is_synergia_action():
                print action.get_synergia_action(),
            deltaE = action.get_final_energy() - action.get_initial_energy()
            if deltaE != 0.0:
                print "deltaE = %0.4g" % deltaE,
            print
            s += action.get_length()
            
    def get_strengths(self):
        particle = self.get_initial_particle()
        brho = particle.ReferenceBRho()
        kxs = []
        kys = []
        ss = []
        s = 0.0
        for element in self.beamline:
            kx = 0.0
            ky = 0.0
            strength = element.Strength()
            length = element.OrbitLength(particle)
            s += length
            if (element.Type() == "quadrupole") or \
                   (element.Type() == "thinQuad"):
                kx = strength/brho;
                ky = -kx
            elif (element.Type() == "CF_sbend") or \
                 (element.Type() == "CF_rbend"):
                ky = -element.getQuadrupole()/brho/length
                kx = -ky + strength**2/(brho**2) 
            elif (element.Type() == "sbend") or \
                 (element.Type() == "rbend"):
		  kx = (strength/brho)**2; # this is probably wrong for rbend
		  pass
            else:
                if (strength != 0.0):
                    sys.stderr.write("gourmet.get_strengths error. element of type %s has non-zero\nStrength(), but is not handled by this routine.\n" % element.Type())
            kxs.append(kx)
            kys.append(ky)
            ss.append(s)
            element.propagate(particle)
        return (numpy.array(ss),numpy.array(kxs),numpy.array(kys))
    
    def delete_actions(self):
        self.actions = []
        self.have_actions = 0

    def _convert_linear_maps(self, chef_linear_maps):
        # units conversion
        # X_impact = U X_external
        # where U = diag(u[0],u[1],u[2],u[3],u[4],u[5])
        u = self.get_initial_u()
        linear_maps = []
        for chef_map in chef_linear_maps:
            map = numpy.zeros((7,7),'d')
            for row in range(0,6):
                for column in range(0,6):
                    chef_row = int(row/2+3*(row%2))
                    chef_column = int(column/2+3*(column%2))
                    map[row,column] = chef_map.get(chef_row,chef_column)* \
                                      u[row]/u[column]
            map[6,6] = 1.0
            linear_maps.append(map)
        return linear_maps

    # reorder chef maps but don't apply scaling
    def _convert_linear_chef_maps(self, chef_linear_maps):
        linear_maps = []
        for chef_map in chef_linear_maps:
            map = numpy.zeros((7,7),'d')
            for row in range(0,6):
                for column in range(0,6):
                    chef_row = int(row/2+3*(row%2))
                    chef_column = int(column/2+3*(column%2))
                    map[row,column] = chef_map.get(chef_row,chef_column)
            map[6,6] = 1.0
            linear_maps.append(map)
        return linear_maps
    
    def generate_fast_mappings(self):
        self._commission()
        self.delete_fast_mappings()
        if not self.have_actions:
            self.generate_actions()
        u = self.get_initial_u()
        for action in self.actions:
            if action.is_mapping():
                self.fast_mappings.append(action.get_data())
        self.have_fast_mappings = 1

    def generate_element_fast_mappings(self):
        self._commission()
        jet_particle = self.get_initial_jet_particle()
        particle = self.get_initial_particle()
        energy = self.initial_energy
        self.num_elements = 0
        self.elements = []
        self.element_fast_mappings = []
        self.element_lengths = []
        for element in self.beamline:
            self.num_elements += 1
            self.elements.append(element)
            self.element_lengths.append(element.OrbitLength(particle))
            element.propagate(jet_particle)
            mapping = mappers.Fast_mapping(self.get_u(energy),
                                                   jet_particle.State())
            self.element_fast_mappings.append(mapping)
            energy = jet_particle.ReferenceEnergy()
            jet_particle = self.get_jet_particle(energy)
        self.have_element_fast_mappings = 1

    def delete_fast_mappings(self):
        self.fast_mappings = []
        self.have_fast_mappings = 0

    def get_fast_mapping(self, index):
        if not self.have_fast_mappings:
            self.generate_fast_mappings()
        return self.fast_mappings[index]

    def get_element_fast_mapping(self, index):
        if not self.have_element_fast_mappings:
            self.generate_element_fast_mappings()
        return self.element_fast_mappings[index]
        
    def get_element_length(self, index):
        if not self.have_element_fast_mappings:
            self.generate_element_fast_mappings()
        return self.element_lengths[index]

    def propagate_element(self, index, bunch):
        self._commission()
        chef_propagate.chef_propagate(bunch.get_local_particles(),
            bunch.get_num_particles_local(),
            self.elements[index],
            self.get_initial_energy(),
            self.particle,
            self.get_initial_u(),
            self.get_initial_u())

    def get_num_elements(self):
        if not self.have_element_fast_mappings:
            self.generate_element_fast_mappings()
        return self.num_elements

    def get_element(self,index):
        if not self.have_element_fast_mappings:
            self.generate_element_fast_mappings()
        return self.elements[index]

    def get_single_linear_map(self):
        self._commission()
        jet_particle = self.get_initial_jet_particle()
        self.beamline.propagate(jet_particle)
        return self._convert_linear_maps([jet_particle.State().jacobian()])[0]

    def get_single_linear_chef_map(self):
        self._commission()
        jet_particle = self.get_initial_jet_particle()
        self.beamline.propagate(jet_particle)
        return self._convert_linear_chef_maps([jet_particle.State().jacobian()])[0]

    def get_single_fast_map(self):
        self._commission()
        jet_particle = self.get_initial_jet_particle()
        self.beamline.propagate(jet_particle)
        mapping = jet_particle.State()
        return mappers.Fast_mapping(self.get_initial_u(),
                                    jet_particle.State())

    def get_single_chef_mapping(self):
        self._commission()
        jet_particle = self.get_initial_jet_particle()
        self.beamline.propagate(jet_particle)
        mapping = jet_particle.State()

    def printpart(self,particle):
        print '|',"%0.5g" % particle.get_x(),
        print "%0.5g" % particle.get_npx(),
        print "%0.5g" % particle.get_y(),
        print "%0.5g" % particle.get_npy(),
        print "%0.5g" % particle.get_cdt(),
        print "%0.5g" % particle.get_ndp(), '|'
        
    def check(self,print_coeffs=0):
        self._commission()
        jet_particle = self.get_initial_jet_particle()
        self.beamline.propagate(jet_particle)
        mapping = jet_particle.State()
        if print_coeffs:
            mapping.printCoeffs()
        testpart = self.get_initial_particle()
        print "initial test particle:"
        self.printpart(testpart)
        self.beamline.propagate(testpart)
        print "test particle after propagation:"
        self.printpart(testpart)

    def get_u(self,energy):
        gamma = energy/self.mass
        beta = math.sqrt(1.0 - 1.0/(gamma*gamma))
        c = PH_MKS_c
        w = 2.0* math.pi* self.scaling_frequency
        u = [w/c,gamma*beta,w/c,gamma*beta,w/c,-gamma*beta*beta]
        return numpy.array(u)

    def get_initial_u(self):
        return self.get_u(self.initial_energy)
        
    def get_initial_brho(self):
        return self.get_initial_particle().ReferenceBRho()
    
class Lattice_function_array:
    def __init__(self):
        self.s = []
        self.beta_x = []
        self.beta_y = []
        self.alpha_x = []
        self.alpha_y = []

    def append(self, lattice_fn):
        self.s.append(lattice_fn.arcLength)
        self.beta_x.append(lattice_fn.beta.hor)
        self.beta_y.append(lattice_fn.beta.ver)
        self.alpha_x.append(lattice_fn.alpha.hor)
        self.alpha_y.append(lattice_fn.alpha.ver)
