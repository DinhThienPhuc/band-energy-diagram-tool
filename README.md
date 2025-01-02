# Band Energy Diagram Tool

A C-based tool for analyzing and visualizing electronic band structures from Abinit output files.

## Overview

This tool processes Abinit output files to extract and analyze electronic band structures, particularly useful for semiconductor materials research. It can determine material types, Fermi energy levels, and band gaps from electronic structure calculations.

## Features

- Parse Abinit output files (\*.out)
- Extract eigenvalues from EIG files
- Calculate band gaps and Fermi levels
- Identify material types (semiconductor, metal, etc.)
- Generate CSV output for further analysis
- Support for various Bravais lattice types

## Project Structure

```
band-energy-diagram-tool/
├── main.c # Source code
├── AbinitWORK/ # Abinit tutorial files and examples
├── Bravais/ # Crystal structure lattice files
└── README.md # Documentation
```

## Input Files

- `tbase*.out`: Contains lattice information, k-points, and k-paths
- `tbase*_EIG`: Eigenvalue data from Abinit calculations

## Output Files

- `tbase*_EIG.csv`: Processed data in CSV format
- Console output including:
  - Material classification
  - Fermi energy value
  - Band gap (Eg) value

## Building

Compile the program using a C compiler:

```bash
gcc -o main main.c
```

## Usage

1. Place your Abinit output files in the appropriate directory
2. Run the compiled executable:

```
./main
```

3. Check the generated CSV files and console output for results

## Output Format

The tool provides the following information:

- Material Type: Classification of the material (typically semiconductor)
- Fermi Level: Energy value of the Fermi level
- Band Gap (Eg): Energy difference between valence and conduction bands

## Dependencies

- Standard C libraries
- Abinit output files

## Author

@DinhThienPhuc

## License

MIT
