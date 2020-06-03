./atpg -ndet 2 -tdfatpg ../sample_circuits/c17.ckt > ../ndet_tdf_pat/c17.pat
./atpg -ndet 2 -tdfsim ../ndet_tdf_pat/c17.pat ../sample_circuits/c17.ckt
# ./atpg -ndet 1 -tdfatpg ../sample_circuits/c499.ckt > ../ndet_tdf_pat/c499.pat
# ./atpg -ndet 1 -tdfsim ../ndet_tdf_pat/c499.pat ../sample_circuits/c499.ckt
# ./atpg -ndet 1 -tdfatpg ../sample_circuits/c7552.ckt > ../ndet_tdf_pat/c7552.pat
# ./atpg -ndet 1 -tdfsim ../ndet_tdf_pat/c7552.pat ../sample_circuits/c7552.ckt
# ./atpg -ndet 1 -compression -tdfatpg ../sample_circuits/c17.ckt > ../ndet_tdf_pat/c17.pat
# ./atpg -ndet 1 -tdfsim ../ndet_tdf_pat/c17.pat ../sample_circuits/c17.ckt