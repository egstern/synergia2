#!/usr/bin/env python

import s2_fish
have_impact = False
try:
    import impact
    have_impact = True
except ImportError:
    pass
import chef_propagate
from pardebug import pardebug

def listify(x):
    if type(x) == type([]) or type(x) == type(()):
        return x
    else:
        return [x]

last_step_length = 0

def propagate(s0,gourmet,bunch_in,diagnostics_in,grid_dim,quiet=1,
    use_s2_fish=False, use_impact=False, use_none=False,
    use_s2_fish_cylindrical=False,
    pgrid=None,field=None,cgrid=None,use_gauss=False,
    periodic=False, aperture=None, radius=None,
    space_charge=True,impedance=False,
    pipe_radiusx=None,pipe_radiusy=None,
    pipe_conduct=None,bunch_spacing=None):

    bunches = listify(bunch_in)
    diagnosticss = listify(diagnostics_in)

    if len(bunches) != len(diagnosticss):
        raise RuntimeError,\
            "propagate: len(bunches) must = len(diagnosticss)"
    s = s0
    global last_step_length
    first_action = 1
    for action in gourmet.get_actions():
        if action.is_mapping():
            for bunch in bunches:
                #~ pardebug("start apply_map...\n")
                action.get_data().apply(bunch.get_local_particles(),
                                   bunch.get_num_particles_local())
                #~ pardebug("done\n")
            last_step_length = action.get_length()
            s += last_step_length
        elif action.is_synergia_action():
            if action.get_synergia_action() == "space charge endpoint":
                if not first_action:
                    for num in range(0,len(bunches)):
                        diagnosticss[num].add(s,bunches[num])
                    if not quiet:
                        print "finished space charge kick"
            elif action.get_synergia_action() == "space charge kick":
                tau = last_step_length
                #~ pardebug("start space charge\n")
                if use_s2_fish:
                    #~ pardebug("s2_fish...\n")
                    s2_fish.apply_space_charge_kick(grid_dim,None,None, bunches, 2*tau,
                        periodic=periodic,aperture=aperture,space_charge=space_charge,
                        impedance=impedance,pipe_radiusx=pipe_radiusx,
                        pipe_radiusy=pipe_radiusy,pipe_conduct=pipe_conduct,
                        bunch_spacing=bunch_spacing)
                elif use_s2_fish_cylindrical:
                    raise RuntimeError,"jfa: temporarily broken!!!!"
                    s2_fish.apply_cylindrical_space_charge_kick(grid_dim,
                        radius,bunch,2*tau,aperture=aperture,space_charge=space_charge,
                        impedance=impedance,pipe_radius=pipe_radius,
                        pipe_conduct=pipe_conduct)
                elif use_impact:
                    raise RuntimeError,"jfa: temporarily broken!!!!"
                    if not have_impact:
                        raise RuntimeError, \
                            "propagate with use_impact=True requires a working impact module"                        
                    if ((pgrid == None) or (field == None) or (cgrid == None)):
                        raise RuntimeError, \
                            "propagate with use_impact=True requires pgrid, field and cgrid to be specified"
                    impact.apply_space_charge_kick(
                        bunch.get_beambunch(),
                        pgrid.get_pgrid2d(),
                        field.get_fieldquant(),
                        field.get_compdom(),
                        field.get_period_length(),
                        cgrid.get_bc_num(),
                        field.get_pipe_radius(),
                        tau, 0, bunch.get_scaling_frequency(),0)
                elif use_gauss:
                    raise RuntimeError,"jfa: temporarily broken!!!!"
                    s2_fish.apply_BasErs_space_charge_kick(bunch, 2*tau)
                elif use_none:
                    pass
                else:
                    raise RuntimeError, \
                        "propagate requires one of use_s2_fish, use_impact or use_none to be True"
                #~ pardebug("end space charge\n")
            elif action.get_synergia_action() == "rfcavity1" or \
                action.get_synergia_action() == "rfcavity2":
                #~ pardebug("rfcavity\n")
                element = action.get_data()
                u_in = gourmet.get_u(action.get_initial_energy())
                u_out = gourmet.get_u(action.get_final_energy())
                for bunch in bunches:
                    #~ pardebug("start chef_propagate...\n")
                    chef_propagate.chef_propagate(
                        bunch.get_local_particles(), bunch.get_num_particles_local(),
                        element, action.get_initial_energy(), gourmet.particle,
                        u_in, u_out)
                    #~ pardebug("end\n")
            else:
                print "unknown action: '%s'" % \
                      action.get_synergia_action()
        else:
            print "action",action.get_type(),"unknown"
        first_action = 0
    return s
