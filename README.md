# Band Structure Diagram Building Tool - Final Project

Structure repo:

- main: executable file
- main.c: source code
- AbinitWORK: contains some tutorial files of Abinit instruction.
- Bravais: contains crystal structure lattice files
- tbase\*.out: file input to parse and define what kind of lattice, it's k-point and k-path
- tbase\*\_EIG: eigennergy file
- tbase\*\_EIG.csv: result output file

Console result:

- Material: material kind. Expect `semiconductor` in almost cases.
- Fermi value: value of the fermi line. This value is used to determine what kind material is.
- Eg: Eigennergy value
