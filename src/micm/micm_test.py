import sys
sys.path.append('/Users/alexgarza/Documents/CSCE482/musica/build')

import micm


solver, error = micm.create_micm("/Users/alexgarza/Documents/CSCE482/musica/src/micm/configs/chapman")
if solver is not None:
        solver.solve( 1, 100, 1, 1, 1)


