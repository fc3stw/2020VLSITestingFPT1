# ./atpg -ndet 1 -tdfatpg ../sample_circuits/c17.ckt | tee ../ndet_tdf_pat/c17.pat
# ./atpg -ndet 1 -tdfsim ../ndet_tdf_pat/c17.pat ../sample_circuits/c17.ckt
./atpg -ndet 3 -tdfatpg ../sample_circuits/c499.ckt > ../ndet_tdf_pat/c499.pat
./atpg -ndet 3 -tdfsim ../ndet_tdf_pat/c499.pat ../sample_circuits/c499.ckt